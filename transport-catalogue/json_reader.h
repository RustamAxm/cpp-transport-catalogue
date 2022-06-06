#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
#include "json.h"
#include "transport_catalogue.h"

namespace json_reader {

    using namespace json;
    using TransportCatalogue = transport_catalogue::TransportCatalogue;

namespace request {

    struct Stop_dist {
        std::string stop;
        size_t distance;
    };

    struct Stop {
        std::string name;
        double latitude;
        double longitude;
        std::unordered_map<std::string, int> road_distances;
    };

    struct Bus {
        std::string name;
        std::vector<std::string> stops;
        bool is_roundtrip;
    };

    struct Stat {
        size_t id;
        std::string name;
    };

}

    class JsonReader {
    public:
        explicit JsonReader(TransportCatalogue& tc) : catalogue_(tc) {}

        void AddDataFrame(std::istream& input) {
            document_ = json::Load(input);
            for (const auto& node : document_.GetRoot().AsMap()) {
                if (node.first == "base_requests") {
                    BaseFiller(node.second.AsArray());
                    AddToCatalogue();
                } else if (node.first == "stat_requests") {
                    StatFiller(node.second.AsArray());
                }
            }


        }

        void AddToCatalogue() {

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

    private:
        TransportCatalogue& catalogue_;
        Document document_;

        std::vector<request::Stop> req_stops_;
        std::vector<request::Bus> req_buses_;
        std::vector<request::Stat> req_stats_;

        request::Stop AddStop(const Dict& dict) {
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

        request::Bus AddBus(const Dict& dict) {
            request::Bus tmp;
            tmp.name = dict.at("name").AsString();
            tmp.is_roundtrip = dict.at("is_roundtrip").AsBool();
            for (const auto& x : dict.at("stops").AsArray()) {
                tmp.stops.push_back(x.AsString());
            }
            return tmp;
        }

        void BaseFiller(const Array& array) {

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

        void StatFiller(const Array& array) {
            Array arr;
            for (const auto& node : array) {

                if (node.AsMap().count("type")) {

                    if (node.AsMap().at("type").AsString() == "Bus") {
                        auto id = node.AsMap().at("id").AsInt();

                        domain::stat_for_printer::Bus tmp;
                        tmp = catalogue_.GetAllBusStat(node.AsMap().at("name").AsString());
                        Dict dict_;
                        dict_["curvature"] = tmp.curvature;
                        dict_["request_id"] = id;
                        dict_["route_length"] = static_cast<int> (tmp.route_lenght);
                        dict_["stop_count"] = tmp.stops_on_route;
                        dict_["unique_stop_count"] = tmp.unique_stops;

                        arr.emplace_back(dict_);

                    } else if (node.AsMap().at("type").AsString() == "Stop") {
                        auto id = node.AsMap().at("id").AsInt();

                        domain::stat_for_printer::Stop tmp;
                        tmp = catalogue_.GetAllStopStat(node.AsMap().at("name").AsString());

                        Array vec;
                        for ( auto x : tmp.buses_on_stop) {
                            vec.push_back(std::string (x));
                        }
                        arr.emplace_back(Dict{ {"buses", std::move(vec)}, {"request_id", id} });

                    }
                }
            }
            json::Print(Document(arr), std::cout);
        }

    };
}