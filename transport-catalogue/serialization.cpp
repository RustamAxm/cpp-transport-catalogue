#include "serialization.h"

namespace transport_data_base {

    void TransportCatalogueSerialization::SetSettings(const SerializationSettings& settings) {
        settings_ = settings;
    }

    void TransportCatalogueSerialization::Serialize(){

        *base_.mutable_transport_catalogue() = std::move(SerializeCatalogue());

        std::filesystem::path path = settings_.file_name;
        std::ofstream out_file(path, std::ios::binary);
        base_.SerializeToOstream(&out_file);
    }

    transport_data_base::Catalogue TransportCatalogueSerialization::SerializeCatalogue() {
        Catalogue tmp_catalogue;
        int s = 0;
        for (auto& [stop_name, stop_data] : catalogue_.GetStopNameToStop()) {
            tmp_catalogue.add_stops();
            *tmp_catalogue.mutable_stops(s) = std::move(SerializeStop(stop_data));
            ++s;
        }
        int b = 0; // bus index in proto
        for (auto& [bus_name, bus_data] : catalogue_.GetBusesForRender()) {
            tmp_catalogue.add_buses();
            *tmp_catalogue.mutable_buses(b) = std::move(SerializeBus(bus_data));
            ++b;
        }
        int d = 0; // dist counter
        for (auto& [stops, length] : catalogue_.GetDistanceTable()) {
            tmp_catalogue.add_distances();          // add element in array of proto
            *tmp_catalogue.mutable_distances(d) = std::move(SerializeDistance(stops, length)); // modify new element
            ++d;
        }
        return tmp_catalogue;
    }

    transport_data_base::Stop TransportCatalogueSerialization::SerializeStop(const domain::Stop* stop_data) {
        transport_data_base::Stop tmp_stop;
        tmp_stop.set_name(std::string(stop_data->name_));
        tmp_stop.mutable_coord()->set_lat(stop_data->coord_.lat);
        tmp_stop.mutable_coord()->set_lng(stop_data->coord_.lng);
        return tmp_stop;
    }

    transport_data_base::Bus TransportCatalogueSerialization::SerializeBus(const domain::Bus* bus_data) {
        transport_data_base::Bus tmp_bus;
        tmp_bus.set_number(std::string(bus_data->number_));
        for (const auto stop : bus_data->stop_names_) {
            tmp_bus.add_stop_names(stop->name_);
        }
        tmp_bus.set_circle(bus_data->circle);
        return tmp_bus;
    }

    transport_data_base::Distance TransportCatalogueSerialization::SerializeDistance(const std::pair<domain::Stop*, domain::Stop*> stops,
                                                                                     size_t length) {
        transport_data_base::Distance tmp_dist;
        tmp_dist.set_from(stops.first->name_);
        tmp_dist.set_to(stops.second->name_);
        tmp_dist.set_distance(static_cast<uint32_t>(length));
        return tmp_dist;
    }

    void TransportCatalogueSerialization::Deserialize() {
        std::filesystem::path path = settings_.file_name;
        std::ifstream in_file(path, std::ios::binary);

        transport_data_base::TransportCatalogue base;

        base.ParseFromIstream(&in_file);

        DeserializeCatalogue(base.transport_catalogue());
    }

    void TransportCatalogueSerialization::DeserializeCatalogue(const transport_data_base::Catalogue& base) {

        for (int i = 0; i < base.stops_size(); ++i) {
            catalogue_.AddStop({base.stops(i).name(),
                                base.stops(i).coord().lat(),
                                base.stops(i).coord().lng()});
        }

        for (int i = 0; i < base.buses_size(); ++i) {
            std::vector<std::string_view> stops;
            for (int j = 0; j < base.buses(i).stop_names_size(); ++j) {
                stops.push_back(base.buses(i).stop_names(j));
            }
            catalogue_.AddBus(base.buses(i).number(), stops, base.buses(i).circle());
        }

        for (int i = 0; i < base.distances_size(); ++i) {
            catalogue_.SetDistances(base.distances(i).from(),
                                    base.distances(i).to(),
                                    base.distances(i).distance());
        }
    }
};