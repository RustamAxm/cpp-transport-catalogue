#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <set>
#include "geo.h"

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

