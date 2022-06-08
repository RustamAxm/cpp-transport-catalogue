
#include "json_reader.h"
#include "request_handler.h"

//void TestInputReader() {
//
//    transport_catalogue::TransportCatalogue catalogue;
///* В первый раз специально хотел не передавать класс каталога в заполняющие и печатающие модули,
// * поэтому навводил много дополнительных структур.
// * Подумал что передавать объект класса в модули не лучшая идея и наплодил контейнеров.
// * Но все же после Ваших коментариев переделал, но структуры оставил.
//*/
//    std::istream& input = std::cin;
//    std::ostream& out = std::cout;
//
//    data_frame_reader::AddDataFrame(catalogue, input);
//    query_reader::AddQuery(catalogue, input, out);
//
//}

void TestJsonReader() {

    std::istream& input = std::cin;
    std::ostream& out = std::cout;

    transport_catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;


    json_reader::JsonReader reader(catalogue, renderer);
    request_handler::RequestHandler request(catalogue, renderer);

    reader.AddDataFrame(input);
//    reader.DocumentPrinter(out);
    request.SetRoutesForRender();
    renderer.Render(out);

}


int main() {
    TestJsonReader();
}
