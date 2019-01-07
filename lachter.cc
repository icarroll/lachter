#include <iostream>

#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>

#include "alphabeta.hh"

int main(int numargs, char * args[]) {
    bool done = false;
    std::string input;

    while (!done) {
        std::cout << "Enter Command: ";
        std::getline(std::cin, input);

        if (input == "quit") {
            done = true;
        } else if (input == "help") {
            std::cout
                << "\nCommand List:\n"
                << "help: Display this help text\n"
                << "quit: Exit the program\n"
                << std::endl;
        } else {
            std::cout << "Unrecognized Command" << std::endl;
        }
    }

    return 0;
}
