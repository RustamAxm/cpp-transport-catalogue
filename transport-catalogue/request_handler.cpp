#include "request_handler.h"


namespace request_handler {

    RequestHandler::RequestHandler(TransportCatalogue& db, MapRenderer& renderer) : db_(db), renderer_(renderer) {
    }

    void RequestHandler::SetRoutesForRender() const {

        std::map<std::string_view, domain::Bus*> busname_to_bus;
        for (const auto& x : db_.GetBusesForRender()) {
            busname_to_bus[x.first] = x.second;
        }
        renderer_.SetBusNameToBus(busname_to_bus);
    }

}