
#include "input_reader.h"

namespace data_frame_reader {

    InputReader::InputReader() {
        ReadTmpData();
    }

    std::vector<StopParserStruct> InputReader::GetStops() {
        for (const auto& stop : tmp_stops) {
            stops_.push_back(StopsParser(stop));
        }
        return stops_;
    }

    std::vector<BusParserStruct> InputReader::GetBuses() {
        for (const auto& bus : tmp_buses) {
            buses_.push_back(BusParser(bus));
        }
        return buses_;
    }

    void InputReader::ReadTmpData() {
        std::string line;
        std::getline(std::cin, line);
        int n = std::stoi(line);

        for (int i = 0; i < n; i++) {
            std::getline(std::cin, line);
            if (line[0] == 'S') {
                tmp_stops.push_back(std::move(line));
            } else if (line[0] == 'B') {
                tmp_buses.push_back(std::move(line));
            }
        }
    }

    std::string_view InputReader::SpaceDeleter(std::string_view word) {
        word.remove_prefix(std::min(word.find_first_not_of(' '), word.size()));
        size_t trim_pos = word.find_last_not_of(' ');

        if (trim_pos != word.npos) {
            word.remove_suffix(word.size() - trim_pos - 1);
        }

        return word;
    }

    StopParserStruct InputReader::StopsParser(std::string_view stop) {

        std::vector<Stop_dist> distances_to_stops;

        std::size_t space = stop.find_first_of(' ');
        stop.remove_prefix(space + 1);
        std::size_t end_stop = stop.find(':');
        std::string_view stop_name = stop.substr(0, end_stop);

        stop.remove_prefix(end_stop + 1);
        std::size_t comma = stop.find_first_of(',');

        double latitude = std::stod(std::string(stop.substr(0, comma)));
        stop.remove_prefix(comma + 1);

        comma = stop.find_first_of(',');
        double longitude = 0.0;
        if (comma == std::string::npos) {
            longitude = std::stod(std::string(stop));
        } else {
            longitude = std::stod(std::string(stop.substr(0, comma)));
            stop.remove_prefix(comma + 1);

            int num = std::count(stop.begin(), stop.end(), ',');

            for (int i = 0; i <= num; ++i) {
                std::size_t metr = stop.find_first_of('m');
                size_t distance = std::stoi(std::string(stop.substr(0, metr)));
                stop.remove_prefix(metr + 4);

                comma = stop.find_first_of(',');
                std::string stop_n = std::string(SpaceDeleter(stop.substr(0, comma)));
                stop.remove_prefix(comma + 1);

                distances_to_stops.push_back({stop_n, distance});
            }

        }

        return {std::string(stop_name), latitude, longitude, distances_to_stops};
    }

    std::vector<std::string> InputReader::StopsFinder(std::string_view text, char sep) {
        std::vector<std::string> names_;

        int num = std::count(text.begin(), text.end(), sep);

        for (int i = 0; i <= num; ++i) {
            size_t split = text.find_first_of(sep);
            std::string stop_name = std::string(SpaceDeleter(text.substr(0, split)));
            text.remove_prefix(split + 1);
            names_.push_back(stop_name);
        }
        return names_;
    }

    BusParserStruct InputReader::BusParser(std::string_view bus) {
        std::size_t space = bus.find_first_of(' ');
        bus.remove_prefix(space + 1);
        std::size_t end_bus = bus.find(':');
        std::string_view bus_number = bus.substr(0, end_bus);
        bus.remove_prefix(end_bus + 1);

        bool circle;

        std::vector<std::string> names_;
        if (bus.find('>') != std::string_view::npos) {
            names_ = StopsFinder(bus, '>');
            circle = true;
        } else {
            names_ = StopsFinder(bus, '-');
            circle = false;
        }

        return {std::string(bus_number), circle, names_};
    }

}
