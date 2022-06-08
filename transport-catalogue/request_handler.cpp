#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

namespace request_handler {

    void RequestHandler::SetRoutesForRender() const {
        std::map<std::string_view, std::vector<geo::Coordinates>> bus_to_stops_coord;
        std::map<std::string_view, domain::Bus*> busname_to_bus;
        for (const auto& x : db_.GetBusesForRender()) {
            std::vector<geo::Coordinates> coord;
            if (x.second->circle) {
                for (const auto& j : x.second->stop_names_) {
                    coord.push_back(j->coord_);
                }
            } else {
                for (size_t i = 0; i < x.second->stop_names_.size(); ++i) {
                    coord.push_back(x.second->stop_names_[i]->coord_);
                }
                for (size_t i = x.second->stop_names_.size() - 1; i > 0; --i) {
                    coord.push_back(x.second->stop_names_[i-1]->coord_);
                }
            }
            bus_to_stops_coord[x.first] = coord;
            busname_to_bus[x.first] = x.second;
        }
        renderer_.SetRoutes(bus_to_stops_coord);
        renderer_.SetBusNameToBus(busname_to_bus);
    }



    void RequestHandler::SetStopsForRender() const {
        std::map<std::string_view, geo::Coordinates> stop_to_stops_coord;
        for (const auto& x : db_.GetStopsForRender()) {
            stop_to_stops_coord[x.first] = x.second->coord_;
        }
        renderer_.SetStops(stop_to_stops_coord);
    }

}