#include "transport_router.h"

namespace transport_router {

    TransportRouter::TransportRouter(TransportCatalogue &catalogue) : tc_(catalogue){
    }

    void TransportRouter::SetSettings(const RouterSettings& settings) {
        settings_ = settings;
    }

    RouterSettings TransportRouter::GetSettings() {
        return settings_;
    }

    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
        return graph_;
    }

    const graph::Router<double>::RoutesInternalData& TransportRouter::GetRouterData() const { //Sprint 14
        return router_.get()->GetRoutesInternalData();
    }

    const std::unordered_map<std::string_view, size_t>* TransportRouter::GetStopToId() const {
        return &stop_to_id_;
    }

    const std::unordered_map<size_t, std::string_view>* TransportRouter::GetIdToStop() const {
        return &id_to_stop_;
    }

    const std::unordered_map<size_t, std::string_view>* TransportRouter::GetEdgeIdToBus() const {
        return &edge_id_to_bus_;
    }

    const std::unordered_map<size_t, size_t>* TransportRouter::GetEdgeIdToSpanCount() const {
        return &edge_id_to_span_count_;
    }

    std::unordered_map<std::string_view, size_t>& TransportRouter::SetStopToId() {
        return stop_to_id_;
    }

    std::unordered_map<size_t, std::string_view>& TransportRouter::SetIdToStop() {
        return id_to_stop_;
    }

    std::unordered_map<size_t, std::string_view>& TransportRouter::SetEdgeIdToBus() {
        return edge_id_to_bus_;
    }

    std::unordered_map<size_t, size_t>& TransportRouter::SetEdgeIdToSpanCount() {
        return edge_id_to_span_count_;
    }

    graph::DirectedWeightedGraph<double>& TransportRouter::ModifyGraph() {
        return graph_;
    }

    void TransportRouter::GenerateEmptyRouter() {
        if (router_ != nullptr) {
            router_.release();
        }
        router_ = std::make_unique<graph::Router<double>>(graph::Router(graph_));
    }

    std::unique_ptr<graph::Router<double>>& TransportRouter::ModifyRouter() {
        return router_;
    }

    void TransportRouter::ComputeGraph() {
        graph_.ResizeIncidenceLists(tc_.GetStopsForRouter().size());

        size_t vertices_cntr = 0;
        for (auto [bus_name, bus_ptr] : tc_.GetBusesForRouter()) {
            if (bus_ptr->circle) {
                std::vector<domain::Stop*> route = bus_ptr->stop_names_;
                FillGraph(route, vertices_cntr, bus_name);
            } else {
                std::vector<domain::Stop*> route = StopsForLineRoute(bus_ptr->stop_names_);
                FillGraph(route, vertices_cntr, bus_name);
            }
        }
        router_ = std::make_unique<graph::Router<double>>(graph::Router(graph_));
    }

    std::optional<result::Route> TransportRouter::ComputeRoute(std::string_view from,
                                                               std::string_view to) {
        if (isNotValidRoute(from, to)) {
            return std::nullopt;
        }

        auto route_data =  router_->BuildRoute(stop_to_id_.at(from),
                                                                stop_to_id_.at(to));

        if (route_data) {
            result::Route result;
            result.total_time = route_data->weight;
            for (auto id : route_data->edges) {
                result::Wait tmp_wait;
                tmp_wait.time = settings_.bus_wait_time;
                tmp_wait.stop_name = id_to_stop_.at(graph_.GetEdge(id).from);
                result::Bus tmp_bus;
                tmp_bus.time = graph_.GetEdge(id).weight - settings_.bus_wait_time;
                tmp_bus.bus = edge_id_to_bus_.at(id);
                tmp_bus.span_count = edge_id_to_span_count_.at(id);
                result.items.emplace_back(std::make_pair(tmp_wait, tmp_bus));
            }
            return result;
        } else {
            return std::nullopt;
        }
    }

    double TransportRouter::CalculateWeight(size_t distance) {
        return static_cast<double>(distance) / (settings_.bus_velocity * 1000.0 / 60);
    }

    std::vector<double> TransportRouter::ComputeWeight(const std::vector<domain::Stop*>& route) {
        std::vector<double> weight_sum(route.size());
        weight_sum[0] = 0;
        double sum = 0;
        for (int i = 1; i < weight_sum.size(); ++i) {
            auto point_from = route[i-1]->name_;
            auto point_to = route[i]->name_;
            size_t distance = tc_.DistanceFinder(point_from, point_to);
            sum += CalculateWeight(distance);
            weight_sum[i] = sum;
        }
        return weight_sum;
    }

    bool TransportRouter::isNotValidRoute(std::string_view from,
                                          std::string_view to) {

        return !tc_.GetStopsForRouter().count(from) ||
               !tc_.GetStopsForRouter().count(to) ||
                tc_.GetStopsForRouter().at(from).empty() ||
                tc_.GetStopsForRouter().at(to).empty();
    }

    void TransportRouter::FillGraph(const std::vector<domain::Stop*>& route,
                                    size_t& vertices_cntr,
                                    std::string_view bus_name) {
        std::vector<double> weight_sum = ComputeWeight(route);
        for (size_t i = 0; i < route.size() - 1; ++i) {
            for (size_t j = i + 1; j < route.size(); ++j) {
                if (!stop_to_id_.count(route[i]->name_)) {
                    id_to_stop_[vertices_cntr] = route[i]->name_;
                    stop_to_id_[route[i]->name_] = vertices_cntr++;
                }
                if (!stop_to_id_.count(route[j]->name_)) {
                    id_to_stop_[vertices_cntr] = route[j]->name_;
                    stop_to_id_[route[j]->name_] = vertices_cntr++;
                }
                graph::Edge<double> edge;
                edge.from = stop_to_id_.at(route[i]->name_);
                edge.to = stop_to_id_.at(route[j]->name_);
                edge.weight = weight_sum[j] - weight_sum[i] + settings_.bus_wait_time;
                size_t edge_id = graph_.AddEdge(edge);
                edge_id_to_bus_[edge_id] = bus_name;
                edge_id_to_span_count_[edge_id] = j - i;
            }
        }
    }

    std::vector<domain::Stop*> TransportRouter::StopsForLineRoute(const std::vector<domain::Stop*>& container) {
        std::vector<domain::Stop*> route;
        for (auto it =  container.begin(); it != container.end(); std::advance(it, 1)) {
            route.push_back(*it);
        }
        for (auto it =  container.rbegin(); it != container.rend(); std::advance(it, 1)) {
            route.push_back(*it);
        }
        return route;
    }
} // namespace transport_router