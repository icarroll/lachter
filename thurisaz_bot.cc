#include <cassert>
#include <iostream>

#include "easywsclient.hpp"
using easywsclient::WebSocket;

#include "alphabeta.hh"


void handle_message(string message) {
    cout << message << endl;
}

int main(int numargs, char * args[]) {
    string url = "ws://localhost:20821/";

    WebSocket::pointer ws = WebSocket::from_url(url);
    assert(ws);

    ws->poll(-1);
    ws->dispatch([](const string & message){handle_message(message);});
    ws->send("T J9-K9");
    ws->poll(-1);
    ws->dispatch([](const string & message){handle_message(message);});

    delete ws;
    return 0;
}
