#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <set>
#include "geo.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace domain {

    struct Stop {
        std::string name_;
        geo::Coordinates coord_;
    };

    struct Bus {
        std::string number_;
        bool circle;
        std::vector<Stop*> stop_names_;
    };

    namespace stat_for_printer {

        struct Bus {
            std::string_view bus_number;
            bool is_valid;
            int stops_on_route;
            int unique_stops;
            size_t route_lenght;
            double curvature;
        };

        struct Stop {
            std::string_view stop;
            bool is_valid;
            bool present_buses;
            std::set<std::string_view> buses_on_stop;
        };
    }

}

