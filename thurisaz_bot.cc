#include <cassert>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

extern "C" {
#include <unistd.h>

#include <libwebsockets.h>
}

#include "alphabeta.hh"

// ws://localhost:20821/
const char * SERVER = "localhost";
const int PORT = 20821;
const char * PATH = "/";

struct lws * client_wsi;

bool iam_dwarf;
bool side_known = false;
mutex side_mtx;
condition_variable side_cv;

stringstream mymove_str;
bool mymove_ready = false;
mutex mymove_mtx;

string theirmove_str;
bool theirmove_received = false;
mutex theirmove_mtx;
condition_variable theirmove_cv;

thread_local int thread_id;

int callback_thurisaz(struct lws * wsi, enum lws_callback_reasons reason,
                      void * user, void * in, size_t len) {
    //cout << "called back with reason " << reason << endl;
    switch (reason) {
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        lwsl_err("CLIENT_CONNECTION_ERROR: %s\n",
             in ? (char *) in : "(null)");
        client_wsi = nullptr;
        break;

    case LWS_CALLBACK_GET_THREAD_ID:
        lwsl_user("returning thread id %d\n", thread_id);
        return thread_id;
        break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        lwsl_user("%s: established\n", __func__);
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        lwsl_user("RX: %s\n", (const char *) in);
        if (side_known) {
            unique_lock<mutex> lck(theirmove_mtx);
            theirmove_str = (const char *) in;
            theirmove_received = true;
            theirmove_cv.notify_all();
        }
        else {
            if (strcmp((const char *) in, "play_dwarf") == 0) iam_dwarf = true;
            else if (strcmp((const char *) in, "play_troll") == 0) iam_dwarf = false;
            else {
                cout << "oops, got " << (const char *) in << endl;
                exit(1); //TODO die better
            }

            unique_lock<mutex> lck(side_mtx);
            side_known = true;
            side_cv.notify_all();
        }
        break;

    case LWS_CALLBACK_CLIENT_WRITEABLE:
        {
            unique_lock<mutex> lck(mymove_mtx);
            if (mymove_ready) {
                int length = mymove_str.str().length();
                char * buf = (char *) malloc(length + LWS_PRE);
                mymove_str.str().copy(buf + LWS_PRE, length);
                lws_write(client_wsi, (unsigned char *) buf + LWS_PRE, length,
                          LWS_WRITE_TEXT);
                free(buf);
                mymove_ready = false;
            }
        }
        break;

    case LWS_CALLBACK_CLIENT_CLOSED:
        client_wsi = nullptr;
        break;

    default:
        break;
    }

    return 0;
}

static const struct lws_protocols protocols[] = {
    {
        "thurisaz",        // protocol name
        callback_thurisaz, // callback function
        0,                 // session data size
        0,                 // receive buffer size
        0,                 // id
        nullptr,           // user pointer
        0,                 // transmit packet size
    },
    {nullptr, nullptr, 0, 0, 0, nullptr, 0}
};

void websocket_magic(int new_thread_id) {
    thread_id = new_thread_id;

    lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE, nullptr);

    struct lws_context_creation_info info;
    memset(& info, 0, sizeof(info));
    info.options = 0;
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;

    struct lws_context * context;
    context = lws_create_context(& info);
    if (! context) exit(1);

    struct lws_client_connect_info i;
    memset(& i, 0, sizeof(i));
    i.context = context;
    i.address = SERVER;
    i.port = PORT;
    i.path = PATH;
    i.host = i.address;
    i.origin = i.address;
    i.ssl_connection = 0;
    i.protocol = "thurisaz";
    i.pwsi = & client_wsi;

    lws * socket = lws_client_connect_via_info(& i);
    if (! socket) exit(1);

    while (client_wsi) {
        lws_service(context, 1000);
    }

    lws_context_destroy(context);
}

int main(int numargs, char * args[]) {
    // set up the websocket connection and start the event loop
    thread_id = 0;
    thread communicate(websocket_magic, 1);

    // inside a block to ensure the lock goes out of scope and is released
    {
        unique_lock<mutex> lck(side_mtx);
        while (! side_known) side_cv.wait(lck);
    }

    gamestate state;
    alphabeta_brain brain(state);

    gamemove mymove;
    gamemove theirmove;

    if (iam_dwarf) goto dwarf_start;
    else goto troll_start;

    while (true) {
    dwarf_start:
        cout << "start thinking" << endl;
        brain.think_depth(4);
        cout << "thinking done" << endl;

        mymove = brain.best_move();
        brain.do_move(mymove);

        cout << "I did " << mymove << endl;

        // SEND
        {
            unique_lock<mutex> lck(mymove_mtx);
            mymove_str.str("");
            mymove_str << mymove;
            mymove_ready = true;
        }
        lws_callback_on_writable(client_wsi);
        cout << "I said " << mymove_str.str() << endl;
    troll_start:
        // RECEIVE
        {
            unique_lock<mutex> lck(theirmove_mtx);
            while (! theirmove_received) theirmove_cv.wait(lck);
            cout << "they said " << theirmove_str << endl;
            stringstream(theirmove_str) >> theirmove;
            theirmove_received = false;
        }

        brain.do_move(theirmove);

        cout << "they did " << theirmove << endl;
    }

    // DISCONNECT

    return 0;
}
