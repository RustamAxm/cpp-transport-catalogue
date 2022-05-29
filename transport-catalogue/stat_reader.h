#pragma once

#include <vector>
#include <iostream>


namespace reader {

    struct Query {
        bool isBus;
        std::string name_;
    };

    class StatReader {
    public:
        StatReader(){
            AddQuery();
        }

        std::vector<Query> GetQuery() {
            return query_;
        }


    private:
        std::vector<Query> query_;

        void AddQuery();

        std::string_view QueryFinder(std::string_view line);
    };
}


