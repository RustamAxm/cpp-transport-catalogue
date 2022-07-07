#pragma once

#include <sstream>

#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

namespace json_reader {

    using namespace json;
    using TransportCatalogue = transport_catalogue::TransportCatalogue;
    using MapRenderer = map_renderer::MapRenderer;
    using Request = request_handler::RequestHandler;
    using TransportRouter = transport_router::TransportRouter;

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
        JsonReader(TransportCatalogue& tc,
                   MapRenderer& renderer,
                   Request& request,
                   TransportRouter& router);

        void AddDataFrame(std::istream& input);

        void FillCatalogue();

        void DocumentPrinter(std::ostream& out);

    private:
        TransportCatalogue& catalogue_;
        MapRenderer& renderer_;
        Request& request_;
        TransportRouter& router_;

        Document document_ ;
        Array arr;

        std::vector<request::Stop> req_stops_;
        std::vector<request::Bus> req_buses_;

        request::Stop AddStop(const Dict& dict);

        request::Bus AddBus(const Dict& dict);

        void FillBase(const Array& array);

        void FillBusArray(const Dict& node_map);

        void FillStopArray(const Dict& node_map);

        void FillMapArray(const Dict& node_map);

        void FillRouteArray(const Dict& node_map);

        void FillStat(const Array& array);

        svg::Color ChooseColor(const json::Node& node);

        void FillRenderSettings(const Dict& dict);

        void FillRoutingSettings(const Dict& dict);
    };
}