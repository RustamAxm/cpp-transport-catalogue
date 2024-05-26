
#include <string>
#include <zmq.hpp>
#include <fstream>
#include <iostream>


int main()
{
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket{context, zmq::socket_type::req};
    socket.connect("tcp://localhost:5555");

    std::istream& input = std::cin;
    std::stringstream in_str;

    std::ifstream inputFile("../../process_request.json");

    // Check if the file is open
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file for reading." << std::endl;
        return 1;
    }

    std::stringstream ss;
    ss << inputFile.rdbuf();
    inputFile.close();
    std::string content = ss.str();

    std::cout << content << std::endl;

    for (auto request_num = 0; request_num < 1; ++request_num)
    {
        // send the request message
        std::cout << "Sending Hello " << request_num << "..." << std::endl;
        socket.send(zmq::buffer(content), zmq::send_flags::none);

        // wait for reply from server
        zmq::message_t reply{};
        socket.recv(reply, zmq::recv_flags::none);

        std::cout << "Received " << reply.to_string();
        std::cout << " (" << request_num << ")";
        std::cout << std::endl;
    }

    return 0;
}

