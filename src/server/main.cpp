// SPDX-License-Identifier: GPL-3.0-only

#include <blamite/engine.hpp>

Blamite::Engine::Engine blamite_engine;    

int main(int argc, const char **argv) {
    int port = 2302;
    if(argc > 1) {
        port = atoi(argv[1]);
    }

    blamite_engine.init_server(port);
    blamite_engine.start();

    return 0;
}
