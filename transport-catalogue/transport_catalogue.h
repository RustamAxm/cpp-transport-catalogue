
#pragma once

#include <set>
#include <unordered_map>
#include <deque>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <iostream>

#include "geo.h"
#include "domain.h"




namespace transport_catalogue {
    using namespace domain;

    class StopHasher {
    public:
        size_t operator()(const std::pair<Stop*, Stop*> plate) const {
            assert(plate.first) ;
            return std::hash<const Stop * >()(plate.first) ;
        }
    };


    class TransportCatalogue {
    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        // хеш таблицы
        std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
        std::unordered_map<std::string_view, Bus*> busname_to_bus_;
        std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_bus_;

        // таблица расстояний
        using DistanceMap = std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopHasher>;
        DistanceMap distances_stop_stop_;

        std::vector<Stop*> StopsPtrFinder(const std::vector<std::string>& stops);

        size_t ComputeAllDistance(const std::vector<Stop*>& container, bool circle);

        double ComputeAllDistanceGeo(const std::vector<Stop*>& container, bool circle);

        template<typename T>
        size_t SortUniq(const std::vector<T>& container);

        std::pair<int, int> StopsCounter(const std::string& query);

        bool isValidBus(const std::string& query);

        bool isValidStop(const std::string& query);

    public:
        TransportCatalogue() = default;

        void AddStop(Stop stop);

        void AddBus(const std::string& bus, const std::vector<std::string>& stops, bool circle);

        void SetDistances(const std::string& from_stop, const std::string& to_stop, size_t distance);

        size_t DistanceFinder(const std::string_view first, const std::string_view second);

        std::pair<int, int> GetStopsStatForBus(const std::string& query);

        size_t GetAllBusDistance(const std::string& query);

        double GetCurvature(const std::string& query);

        std::optional<std::reference_wrapper<const std::set<std::string_view>>>
        GetBusesForStop(const std::string& query);

        stat_for_printer::Bus GetAllBusStat(const std::string& query);

        stat_for_printer::Stop GetAllStopStat(const std::string& query);

        // for map renderer
        std::unordered_map<std::string_view, Bus*> GetBusesForRender();
        // for transport router
        std::unordered_map<std::string_view, std::set<std::string_view>>& GetStopsForRouter();
        std::unordered_map<std::string_view, Bus*>& GetBusesForRouter();

        ~TransportCatalogue() = default;
    };

    template<typename T>
    size_t TransportCatalogue::SortUniq(const std::vector<T>& container) {
        auto tmp = container;
        std::sort(tmp.begin(), tmp.end());
        auto words_end = std::unique(tmp.begin(), tmp.end());
        tmp.erase(words_end, tmp.end());

        return tmp.size();
    }
}