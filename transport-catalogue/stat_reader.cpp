#include "stat_reader.h"

namespace query_reader {

    std::string_view QueryFinder(std::string_view line) {
        std::size_t space = line.find_first_of(' ');
        line.remove_prefix(space + 1);
        return line;
    }

    void AddQuery(transport_catalogue::TransportCatalogue& catalogue,
                  std::istream& input,
                  std::ostream& out) {
        std::string line;

        std::getline(input, line);
        int n = std::stoi(line);

        for (int i = 0; i < n; ++i) {
            std::getline(input, line);
            if (line[0] == 'B') {
                std::string bus_number = std::string(QueryFinder(line));
                printer::Bus( catalogue.GetAllBusStat(bus_number), out);

            } else if (line[0] == 'S') {
                std::string stop_name = std::string(QueryFinder(line));
                printer::Stop(catalogue.GetAllStopStat(stop_name), out);
            }
        }
    }
}

namespace printer {

    void Bus(const transport_catalogue::stat_for_printer::Bus& bus_stat, std::ostream& out) {
        if (bus_stat.is_valid) {
            out <<"Bus "<< bus_stat.bus_number << ": ";
            out << std::setprecision(6);
            out << bus_stat.stops_on_route <<" stops on route, ";
            out << bus_stat.unique_stops <<" unique stops, ";
            out << bus_stat.route_lenght << " route length, ";
            out << bus_stat.curvature << " curvature";
            out << std::endl;
        } else {
            out <<"Bus " << bus_stat.bus_number << ": " << "not found" << std::endl;
        }
    }

    void Stop(const transport_catalogue::stat_for_printer::Stop& stop_stat, std::ostream& out) {
        if (stop_stat.is_valid) {
            if (!stop_stat.present_buses) {
                out << "Stop " << stop_stat.stop << ": " << "no buses" << std::endl;
            } else {
                out << "Stop " << stop_stat.stop << ": buses ";
                for (const auto& item : stop_stat.buses_on_stop) {
                    out << item << " ";
                }
                out << std::endl;
            }
        } else {
            out << "Stop " << stop_stat.stop << ": " << "not found" << std::endl;
        }
    }

}

