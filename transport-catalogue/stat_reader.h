#pragma once

#include <vector>
#include <iostream>
#include <iomanip>

#include "transport_catalogue.h"


namespace query_reader {

    void AddQuery(transport_catalogue::TransportCatalogue& catalogue,
                  std::istream& input,
                  std::ostream& out);
    std::string_view QueryFinder(std::string_view line);

}

namespace printer {

    void Bus(const transport_catalogue::stat_for_printer::Bus& bus_stat, std::ostream& out);
    void Stop(const transport_catalogue::stat_for_printer::Stop& stop_stat, std::ostream& out);

}


