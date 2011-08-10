#ifndef H_CONST_H

    #define H_CONST_H


    typedef char p_BOOL;

    #ifndef TRUE
        #define TRUE 1
    #endif

    #ifndef FALSE
        #define FALSE 0
    #endif


    #define TCP 1
    #define HTTP 11
    #define SLOW_HTTP 12
    #define UDP 2
    #define SYN 7
    #define SUDP 8
    #define TSYN 99

    #define CONSOLE 0
    #define IRC_COMMAND 1
    #define MINOR 2
    #define GRAVE 3

    #define EMPTY -1
    #define READY 0
    #define FIRING 1
    #define STOPPING 2
    #define TIMEOUT 3
    #define PAUSED 4
    #define HIVEMIND_READY 10
    #define HIVEMIND_CONNECTING 11
    #define HIVEMIND_ERROR 12

    #define ARG_SIZE 256

    #include <stdio.h>
    #include <ctype.h>
    #include <stdlib.h>

#ifndef u_char
typedef unsigned char u_char;
#endif

    /* 6 bytes MAC address */
    typedef struct  {
        u_char byte1;
        u_char byte2;
        u_char byte3;
        u_char byte4;
        u_char byte5;
        u_char byte6;
    } mac_address;

    /* 4 bytes IP address */
    typedef struct  {
        u_char byte1;
        u_char byte2;
        u_char byte3;
        u_char byte4;
    } ip_address;

    //#define RAND_MAX 1337


    #ifdef WIN32

        #include <winsock2.h>

    #elif defined (linux)

        #define pcap_open(a,b,c,d,e,f) pcap_open_live(a,b,c,d,f)

        #include <string.h>
        #include <errno.h>
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>

            /* B RAW */
        #include <netinet/ip.h>
        #include <netinet/tcp.h>
            /* E RAW */

        #include <unistd.h> /* close */
        #include <netdb.h> /* gethostbyname */
        #define INVALID_SOCKET -1
        #define SOCKET_ERROR -1
        #define closesocket(s) close(s)
        #ifndef SOCKET
            typedef int SOCKET;
        #endif
        typedef struct sockaddr_in SOCKADDR_IN;
        typedef struct sockaddr SOCKADDR;
        typedef struct in_addr IN_ADDR;

    #else

        #error not defined for this platform

    #endif


#endif
