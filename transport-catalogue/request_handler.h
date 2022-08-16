#pragma once

#include <optional>
#include <iostream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "domain.h"
#include "transport_router.h"


namespace request_handler {

    using TransportCatalogue = transport_catalogue::TransportCatalogue;
    using MapRenderer = map_renderer::MapRenderer;

    class RequestHandler {
    public:
        explicit RequestHandler(TransportCatalogue& db,
                                MapRenderer& renderer);

        void SetRoutesForRender() const;

    private:
        TransportCatalogue& db_;
        MapRenderer& renderer_;
    };
}


