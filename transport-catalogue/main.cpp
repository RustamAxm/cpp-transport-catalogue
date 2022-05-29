
#include "stat_reader.h"
#include "input_reader.h"


int main() {
    transport_catalogue::TransportCatalogue catalogue;
/* В первый раз специально хотел не передавать класс каталога в заполняющие и печатающие модули,
 * поэтому навводил много дополнительных структур.
 * Подумал что передавать объект класса в модули не лучшая идея и наплодил контейнеров.
 * Но все же после Ваших коментариев переделал, но структуры оставил.
*/
    std::istream& input = std::cin;
    std::ostream& out = std::cout;

    data_frame_reader::AddDataFrame(catalogue, input);
    query_reader::AddQuery(catalogue, input, out);

    return 0;
}
