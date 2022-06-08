#pragma once

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

#include <optional>
#include <iostream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "domain.h"


namespace request_handler {

    using TransportCatalogue = transport_catalogue::TransportCatalogue;
    using MapRenderer = map_renderer::MapRenderer;

    class RequestHandler {
    public:
        explicit RequestHandler(TransportCatalogue& db, MapRenderer& renderer) : db_(db), renderer_(renderer) {
        }

        std::optional<const domain::Bus*> GetBusStat(std::string_view bus_name);

        const std::set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

        void RenderMap() const;

        void SetStopsForRender() const;

        void SetRoutesForRender() const;

    private:
        TransportCatalogue& db_;
        MapRenderer& renderer_;
    };
}


