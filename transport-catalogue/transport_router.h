#pragma once

#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <map>
#include <optional>
#include <memory>

namespace transport_router {

    struct RoutingSettings {
        int bus_wait_time = 0;
        double bus_velosuty = 0.0;
    };

    namespace result {
        struct Wait {
            std::string stop_name;
            int time = 0;
        };

        struct Bus {
            std::string bus;
            size_t span_count;
            double time;
        };

        struct Route {
            double total_time = 0.0;
            std::vector<std::pair<Wait, Bus>> items;
        };
    }

    using TransportCatalogue = transport_catalogue::TransportCatalogue;
    class TransportRouter {
    public:
        explicit TransportRouter(TransportCatalogue& catalogue);

        void SetSettings(RoutingSettings& settings);

        void ComputeGraph();

        std::optional<result::Route> ComputeRoute(std::string_view from, std::string_view to);

    private:
        RoutingSettings settings_;
        TransportCatalogue& tc_;
        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> router_ = nullptr;

        std::unordered_map<std::string_view, size_t> stop_to_id_;
        std::unordered_map<size_t, std::string_view> id_to_stop_;
        std::unordered_map<size_t, std::string_view> edge_id_to_bus_;
        std::unordered_map<size_t, size_t> edge_id_to_span_count_;

        double CalculateWeight(size_t distance);

        bool isNotValidRoute(std::string_view from, std::string_view to);

        std::vector<double> ComputeWeight(const std::vector<domain::Stop*>& route);

        void FillGraph(const std::vector<domain::Stop*>& route, size_t& vertices_cntr, std::string_view bus_name);

        std::vector<domain::Stop*> StopsForLineRoute(const std::vector<domain::Stop*>& container);
    };
}

