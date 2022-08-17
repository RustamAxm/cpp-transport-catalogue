#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <transport_catalogue.pb.h>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

using namespace std::string_literals;

namespace transport_data_base {

struct SerializationSettings {
    std::string file_name = ""s;
};


class TransportCatalogueSerialization{
public:
    TransportCatalogueSerialization(transport_catalogue::TransportCatalogue& catalogue,
                                    map_renderer::MapRenderer& renderer,
                                    transport_router::TransportRouter& router) :
                                    catalogue_(catalogue),
                                    renderer_(renderer),
                                    router_(router) {}

    void SetSettings(const SerializationSettings& settings);

    void Serialize();

    void Deserialize();

private:
    transport_catalogue::TransportCatalogue& catalogue_;
    map_renderer::MapRenderer& renderer_;
    transport_router::TransportRouter& router_;
    SerializationSettings settings_;

    mutable transport_data_base::TransportCatalogue base_;
    /// Serialize
    // Transport catalogue
    transport_data_base::Catalogue SerializeCatalogue();
    transport_data_base::Stop SerializeStop(const domain::Stop* stop_data);
    transport_data_base::Bus SerializeBus(const domain::Bus* bus_data);
    transport_data_base::Distance SerializeDistance(const std::pair<domain::Stop*, domain::Stop*> stops,
                                                        size_t length);
    // Map renderer
    transport_data_base::MapRenderer SerializeMapRenderer();
    transport_data_base::RenderSettings SerializeRenderSettings();
    transport_data_base::Color SerializeColor(const svg::Color& color);
    // Transport router
    transport_data_base::TransportRouter SerializeTransportRouter();
    transport_data_base::RouterSettings SerializeRouterSettingsData();
    transport_data_base::TransportRouterData SerializeTransportRouterClass();
    transport_data_base::Router SerializeRouter();
    transport_data_base::Graph SerializeGraph();

    /// Deserialize
    // Transport catalogue
    void DeserializeCatalogue(const transport_data_base::Catalogue& base);
    // Map renderer
    void DeserializeMapRenderer(const transport_data_base::MapRenderer& base);
    map_renderer::RenderSettings DeserializeMapRenderSettings(const transport_data_base::RenderSettings& render_set);
    svg::Color DeserializeColor(const transport_data_base::Color& base_color);
    // Transport router
    void DeserializeTransportRouterData(const transport_data_base::TransportRouter& base);
    transport_router::RouterSettings DeserializeTransportRouterSettings(const transport_data_base::RouterSettings& base);
    void DeserializeTransportRouterClass(const transport_data_base::TransportRouterData& base);

    std::unordered_map<std::string_view, size_t>
    DeserializeStopToId(const transport_data_base::TransportRouterData& base);
    std::unordered_map<size_t, std::string_view>
    DeserializeIdToStop(const transport_data_base::TransportRouterData& base);
    std::unordered_map<size_t, std::string_view>
    DeserializeEdgeIdToBus(const transport_data_base::TransportRouterData& base);
    std::unordered_map<size_t, size_t>
    DeserializeEdgeIdToSpanCount(const transport_data_base::TransportRouterData& base);
    // Router
    void DeserializeRouter(const transport_data_base::Router& base);
    std::optional<graph::Router<double>::RouteInternalData>
    DeserializeRouteInternal(transport_data_base::RouteInternalDataVectorElem& base);
    // Graph
    void DeserializeGraph(const transport_data_base::Graph& base);
    std::vector<graph::Edge<double>>
    DeserializeGraphEdges(const transport_data_base::Graph& base);
    std::vector<graph::IncidenceList>
    DeserializeGraphIncidenceLists(const transport_data_base::Graph& base);
};

}