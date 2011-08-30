#ifndef H_CONFIG_H

    #define H_CONFIG_H

    #include "canon.h"
    #include "hivemind.h"

    #include <string.h>
    #include <pthread.h>
    #include <time.h>
    #include <stdlib.h>

    #ifdef GTK_GUI
        #include "gtk_module.h"
    #endif
    #ifdef SUPER_LOIC
        /* Super mode */

        #include <pcap.h>


            #ifdef WIN32
                #include "Win32-Extensions.h"
                #define PCAP_SRC_FILE_STRING "file://"
                #define PCAP_SRC_IF_STRING "rpcap://"
                #define PCAP_ERRBUF_SIZE 256
            #endif

        typedef struct {
                    u_int bf_len;
                    struct bpf_insn* bf_insns;
            } bpf_program;





        /* Ethernet header */
        typedef struct {
            mac_address dmac;
            mac_address smac;
        } eth_header;

        /* IPv4 header */
        typedef struct {
            u_char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
            u_char  tos;            // Type of service
            u_short tlen;           // Total length
            u_short identification; // Identification
            u_short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
            u_char  ttl;            // Time to live
            u_char  proto;          // Protocol
            u_short crc;            // Header checksum
            ip_address  saddr;      // Source address
            ip_address  daddr;      // Destination address
            u_int   op_pad;         // Option + Padding
        } ip_header;



        /* UDP header*/
        typedef struct  {
            u_short sport;          // Source port
            u_short dport;          // Destination port
            u_short len;            // Datagram length
            u_short crc;            // Checksum
        } udp_header;

        /* TCP header */
        typedef u_int32_t tcp_seq;
        typedef struct {
                u_short th_sport;	/* source port */
                u_short th_dport;	/* destination port */
                tcp_seq th_seq;		/* sequence number */
                tcp_seq th_ack;		/* acknowledgement number */
                u_char th_offx2;	/* data offset, rsvd */
                u_char th_flags;
                u_short th_win;		/* window */
                u_short th_sum;		/* checksum */
                u_short th_urp;		/* urgent pointer */
        } tcp_header;



         #include "superCanon.h"
    #endif



typedef struct {
    p_BOOL configured;
    u_char interface_index;
    char*  interface_name;
    mac_address interface_mac;
    ip_address interface_ip;
    mac_address router_mac;
    simple_pkt_t* pattern_packet;
} network_config_t;

typedef struct {
    int interface_index;
    char* interface_name;
    ip_address interface_ip;
} interface_t;

typedef struct {
    int value;
    char* url;
} mtarget_t;



typedef struct {
        char*   target;
        string_list_t   message;
        string_list_t   subsite;
        int     port;
        int     method;
        int      speed;
        p_BOOL   wait;
        p_BOOL   random;
        p_BOOL   gzip;
        p_BOOL   useGet;

        /* Super Mode */
        p_BOOL   spoof;
        ip_address spoofed_ip;
        int      sPort;
        /* End Super Mode */
} lazor_config_t;


typedef struct {
        char* hive_server;
        int   hive_port;
        char* hive_channel;

        char* overlord_url;
        int   refresh_rate;

        int   nb_threads;
        int   status;
        int   timeout;

        p_BOOL super;
} main_config_t;


    int config_from_args(const char** arguments, int nb, int from);
    int config_from_args_irc(const char** arguments, int nb, int from);
    int config_hivemind(const char** arguments, int nb);

    int reset_config();
    void print_config();

    int setStatus(int input);
    int setTimeout(int input);
    int setNbThreads(int input);
    int setTarget(const char* input);
    int setPort(const int input);
    int setSpeed(int input);
    int setUdpMessage(const char* input, u_char mode);
    int setSubsite(const char* input, u_char mode);
    int setProtocol(const char* input);
    int setProtocoli(int proto);
    int setGzipEnabled(p_BOOL val);
    int setRandomCharsEnabled(p_BOOL val);
    int setWaitEnabled(p_BOOL val) ;
    int setUseGetEnabled(p_BOOL val);

    int getStatus();
    int getTimeout();
    int getNbThreads();
    char* getTarget();
    int getPort();
    int getSpeed();
    char* getUdpMessage();
    char* getSubsite();
    int getProtocol();
    p_BOOL isWaitEnabled();
    p_BOOL isRandomEnabled();
    p_BOOL isGzipEnabled();
    p_BOOL useSlowGet();
    p_BOOL isSuperMode();

    p_BOOL isInHivemind();
    void imInHivemind( p_BOOL yesornot );


    int getNbPacketsSent(int nt);
    int setNbPacketsSent(int nt, int nbp);


    int setTarget_From_Hivemind_Line( const char* str );

    #ifdef SUPER_LOIC
        /* Super Mode */

        int setNetworkReady();
        int setPatternPacket(const u_char* pkt_data, int size);


        int setNetworkInterface(const char* input);
        int setInterfaceIndex(u_char index);
        int setInterfaceName(const char* input);

        int setInterfaceMac(mac_address mac);
        int setInterfaceIp(ip_address ip);
        int setRouterMac(mac_address mac);

        int setSpoofedIp(const char* input);


        p_BOOL isNetworkReady();
        simple_pkt_t* getPatternPacket();

        const char* getInterfaceName();
        u_char getInterfaceIndex();

        mac_address getInterfaceMac();
        ip_address getInterfaceIp();
        mac_address getRouterMac();

        int checkInterfaceMac(mac_address mac);
        int checkInterfaceIp(ip_address ip);
        int checkRouterMac(mac_address mac);

        p_BOOL isIPSpoofingEnabled();
        ip_address getSpoofedIp();
    #endif


#endif
