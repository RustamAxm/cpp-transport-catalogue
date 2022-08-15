
#include "json_reader.h"
#include "request_handler.h"
#include "serialization.h"


void MakeBseAndSerialize(std::istream& input, std::ostream& out) {

    transport_catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalogue);

    request_handler::RequestHandler request(catalogue,
                                            renderer);

    transport_data_base::TransportCatalogueSerialization serialization(catalogue);

    json_reader::JsonReader reader(catalogue,
                                   renderer,
                                   request,
                                   router,
                                   serialization);
    reader.AddDataFrame(input); // add to transport catalogue

    serialization.Serialize();
}

void ProcessRequestAndDeserialize(std::istream& input, std::ostream& out) {

    transport_catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalogue);

    request_handler::RequestHandler request(catalogue,
                                            renderer);

    transport_data_base::TransportCatalogueSerialization serialization(catalogue);

    json_reader::JsonReader reader(catalogue,
                                   renderer,
                                   request,
                                   router,
                                   serialization);
    reader.AddDataFrame(input); // add to transport catalogue

    serialization.Deserialize();
    reader.DocumentPrinter(out);
}

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

    request_handler::RequestHandler request(catalogue,
                                            renderer);

    transport_data_base::TransportCatalogueSerialization serialization(catalogue);

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

int TestFillBase(int argc, char* argv[]) {
    const std::string mode = "make_base";

    std::istream& input = std::cin;
    std::ostream& out = std::cout;

    if (mode == "make_base") {
        MakeBseAndSerialize(std::cin, std::cout);
    }
    else if (mode == "process_requests") {
        ProcessRequestAndDeserialize(std::cin, std::cout);
    }

    return 0;
}

int SimpleTest() {

    std::istream& input = std::cin;
    std::ostream& out = std::cout;

    transport_catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalogue);

    request_handler::RequestHandler request(catalogue,
                                            renderer);

    transport_data_base::TransportCatalogueSerialization serialization(catalogue);

    json_reader::JsonReader reader(catalogue,
                                   renderer,
                                   request,
                                   router,
                                   serialization);
    reader.AddDataFrame(input); // add to transport catalogue

//    request.SetRoutesForRender(); // add buses and stops to render
//    renderer.Render(out); // print just map
    reader.DocumentPrinter(out); // print request and map
    return 0;
}

int main(int argc, char* argv[]) {
//    return TestFillBase(argc, argv);
//    return SimpleTest();
    return Production(argc, argv);
}