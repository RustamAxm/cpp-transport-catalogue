#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

#include "geo.h"
#include "transport_catalogue.h"

namespace data_frame_reader {

    struct Stop_dist {
        std::string stop;
        size_t distance;
    };

    struct StopParserStruct {
        std::string name_;
        geo::Coordinates coord_;
        std::vector<Stop_dist> distances_to_stops;
    };

    struct BusParserStruct {
        std::string number_;
        bool circle;
        std::vector<std::string> stops_;
    };

    class InputReader {
    public:

        InputReader(std::istream& input);

        std::vector<StopParserStruct> GetStops();

        std::vector<BusParserStruct> GetBuses();

    private:
        std::vector<std::string> tmp_stops;
        std::vector<std::string> tmp_buses;

        std::vector<StopParserStruct> stops_;
        std::vector<BusParserStruct> buses_;

        void ReadTmpData(std::istream& input);

        std::string_view SpaceDeleter(std::string_view word);

        StopParserStruct StopsParser(std::string_view stop);

        std::vector<std::string> StopsFinder(std::string_view text, char sep);

        BusParserStruct BusParser(std::string_view bus);

    };

    void AddDataFrame(transport_catalogue::TransportCatalogue& catalogue, std::istream& input);

}