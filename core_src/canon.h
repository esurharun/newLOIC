#ifndef H_CANON_H

    #define H_CANON_H


    #include <pthread.h>

    #include "gears.h"
    #include "config.h"

    int fire(int id);

    int getNbPacketsSent(int nt);

    int http_fire(int id);
    int http_fire_classic(int id);
    int udp_fire(int id);
    int tcp_fire(int id);
    int slowloris_fire(int id);
    int syn_fire(int id);

    int start_canon();
    int stop_canon();

    int oversee_timeout(int time);

#endif
