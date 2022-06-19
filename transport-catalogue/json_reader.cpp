#include "json_reader.h"

namespace json_reader {

    JsonReader::JsonReader(TransportCatalogue& tc, MapRenderer& renderer, Request& request) :
    catalogue_(tc), renderer_(renderer), request_(request) {}


    void JsonReader::AddDataFrame(std::istream& input) {
        document_ = json::Load(input);
        for (const auto& node : document_.GetRoot().AsMap()) {
            if (node.first == "base_requests") {
                FillBase(node.second.AsArray());
                FillCatalogue();
            } else if (node.first == "stat_requests") {
                FillStat(node.second.AsArray());
            } else if (node.first == "render_settings") {
                FillRenderSettings(node.second.AsMap());
            }
        }
    }


    void JsonReader::FillCatalogue() {

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
        request::Stop stop;
        stop.name = dict.at("name").AsString();
        stop.latitude = dict.at("latitude").AsDouble();
        stop.longitude = dict.at("longitude").AsDouble();

        if (dict.count("road_distances")) {
            for (const auto& x : dict.at("road_distances").AsMap()) {
                stop.road_distances[x.first] = x.second.AsInt();
            }
        }
        return stop;
    }

    request::Bus JsonReader::AddBus(const Dict& dict) {
        request::Bus bus;
        bus.name = dict.at("name").AsString();
        bus.is_roundtrip = dict.at("is_roundtrip").AsBool();
        for (const auto& x : dict.at("stops").AsArray()) {
            bus.stops.push_back(x.AsString());
        }
        return bus;
    }

    void JsonReader::FillBase(const Array& array) {

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

    void JsonReader::FillBusArray(const Dict& node_map) {
        auto id = node_map.at("id").AsInt();

        domain::stat_for_printer::Bus bus_stat;
        bus_stat = catalogue_.GetAllBusStat(node_map.at("name").AsString());
        Dict dict_;

        Builder for_doc_to_print_;
        for_doc_to_print_.StartDict();
        if (bus_stat.is_valid) {
            for_doc_to_print_.Key("curvature").Value(bus_stat.curvature);
            for_doc_to_print_.Key("request_id").Value(id);
            for_doc_to_print_.Key("route_length").Value(static_cast<int> (bus_stat.route_lenght));
            for_doc_to_print_.Key("stop_count").Value(bus_stat.stops_on_route);
            for_doc_to_print_.Key("unique_stop_count").Value(bus_stat.unique_stops);
        } else {
            for_doc_to_print_.Key("request_id").Value(id);
            for_doc_to_print_.Key("error_message").Value("not found"s);
        }
        for_doc_to_print_.EndDict();

        arr.emplace_back(for_doc_to_print_.Build());

    }

    void JsonReader::FillStopArray(const Dict& node_map) {
        auto id = node_map.at("id").AsInt();

        domain::stat_for_printer::Stop stop_stat = catalogue_.GetAllStopStat(node_map.at("name").AsString());
        Dict dict_;

        Builder for_doc_to_print_;
        for_doc_to_print_.StartDict();

        if (stop_stat.is_valid) {
            Array vec;
            for ( auto x : stop_stat.buses_on_stop) {
                vec.push_back(std::string (x));
            }
            for_doc_to_print_.Key("buses").Value(std::move(vec));
            for_doc_to_print_.Key("request_id").Value(id);
        } else {
            for_doc_to_print_.Key("request_id").Value(id);
            for_doc_to_print_.Key("error_message").Value("not found"s);
        }
        for_doc_to_print_.EndDict();
        arr.emplace_back(std::move(for_doc_to_print_.Build()));
    }

    void JsonReader::FillMapArray(const Dict& node_map) {
        auto id = node_map.at("id").AsInt();
        Builder for_doc_to_print_;
        for_doc_to_print_.StartDict();
        for_doc_to_print_.Key("request_id").Value(id);
        std::ostringstream buffer;
        request_.SetRoutesForRender();
        renderer_.Render(buffer);
        for_doc_to_print_.Key("map").Value(buffer.str());
        for_doc_to_print_.EndDict();

        arr.emplace_back(for_doc_to_print_.Build());
    }

    void JsonReader::FillStat(const Array& array) {
        for (const auto& node : array) {
            if (node.AsMap().count("type")) {
                if (node.AsMap().at("type").AsString() == "Bus") {
                    FillBusArray(node.AsMap());
                } else if (node.AsMap().at("type").AsString() == "Stop") {
                    FillStopArray(node.AsMap());
                } else if (node.AsMap().at("type").AsString() == "Map") {
                    FillMapArray(node.AsMap());
                }
            }
        }
    }

    svg::Color JsonReader::ChooseColor(const json::Node& node) {
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

    void JsonReader::FillRenderSettings(const Dict& dict) {
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
        rend.underlayer_color = ChooseColor(dict.at("underlayer_color"));
        rend.underlayer_width = dict.at("underlayer_width").AsDouble();
        for (const auto& node : dict.at("color_palette").AsArray()) {
            rend.color_palette.push_back(ChooseColor(node));
        }
        renderer_.SetSettings(rend);
    }
}