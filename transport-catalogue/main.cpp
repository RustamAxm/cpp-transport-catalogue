
#include "json_reader.h"
#include "request_handler.h"


void TestJsonReader() {

    std::istream& input = std::cin;
    std::ostream& out = std::cout;

    transport_catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalogue);

    request_handler::RequestHandler request(catalogue,
                                            renderer);

    json_reader::JsonReader reader(catalogue,
                                   renderer,
                                   request,
                                   router);
    reader.AddDataFrame(input); // add to transport catalogue

//    request.SetRoutesForRender(); // add buses and stops to render
//    renderer.Render(out); // print just map
    reader.DocumentPrinter(out); // print request and map
}

int main() {
    TestJsonReader();
}
