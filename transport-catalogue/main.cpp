
#include "json_reader.h"
#include "request_handler.h"
#include "serialization.h"
#include <zmq.hpp>


void ZMQMessage(json_reader::JsonReader reader,
                std::istream& input,
                std::ostream& out) {
    zmq::context_t context(1);
    zmq::socket_t socket{context, zmq::socket_type::rep};
    socket.bind("tcp://*:5555");

    for (int i = 0; i < 1; i++) {
        zmq::message_t request;
        std::stringstream in;
        std::stringstream out_str;
        // receive a request from client
        socket.recv(request, zmq::recv_flags::none);
        auto data_rq = request.to_string();
        std::cout << "start req" << std::endl;
        std::cout << data_rq << " siae = " << data_rq.size() << std::endl;
        std::cout << "end req" << std::endl;

        in << data_rq << std::endl;
        reader.AddDataFrame(in); // add to transport catalogue
        reader.DocumentPrinter(out_str);
        // send the reply to the client
        auto data = out_str.str();
        std::cout << data << " size = " << data.size() << std::endl;
        socket.send(zmq::buffer(data), zmq::send_flags::none);
    }
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

    transport_data_base::TransportCatalogueSerialization serialization(catalogue,
                                                                       renderer,
                                                                       router);

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
    else if (mode == "zmq") {
        ZMQMessage(reader, input, out);
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