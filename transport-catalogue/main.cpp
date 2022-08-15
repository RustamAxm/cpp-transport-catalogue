
#include "json_reader.h"
#include "request_handler.h"
#include "serialization.h"


void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n";
}

int Production(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }
    const std::string_view mode(argv[1]);

    std::istream& input = std::cin;
    std::ostream& out = std::cout;

    transport_catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalogue);

    transport_data_base::TransportCatalogueSerialization serialization(catalogue, renderer);

    request_handler::RequestHandler request(catalogue,
                                            renderer);

    json_reader::JsonReader reader(catalogue,
                                   renderer,
                                   request,
                                   router,
                                   serialization);


    if (mode == "make_base") {
        reader.AddDataFrame(input); // add to transport catalogue
        serialization.Serialize();
    }
    else if (mode == "process_requests") {
        reader.AddDataFrame(input); // add to transport catalogue
        reader.DocumentPrinter(out);
    }
    else {
        PrintUsage();
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {

    return Production(argc, argv);
}