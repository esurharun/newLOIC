#ifndef H_HIVEMIND_H

    #define H_HIVEMIND_H

    #include "const.h"
    #include "gears.h"
    #include "config.h"

    #include "../include/libircclient/include/libircclient.h"

    int disconnect();
    int start_hivemind(const char* server, int port, const char* chan);


#endif
