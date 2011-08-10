#ifndef H_SUPERGEARS_H

    #define HAVE_REMOTE

    #include "config.h"
    #include <stdlib.h>
    #include <pcap.h>


    #define IPTOSBUFFERS    12

    int c_print_devices_list();
    pcap_if_t* c_get_device_from_index(int index) ;
    char* c_get_interface_name_from_index(int index);
    char* c_get_interface_hname_from_index(int index);
    char* get_custom_string_from_device(int index);
    char* get_ip_from_custom_device( int my_index );
    char* c_get_ip_from_device(pcap_if_t* d);
    int c_send_packet(char* adapter);
    int super_syn_fire(char* adapter);
    void c_send_packet_queue(char *adapter);
    void queue_from_file(char* device, char* filename);
    int c_get_packet(char* device);
    void c_infos_from_packet(const struct pcap_pkthdr* header, const u_char* pkt_data) ;

    void print_packet(const u_char* packet, int p_len);
    int c_devices_count();

    int c_get_pattern_packet(char* args[3]);
    char* c_ip_from_device(pcap_if_t* d);
    int send_pattern_packet(const char* dst, int port);
    int c_config_pattern_packet( const u_char* pkt_data, int size );
    int c_check_pattern_packet( const u_char* pkt_data, int size );

    int pktcpy(u_char* out, const u_char* in, int size);

    int c_forge_packet(u_char* pkt_out, int type);

    mac_address string2mac(const char* str);
    ip_address string2ip(const char* str);
    char* ip2string( ip_address IPin );
    ip_address randIP(int cat);

    void print_custom_device_list();

    int generate_devices_list();
    int get_custom_device_count();
    char* get_ip_from_custom_device( int my_index );
    int get_myindex_from_pcapindex( int pcap_index );
    char* get_custom_string_from_device( int my_index );


#endif
