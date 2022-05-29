#include <iomanip>

#include "stat_reader.h"
#include "transport_catalogue.h"
#include "input_reader.h"


void BusPrinter(const transport_catalogue::stat_for_printer::Bus& bus_stat) {
    if (bus_stat.is_valid) {
        std::cout <<"Bus "<< bus_stat.bus_number << ": ";
        std::cout << std::setprecision(6);
        std::cout << bus_stat.stops_on_route <<" stops on route, ";
        std::cout << bus_stat.unique_stops <<" unique stops, ";
        std::cout << bus_stat.route_lenght << " route length, ";
        std::cout << bus_stat.curvature << " curvature";
        std::cout << std::endl;
    } else {
        std::cout <<"Bus " << bus_stat.bus_number << ": " << "not found" << std::endl;
    }
}
void StopPrinter(const transport_catalogue::stat_for_printer::Stop& stop_stat) {
    if (stop_stat.is_valid) {
        if (!stop_stat.present_buses) {
            std::cout << "Stop " << stop_stat.stop << ": " << "no buses" << std::endl;
        } else {
            std::cout << "Stop " << stop_stat.stop << ": buses ";
            for (const auto& item : stop_stat.buses_on_stop) {
                std::cout << item << " ";
            }
            std::cout << std::endl;
        }
    } else {
        std::cout << "Stop " << stop_stat.stop << ": " << "not found" << std::endl;
    }
}



int main() {
    using namespace data_frame_reader;

    data_frame_reader::InputReader add_data;

    auto stops = add_data.GetStops();

    transport_catalogue::TransportCatalogue catalogue;

    for (const auto& stop : stops) {
        catalogue.AddStop(stop.name_, stop.coord_.lat, stop.coord_.lng);
    }
    auto buses = add_data.GetBuses();

    for (const auto& bus : buses) {
        catalogue.AddBus(bus.number_, bus.stops_, bus.circle);
    }

    for (const auto& stop : stops) {
        for (const auto& x : stop.distances_to_stops) {
            catalogue.AddDistances(stop.name_, x.stop, x.distance);
        }
    }

    reader::StatReader query_prep;

    auto query = query_prep.GetQuery();
    for (const auto& x : query) {
        if (x.isBus) {
            BusPrinter( catalogue.GetAllBusStat(x.name_));
        } else {
            StopPrinter(catalogue.GetAllStopStat(x.name_));
        }
    }

    return 0;
}
