#include "stat_reader.h"

namespace reader {

    std::string_view StatReader::QueryFinder(std::string_view line) {
        std::size_t space = line.find_first_of(' ');
        line.remove_prefix(space + 1);
        return line;
    }

    void StatReader::AddQuery() {
        std::string line;

        std::getline(std::cin, line);
        int n = std::stoi(line);

        for (int i = 0; i < n; ++i) {
            std::getline(std::cin, line);
            if (line[0] == 'B') {
                std::string bus_number = std::string(QueryFinder(line));
                query_.push_back({true, bus_number});

            } else if (line[0] == 'S') {
                std::string stop_name = std::string(QueryFinder(line));
                query_.push_back( {false, stop_name});
            }
        }
    }
}

