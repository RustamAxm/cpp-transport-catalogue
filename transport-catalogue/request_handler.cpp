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

        std::map<std::string_view, domain::Bus*> busname_to_bus;
        for (const auto& x : db_.GetBusesForRender()) {
            busname_to_bus[x.first] = x.second;
        }
        renderer_.SetBusNameToBus(busname_to_bus);
    }


    void RequestHandler::SetStopsForRender() const {
        std::map<std::string_view, geo::Coordinates*> stop_to_stops_coord;
        for (const auto& x : db_.GetStopsForRender()) {
            stop_to_stops_coord[x.first] = &x.second->coord_;
        }
        renderer_.SetStops(stop_to_stops_coord);
    }

}