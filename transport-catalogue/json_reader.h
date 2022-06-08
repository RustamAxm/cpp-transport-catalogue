#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace json_reader {

    using namespace json;
    using TransportCatalogue = transport_catalogue::TransportCatalogue;
    using MapRenderer = map_renderer::MapRenderer;

namespace request {

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

}


    class JsonReader {
    public:
        JsonReader(TransportCatalogue& tc, MapRenderer& renderer);

        void AddDataFrame(std::istream& input);

        void AddToCatalogue();

        void DocumentPrinter(std::ostream& out);

    private:
        TransportCatalogue& catalogue_;
        Document document_;
        Array arr;

        MapRenderer& renderer_;

        std::vector<request::Stop> req_stops_;
        std::vector<request::Bus> req_buses_;


        request::Stop AddStop(const Dict& dict);

        request::Bus AddBus(const Dict& dict);

        void BaseFiller(const Array& array);

        void BusArrayFiller(const Dict& node_map);

        void StopArrayFiller(const Dict& node_map);

        void MapArrayFiller(const Dict& node_map);

        void StatFiller(const Array& array);

        svg::Color ColorMatcher(const json::Node& node);

        void RenderFiller(const Dict& dict);
    };
}