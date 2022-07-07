
#include "transport_catalogue.h"

namespace transport_catalogue {

    std::vector<Stop*> TransportCatalogue::StopsPtrFinder(const std::vector<std::string>& stops) {
        std::vector<Stop*> names_;
        for (const auto& stop_name : stops) {
            names_.push_back(stopname_to_stop_.at(stop_name));
        }
        return names_;
    }

    size_t TransportCatalogue::DistanceFinder(const std::string_view first, const std::string_view second) {

        size_t distance = 0;

        std::pair<Stop*, Stop*> key_ = std::make_pair(stopname_to_stop_.at(first),
                                                      stopname_to_stop_.at(second));
        if (distances_stop_stop_.count(key_)) {
            return distances_stop_stop_.at(key_);
        }
        key_ = std::make_pair(stopname_to_stop_.at(second),
                              stopname_to_stop_.at(first));
        if (distances_stop_stop_.count(key_)) {
            return distances_stop_stop_.at(key_);
        }
        return distance;
    }

    size_t TransportCatalogue::ComputeAllDistance(const std::vector<Stop*>& container, bool circle) {
        size_t all_distance = 0;
        for (size_t i = 0; i < container.size() - 1; ++i) {
            all_distance += DistanceFinder(container[i]->name_, container[i+1]->name_);
        }
        if (!circle) {
            for (size_t i = container.size() - 1; i > 0; --i) {
                all_distance += DistanceFinder(container[i]->name_,container[i-1]->name_);
            }
        }
        return all_distance;
    }

    double TransportCatalogue::ComputeAllDistanceGeo(const std::vector<Stop*>& container, bool circle) {
        double all_distance = 0.0;
        for (size_t i = 0; i < container.size() - 1; ++i) {
            all_distance += ComputeDistance(container[i]->coord_, container[i+1]->coord_);
        }
        if (!circle) {
            for (size_t i = container.size() - 1; i > 0; --i) {
                all_distance += ComputeDistance(container[i]->coord_,container[i-1]->coord_);
            }
        }
        return all_distance;
    }

    std::pair<int, int> TransportCatalogue::StopsCounter(const std::string& query) {
        size_t stops = 0;
        size_t unique_stops = SortUniq(busname_to_bus_.at(query)->stop_names_);
        if (busname_to_bus_.at(query)->circle) {
            stops = busname_to_bus_.at(query)->stop_names_.size();
        } else {
            stops = 2 * busname_to_bus_.at(query)->stop_names_.size() - 1;
        }

        return {stops, unique_stops};
    }

    void TransportCatalogue::AddStop(Stop stop) {

        stops_.push_back(std::move(stop));
        stopname_to_stop_.insert({stops_.back().name_, &stops_.back()});
    }

    void TransportCatalogue::AddBus(const std::string& bus, const std::vector<std::string>& stops, bool circle) {
        Bus tmp_bus = {bus, circle, StopsPtrFinder(stops)};
        buses_.push_back(tmp_bus);
        busname_to_bus_.insert({buses_.back().number_, &buses_.back()});

        for (auto stop : buses_.back().stop_names_) {
            stopname_to_bus_[stop->name_].insert(buses_.back().number_);
        }
    }

    void TransportCatalogue::SetDistances(const std::string& from_stop, const std::string& to_stop, size_t distance) {
        auto stop_from = stopname_to_stop_.at(from_stop);
        auto stop_to = stopname_to_stop_.at(to_stop);
        std::pair<Stop*, Stop*> key_ = std::make_pair(stop_from, stop_to);
        distances_stop_stop_.insert({key_, distance});
    }

    std::pair<int, int> TransportCatalogue::GetStopsStatForBus(const std::string& query) {
        return StopsCounter(query);
    }

    size_t TransportCatalogue::GetAllBusDistance(const std::string& query) {
        return ComputeAllDistance(busname_to_bus_.at(query)->stop_names_,
                                  busname_to_bus_.at(query)->circle);
    }

    double TransportCatalogue::GetCurvature(const std::string& query) {
        size_t length = GetAllBusDistance(query);
        double coord_length = ComputeAllDistanceGeo(busname_to_bus_.at(query)->stop_names_,
                                                    busname_to_bus_.at(query)->circle);

        return static_cast<double>(length / std::abs(coord_length));
    }

    bool TransportCatalogue::isValidBus(const std::string& query) {
        return busname_to_bus_.count(query) > 0;
    }

    bool TransportCatalogue::isValidStop(const std::string& query) {
        return stopname_to_stop_.count(query) > 0;
    }

    std::optional<std::reference_wrapper<const std::set<std::string_view>>>
    TransportCatalogue::GetBusesForStop(const std::string& query) {
        auto found = stopname_to_bus_.find(query);
        if (found == stopname_to_bus_.end()) {
            return std::nullopt;
        } else {
            return found->second;
        }
    }

    stat_for_printer::Bus TransportCatalogue::GetAllBusStat(const std::string& query) {
        stat_for_printer::Bus stat;
        stat.bus_number = query;
        stat.is_valid = isValidBus(query);
        if (stat.is_valid) {
            auto stop_stat = GetStopsStatForBus(query);
            stat.stops_on_route = stop_stat.first;
            stat.unique_stops = stop_stat.second;
            stat.route_lenght = GetAllBusDistance(query);
            stat.curvature = GetCurvature(query);
        }
        return stat;
    }

    stat_for_printer::Stop TransportCatalogue::GetAllStopStat(const std::string& query) {
        stat_for_printer::Stop stat;
        stat.stop = query;
        stat.is_valid = isValidStop(query);
        if (stat.is_valid) {
            auto data = GetBusesForStop(query);
            if (data == std::nullopt) {
                stat.present_buses = false;
            } else {
                stat.present_buses = true;
                stat.buses_on_stop = GetBusesForStop(query)->get();
            }
        }
        return stat;
    }

    std::unordered_map<std::string_view, Bus*> TransportCatalogue::GetBusesForRender() {
        return busname_to_bus_;
    }

    std::unordered_map<std::string_view, std::set<std::string_view>>& TransportCatalogue::GetStopsForRouter() {
        return stopname_to_bus_;
    }

    std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetBusesForRouter() {
        return busname_to_bus_;
    }


}