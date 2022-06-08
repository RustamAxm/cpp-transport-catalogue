#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {

    JsonReader::JsonReader(TransportCatalogue& tc, MapRenderer& renderer) :
    catalogue_(tc), renderer_(renderer) {}


    void JsonReader::AddDataFrame(std::istream& input) {
        document_ = json::Load(input);
        for (const auto& node : document_.GetRoot().AsMap()) {
            if (node.first == "base_requests") {
                BaseFiller(node.second.AsArray());
                AddToCatalogue();
            } else if (node.first == "stat_requests") {
                StatFiller(node.second.AsArray());
            } else if (node.first == "render_settings") {
                RenderFiller(node.second.AsMap());
            }
        }
    }


    void JsonReader::AddToCatalogue() {

        for (const auto& stop : req_stops_) {
            catalogue_.AddStop({stop.name, stop.latitude, stop.longitude});
        }

        for (const auto& bus : req_buses_) {
            catalogue_.AddBus(bus.name, bus.stops, bus.is_roundtrip);
        }

        for (const auto& stop : req_stops_) {
            for (const auto& x : stop.road_distances) {
                catalogue_.SetDistances(stop.name, x.first, x.second);
            }
        }
    }

    void JsonReader::DocumentPrinter(std::ostream& out) {
        json::Print(Document(arr), out);
    }



    request::Stop JsonReader::AddStop(const Dict& dict) {
        request::Stop tmp;
        tmp.name = dict.at("name").AsString();
        tmp.latitude = dict.at("latitude").AsDouble();
        tmp.longitude = dict.at("longitude").AsDouble();

        if (dict.count("road_distances")) {
            for (const auto& x : dict.at("road_distances").AsMap()) {
                tmp.road_distances[x.first] = x.second.AsInt();
            }
        }
        return tmp;
    }

    request::Bus JsonReader::AddBus(const Dict& dict) {
        request::Bus tmp;
        tmp.name = dict.at("name").AsString();
        tmp.is_roundtrip = dict.at("is_roundtrip").AsBool();
        for (const auto& x : dict.at("stops").AsArray()) {
            tmp.stops.push_back(x.AsString());
        }
        return tmp;
    }

    void JsonReader::BaseFiller(const Array& array) {

        for(const auto& node : array) {
            if (node.AsMap().count("type")) {
                if (node.AsMap().at("type").AsString() == "Stop") {
                    req_stops_.push_back(AddStop(node.AsMap()));
                } else if (node.AsMap().at("type").AsString() == "Bus") {
                    req_buses_.push_back(AddBus(node.AsMap()));
                }
            }
        }
    }

    void JsonReader::BusArrayFiller(const Dict& node_map) {
        auto id = node_map.at("id").AsInt();

        domain::stat_for_printer::Bus tmp;
        tmp = catalogue_.GetAllBusStat(node_map.at("name").AsString());
        Dict dict_;
        if (tmp.is_valid) {
            dict_["curvature"] = tmp.curvature;
            dict_["request_id"] = id;
            dict_["route_length"] = static_cast<int> (tmp.route_lenght);
            dict_["stop_count"] = tmp.stops_on_route;
            dict_["unique_stop_count"] = tmp.unique_stops;
        } else {
            dict_["request_id"] = id;
            dict_["error_message"] = "not found"s;
        }

        arr.emplace_back(dict_);
    }

    void JsonReader::StopArrayFiller(const Dict& node_map) {
        auto id = node_map.at("id").AsInt();

        domain::stat_for_printer::Stop tmp;
        tmp = catalogue_.GetAllStopStat(node_map.at("name").AsString());
        Dict dict_;
        if (tmp.is_valid) {
            Array vec;
            for ( auto x : tmp.buses_on_stop) {
                vec.push_back(std::string (x));
            }
            dict_["buses"] = std::move(vec);
            dict_["request_id"] = id;

        } else {
            dict_["request_id"] = id;
            dict_["error_message"] = "not found"s;
        }
        arr.emplace_back(dict_);
    }

    void JsonReader::MapArrayFiller(const Dict& node_map) {
        auto id = node_map.at("id").AsInt();

    }

    void JsonReader::StatFiller(const Array& array) {
        for (const auto& node : array) {
            if (node.AsMap().count("type")) {
                if (node.AsMap().at("type").AsString() == "Bus") {
                    BusArrayFiller(node.AsMap());
                } else if (node.AsMap().at("type").AsString() == "Stop") {
                    StopArrayFiller(node.AsMap());
                } else if (node.AsMap().at("type").AsString() == "Map") {
                    MapArrayFiller(node.AsMap());
                }
            }
        }
    }

    svg::Color JsonReader::ColorMatcher(const json::Node& node) {
        if (node.IsArray()) {
            if (node.AsArray().size() == 3) {
                svg::Rgb rgb;
                rgb.red = node.AsArray()[0].AsInt();
                rgb.green = node.AsArray()[1].AsInt();
                rgb.blue = node.AsArray()[2].AsInt();
                return rgb;
            } else {
                svg::Rgba rgba;
                rgba.red = node.AsArray()[0].AsInt();
                rgba.green = node.AsArray()[1].AsInt();
                rgba.blue = node.AsArray()[2].AsInt();
                rgba.opacity = node.AsArray()[3].AsDouble();
                return rgba;
            }
        } else {
            return node.AsString();
        }
    }

    void JsonReader::RenderFiller(const Dict& dict) {
        map_renderer::RenderSettings rend;
        rend.width = dict.at("width").AsDouble();
        rend.height = dict.at("height").AsDouble();
        rend.padding = dict.at("padding").AsDouble();
        rend.line_width = dict.at("line_width").AsDouble();
        rend.stop_radius = dict.at("stop_radius").AsDouble();
        rend.bus_label_font_size = dict.at("bus_label_font_size").AsInt();
        rend.bus_label_offset[0] = dict.at("bus_label_offset").AsArray()[0].AsDouble();
        rend.bus_label_offset[1] = dict.at("bus_label_offset").AsArray()[1].AsDouble();
        rend.stop_label_font_size = dict.at("stop_label_font_size").AsInt();
        rend.stop_label_offset[0] = dict.at("stop_label_offset").AsArray()[0].AsDouble();
        rend.stop_label_offset[1] = dict.at("stop_label_offset").AsArray()[1].AsDouble();
        rend.underlayer_color = ColorMatcher(dict.at("underlayer_color"));
        rend.underlayer_width = dict.at("underlayer_width").AsDouble();
        for (const auto& node : dict.at("color_palette").AsArray()) {
            rend.color_palette.push_back(ColorMatcher(node));
        }
        renderer_.SetSettings(rend);
    }
}