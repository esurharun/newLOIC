#ifndef H_SUPERCANON_H

    #define HAVE_REMOTE

    #include <pcap.h>
    #include "gears.h"
    #include "superGears.h"


    #ifdef WIN32
        #include "Win32-Extensions.h"
        #define PCAP_SRC_FILE_STRING "file://"
        #define PCAP_SRC_IF_STRING "rpcap://"
        #define PCAP_ERRBUF_SIZE 256
    #endif
int charge_superCanon();
int start_superCanon();
int test_superFire(int id) ;
int syn_superFire(int id) ;
int udp_superFire(int id);

#endif
