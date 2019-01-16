#include <cassert>
#include <iostream>
#include <sstream>

extern "C" {
#include <unistd.h>

#include <libwebsockets.h>
}

#include "alphabeta.hh"

int interrupted, rx_seen, test;
struct lws * client_wsi;

int callback_thurisaz(struct lws * wsi, enum lws_callback_reasons reason,
                      void * user, void * in, size_t len) {
    switch (reason) {
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        lwsl_err("CLIENT_CONNECTION_ERROR: %s\n",
             in ? (char *)in : "(null)");
        client_wsi = NULL;
        break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        lwsl_user("%s: established\n", __func__);
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        lwsl_user("RX: %s\n", (const char *)in);
        rx_seen++;
        if (test && rx_seen == 10)
            interrupted = 1;
        break;

    case LWS_CALLBACK_CLIENT_CLOSED:
        client_wsi = NULL;
        break;

    default:
        break;
    }

    return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static const struct lws_protocols protocols[] = {
    {
        "thurisaz",
        callback_thurisaz,
        0,
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
    //string url = "ws://localhost:20821/";

    lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE, NULL);

    struct lws_context_creation_info info;
    memset(& info, 0, sizeof(info));
    info.options = 0;
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;

    struct lws_context * context;
    context = lws_create_context(& info);

    struct lws_client_connect_info i;
    memset(& i, 0, sizeof(i));
    i.context = context;
    i.port = 20821;
    i.address = "localhost";
    i.path = "/";
    i.host = i.address;
    i.origin = i.address;
    i.ssl_connection = 0;
    i.protocol = NULL;
    i.pwsi = & client_wsi;

    lws_client_connect_via_info(& i);

    while (client_wsi) {
        lws_service(context, 1000);
    }

    lws_context_destroy(context);

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
