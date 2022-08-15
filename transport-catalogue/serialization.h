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
                                    map_renderer::MapRenderer& renderer) :
            catalogue_(catalogue),
            renderer_(renderer) {}

    void SetSettings(const SerializationSettings& settings);

    void Serialize();

    void Deserialize();

private:
    transport_catalogue::TransportCatalogue& catalogue_;
    map_renderer::MapRenderer& renderer_;
//    transport_router::TransportRouter& router_;
    SerializationSettings settings_;

    mutable transport_data_base::TransportCatalogue base_;

    transport_data_base::Catalogue SerializeCatalogue();
    transport_data_base::Stop SerializeStop(const domain::Stop* stop_data);
    transport_data_base::Bus SerializeBus(const domain::Bus* bus_data);
    transport_data_base::Distance SerializeDistance(const std::pair<domain::Stop*, domain::Stop*> stops,
                                                        size_t length);

    transport_data_base::MapRenderer SerializeMapRenderer();
    transport_data_base::RenderSettings SerializeRenderSettings();
    transport_data_base::Color SerializeColor(const svg::Color& color);

    void DeserializeCatalogue(const transport_data_base::Catalogue& base);

    void DeserializeMapRenderer(const transport_data_base::MapRenderer& base);
    map_renderer::RenderSettings DeserializeMapRenderSettings(const transport_data_base::RenderSettings& render_set);
    svg::Color DeserializeColor(const transport_data_base::Color& base_color);
};

}