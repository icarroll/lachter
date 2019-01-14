#include <cassert>
#include <iostream>
#include <sstream>

extern "C" {
#include <unistd.h>

#include <libwebsockets.h>
}

#include "alphabeta.hh"

static const struct lws_protocols protocols[] = {
    {
        "thurisaz",
        NULL, //XXX callback_thurisaz,
        0xdeadbeef, //XXX sizeof(WHUT)
        0,
    },
    { NULL, NULL, 0, 0 }
};

bool iam_dwarf;

void which_side(string message) {
    if (message == "play_dwarf") iam_dwarf = true;
    else if (message == "play_troll") iam_dwarf = false;
    else {
        cout << "bad message: " << message << endl;
    }
}

gamestate state;
alphabeta_brain brain(state);


string theirmove_str;
void handle_message(string message) {
    theirmove_str = message;
}

int main(int numargs, char * args[]) {
    string url = "ws://localhost:20821/";

    struct lws_client_connect_info i;

    struct lws_context_creation_info info;
    memset(& info, 0, sizeof(info));
    info.options = 0;
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;

    struct lws_context * context;
    context = lws_create_context(& info);

    //-----

    gamemove mymove;
    stringstream mymove_str;
    gamemove theirmove;

    if (iam_dwarf) goto dwarf_start;
    else goto troll_start;

    while (true) {
    dwarf_start:
        sleep(5);

        cout << "start thinking" << endl;
        brain.think_depth(4);
        cout << "thinking done" << endl;

        mymove = brain.best_move();
        brain.do_move(mymove);

        cout << "I did " << mymove << endl;
        mymove_str.str("");
        mymove_str << mymove;

        // SEND
        cout << "I said " << mymove_str.str() << endl;
    troll_start:
        // RECEIVE
        cout << "they said " << theirmove_str << endl;

        stringstream(theirmove_str) >> theirmove;
        brain.do_move(theirmove);

        cout << "they did " << theirmove << endl;
        sleep(1);
    }

    // DISCONNECT

    return 0;
}
