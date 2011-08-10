#include "config.h"

lazor_config_t lazor_config;
main_config_t main_config;
network_config_t network_config;
simple_pkt_t g_pattern_packet;




pthread_mutex_t c_mutex = PTHREAD_MUTEX_INITIALIZER;


int config_hivemind(const char** arguments, int nb) {

    printf("Begin Hive config.\n");

    int i;
    for (i=0;i<nb;i++) {
        printf("[%d] -> %s",i,arguments[i]);
    }

    if (nb > 0) {

        char delims[] = ":";
        char* server = NULL;
        char* port = NULL;
        char* chan = NULL;

        printf(".\n");

        server = strtok( (char*) arguments[0], delims );
        port = strtok( (char*) NULL, delims );

        printf("!\n");

        if (server == NULL || port == NULL) {
            if (nb > 1) {
                server = (char*) malloc( sizeof(char) * ( strlen(arguments[0]) + 1 ) );
                strcpy(server,arguments[0]);
                port = (char*) malloc( sizeof(char) * ( strlen(arguments[1]) + 1 ) );
                strcpy(port,arguments[1]);

            }
            else {
                loic_error("Hivemind : Too few arguments.",CONSOLE);
                return 1;
            }

            if (nb > 2) {
                chan = (char*) malloc( sizeof(char) * ( strlen(arguments[2]) + 1 ) );
                strcpy(chan,arguments[2]);
            }

        }
        else if (nb > 1 ) {
            chan = (char*) malloc( sizeof(char) * ( strlen(arguments[1]) + 1 ) );
            strcpy(chan,arguments[1]);
        }

        if (chan == NULL) {
            chan = (char*) malloc( sizeof(char) * ( strlen("#loic") + 1 ) );
            strcpy(chan,"#loic");
        }


        printf("Hive configured for %s:%s channel %s.\n",server,port,chan);

        start_hivemind(server,atoi(port),chan);

        return 0;


    }

    return 1;

}


int thread_canon() {
    return start_canon();
}


int config_from_args_irc(const char** arguments, int nb, int from) {

    char delims[] = "=";
    char* key = NULL;
    char* value = NULL;
    int retour = 0;

    int i;
    for (i=1; i < nb; i++) {

        key = strtok( (char*) arguments[i], delims );
        value = strtok( (char*) NULL, delims );

        printf("For key : %s\n",key);

        if (key != NULL && value != NULL) {

            printf("%s -- %s\n",key,value);

            // Check host ?
            if ( p_strcmpi(key,"TARGETIP") == 0 ) {
                setTarget(value);
            }

            else if ( p_strcmpi(key,"METHOD") == 0 ) {
                setProtocol(value);
            }

            else if ( p_strcmpi(key,"PORT") == 0  ) {
                setPort(atoi(value));
            }

            /* NEW */
            else if ( p_strcmpi(key,"TIMEOUT") == 0  ) {
                setTimeout(atoi(value));
            }
            /* END NEW */

            else if ( p_strcmpi(key,"MESSAGE") == 0 ) {
                setUdpMessage(value);
            }

            else if ( p_strcmpi(key,"WAIT") == 0  ) {

                if ( p_strcmpi(value,"YES") == 0 || p_strcmpi(value,"TRUE") == 0 || atoi(value) > 1 )
                    lazor_config.wait = 1;
                else if ( p_strcmpi(value,"NO") == 0 || p_strcmpi(value,"FALSE") == 0 || atoi(value) < 1 )
                    lazor_config.wait = 0;

            }

            else if (  p_strcmpi(key,"RANDOM") == 0  ) {

                if ( p_strcmpi(value,"YES") == 0 || p_strcmpi(value,"TRUE") == 0 || atoi(value) > 1 )
                    lazor_config.random = 1;
                else if ( p_strcmpi(value,"NO") == 0 || p_strcmpi(value,"FALSE") == 0 || atoi(value) < 1 )
                    lazor_config.random = 0;

            }

            key = NULL;
            value = NULL;

            #ifdef GTK_GUI
                refresh_opts();
            #endif
        }
        else if (i == nb-1 && key != NULL) {

            if ( p_strcmpi(key,"START") == 0 || p_strcmpi(key,"GO") == 0 ) {
                if (getStatus() == READY) {
                    pthread_t canon_thread;
                    pthread_create(&canon_thread,NULL,(void*)thread_canon,(void*)NULL);

                }
                else
                    loic_error("Already firing",from);
            }

            else if ( p_strcmpi(key,"STOP") == 0 || p_strcmpi(key,"PAUSE") == 0 ) {
                if (getStatus() == FIRING)
                    stop_canon();
                else
                    loic_error("Not firing",from);

            }

            else if ( p_strcmpi(key,"DEFAULT") == 0 || p_strcmpi(key,"RESET") == 0 ) {
                if (getStatus() == READY)
                    reset_config();
                else
                    loic_error("Not firing",from);

            }

        }


    }


    return retour;
}



int config_from_args(const char** arguments, int nb, int from) {

    int configured = 0;

    if (from == IRC_COMMAND) {
        configured = config_from_args_irc(arguments,nb,from)+1;
    }


    if (configured != 1) {

        int i;

        for (i=1; i < nb; i++) {

            // << --hive server:port #chan >> or << --hive server port #chan >>
            if ( ( p_strcmpi(arguments[i],"--hive") == 0 || p_strcmpi(arguments[i],"--hive_mode") == 0 || p_strcmpi(arguments[i],"-h") == 0 ||
                   p_strcmpi(arguments[i],"--hivemind") == 0 || p_strcmpi(arguments[i],"--hivemind_mode") == 0 )/* ||
                   p_strcmpi(arguments[i],"--hidden") == 0 || p_strcmpi(arguments[i],"--hidden_mode") == 0 ) */
                   && from == CONSOLE ) {


                if (nb >= i+2) {
                    char** argHive = (char**) malloc(sizeof(char)*(nb-i)*ARG_SIZE);
                    int j;

                    for (j=0; j < nb-i-1; j++) {
                        argHive[j] = (char*) malloc(sizeof(char)*ARG_SIZE);
                        strcpy(argHive[j],arguments[i+j+1]);
                    }

                    config_hivemind((const char**)argHive,j);


                }
                else
                    loic_error("Bad Hive args",from);

            }

            else if ( p_strcmpi(arguments[i],"UDP") == 0 || atoi(arguments[i]) == -2 )
                lazor_config.method = UDP;

            else if ( p_strcmpi(arguments[i],"HTTP") == 0 || atoi(arguments[i]) == -4 )
                lazor_config.method = HTTP;

            else if ( p_strcmpi(arguments[i],"SLOWLORIS") == 0 || p_strcmpi(arguments[i],"SLOWLOIC") == 0 || atoi(arguments[i]) == -5 )
                lazor_config.method = SLOW_HTTP;

            else if ( p_strcmpi(arguments[i],"SYN") == 0 || p_strcmpi(arguments[i],"SYNFLOOD") == 0 || atoi(arguments[i]) == -6 )
                lazor_config.method = SYN;

            else if ( p_strcmpi(arguments[i],"TCP") == 0 || atoi(arguments[i]) == -1 )
                lazor_config.method = TCP;


            else if ( p_strcmpi(arguments[i],"--target") == 0 || p_strcmpi(arguments[i],"-t") == 0 ) {
                i++;
                if ( i >= nb ||  setTarget(arguments[i]) != 0 ) {

                    loic_error("Syntax Error !",from);
                }
            }

            else if ( p_strcmpi(arguments[i],"--port") == 0 || p_strcmpi(arguments[i],"-p") == 0 ) {
                i++;
                if ( i >= nb ||  setPort( atoi(arguments[i]) ) != 0 ) {

                    loic_error("Syntax Error !",from);
                }
            }

            else if ( p_strcmpi(arguments[i],"--speed") == 0 || p_strcmpi(arguments[i],"-s") == 0 ) {
                i++;
                if ( i >= nb ||  setSpeed( atoi(arguments[i]) ) != 0 ) {

                    loic_error("Syntax Error !",from);
                }
            }

            else if ( p_strcmpi(arguments[i],"--threads") == 0 || p_strcmpi(arguments[i],"-nt") == 0 ) {
                i++;
                if ( i >= nb ||  setNbThreads( atoi(arguments[i]) ) != 0 ) {

                    loic_error("Syntax Error !",from);
                }
            }

            else if ( p_strcmpi(arguments[i],"--timeout") == 0 || p_strcmpi(arguments[i],"-to") == 0 ) {
                i++;
                if ( i >= nb ||  setTimeout( atoi(arguments[i]) ) != 0 ) {

                    loic_error("Syntax Error !",from);
                }
            }

            else if ( p_strcmpi(arguments[i],"--udp-message") == 0 || p_strcmpi(arguments[i],"-m") == 0  ) {
                i++;
                if ( i >= nb ||  setUdpMessage(arguments[i]) != 0 ) {

                    loic_error("Syntax Error !",from);
                }
            }

            else if ( p_strcmpi(arguments[i],"--http-subsite") == 0 || p_strcmpi(arguments[i],"-hs") == 0  ) {
                i++;
                if ( i >= nb ||  setSubsite(arguments[i]) != 0 ) {

                    loic_error("Syntax Error !",from);
                }
            }

            else if ( p_strcmpi(arguments[i],"--wait-for-response") == 0 || p_strcmpi(arguments[i],"-w") == 0  )
                    lazor_config.wait = TRUE;

            else if ( p_strcmpi(arguments[i],"--random") == 0 || p_strcmpi(arguments[i],"-r") == 0  )
                    lazor_config.random = TRUE;

            else if ( p_strcmpi(arguments[i],"--gzip") == 0 || p_strcmpi(arguments[i],"-gz") == 0  )
                    lazor_config.gzip = TRUE;

            else if ( p_strcmpi(arguments[i],"--slowloris-get") == 0 || p_strcmpi(arguments[i],"-get") == 0  )
                    lazor_config.useGet = TRUE;

            else if ( p_strcmpi(arguments[i],"--method") == 0 || p_strcmpi(arguments[i],"--protocol") == 0  ) {
                i++;
                if ( i >= nb ||  setProtocol(arguments[i]) != 0 ) {

                    loic_error("Syntax Error !",from);
                }
            }

            /* Super Mode */


            else if ( p_strcmpi(arguments[i],"--super") == 0 || atoi(arguments[i]) == -1 )
                main_config.super = TRUE;

            #ifdef SUPER_LOIC

                else if ( p_strcmpi(arguments[i],"-listen") == 0 ) {
                        i++;
                        char* dev =  c_get_interface_name_from_index( atoi(arguments[i]) );
                        if ( i <= nb ) {

                            int c=0;

                            c_get_packet(dev);
                            printf("-%d-\n",c);
                            c++;


                            //loic_error("Interface Error !",from);
                        }

                }

                else if ( p_strcmpi(arguments[i],"-charge") == 0 ) {

                        charge_superCanon();

                }

                /* End Super Mode */



                else if ( p_strcmpi(arguments[i],"--interface") == 0 || p_strcmpi(arguments[i],"-i") == 0  ) {
                    i++;
                    if ( i >= nb ||  setNetworkInterface(arguments[i]) != 0 ) {

                        loic_error("Syntax Error !",from);
                    }
                }

                else if ( p_strcmpi(arguments[i],"--ip-spoof") == 0 || p_strcmpi(arguments[i],"-A") == 0  ) {
                    i++;
                    if ( i >= nb ||  setSpoofedIp(arguments[i]) != 0 ) {

                        loic_error("Syntax Error !",from);
                    }
                }

            #endif


            else if ( strcmp(getTarget(),"") == 0 ) {
                setTarget(arguments[i]);
            }

            else {

                loic_error("Syntax Error !",from);
            }


        }

        configured = 1;
    }

    if ( strcmp(getTarget(),"") == 0 ) {
        loic_error("You have not defined the target.",from);
    }
    else
        setStatus(READY);

    print_config();

    return 0;
}

int reset_config() {


    if (getTarget() == NULL) {
        lazor_config.target = (char*) malloc(sizeof(lazor_config.target)*128);
        strcpy(lazor_config.target,"");
        main_config.status = EMPTY;
    }

    lazor_config.message = (char*) malloc(sizeof(lazor_config.message)*1024);
    strcpy(lazor_config.message,"hello");

    lazor_config.subsite = (char*) malloc(sizeof(lazor_config.subsite)*1024);
    strcpy(lazor_config.subsite,"/");

    /* Super Mode */
    network_config.interface_index = 0;
    network_config.interface_name = (char*) malloc(sizeof(network_config.interface_name)*2048);
    network_config.configured = FALSE;

    strcpy(network_config.interface_name,"no_interface");

    u_char* pdata = (u_char*) malloc (sizeof(u_char) * 100 );

    g_pattern_packet.pkt_data = pdata;
    network_config.pattern_packet = &g_pattern_packet;

    lazor_config.spoof = FALSE;
    lazor_config.sPort = -1;
    /* End Super Mode */

    main_config.nb_threads = 10;
    main_config.timeout = 9999;
    main_config.super = FALSE;

    lazor_config.port = 80;
    lazor_config.method = UDP;
    lazor_config.speed = 100;
    lazor_config.wait = FALSE;
    lazor_config.random = FALSE;
    lazor_config.gzip = FALSE;



    return 0;


}


void print_config() {
        printf("\nLazor configuation : \n");
        printf("\t- Target -> %s\n\t- Port -> %d\n\t- Method -> %d\n\t- Speed -> %d\n\t- Message -> %s\n\t- Wait -> %d\n\t- Random -> %d\n",lazor_config.target, lazor_config.port, lazor_config.method, lazor_config.speed, lazor_config.message, lazor_config.wait, lazor_config.random);
        printf("\t- Threads -> %d\n\n",main_config.nb_threads);
}

int setNbThreads(int input) {

    if (input > 0) {

        if (input > 50)
            input = 50;

        main_config.nb_threads = input;

        return 0;

    }

    return 1;
}


int setStatus(int input) {

    if (input > -1) {
        main_config.status = input;
        return 0;

    }

    return 1;
}

int setTimeout(int input) {

    if (input > -1) {
        main_config.timeout = input;
        return 0;

    }

    return 1;
}


int setTarget(const char* input) {

    if (input != NULL) {
        strcpy(lazor_config.target,input);
        return 0;

    }

    return 1;
}

int setPort(const int input) {

    if (input > 0 && input < 99999) {
        lazor_config.port = input;
        return 0;
    }

    return 1;

}

int setSpeed(int input) {

    if (input > 0) {

        if (input > 100) {
            input = 200;
        }

        lazor_config.speed = input;

        return 0;


    }

    return 1;
}

int setUdpMessage(const char* input) {

    if (input != NULL) {
        strcpy(lazor_config.message,input);
        return 0;
    }

    return 1;

}

int setSubsite(const char* input) {

    if (input != NULL) {
        strcpy(lazor_config.subsite,input);
        return 0;
    }

    return 1;

}

int setProtocol(const char* input) {

    if (input != NULL) {

        if      ( p_strcmpi(input,"UDP") == 0 || atoi(input) == 2 )
            lazor_config.method = UDP;
        else if ( p_strcmpi(input,"HTTP") == 0 || atoi(input) == 4 )
            lazor_config.method = HTTP;
        else if ( p_strcmpi(input,"TCP") == 0 || atoi(input) == 1 )
            lazor_config.method = TCP;
        else
            return 1; // Invalid argument

        return 0;
    }

    return 1;
}

int setProtocoli(int proto) {

    if (proto != -1) {
        lazor_config.method = proto;

        return 0;
    }

    return 1;
}

int setGzipEnabled(p_BOOL val) {
    lazor_config.gzip = val;

    return 0;
}

int setRandomCharsEnabled(p_BOOL val) {
    lazor_config.random = val;

    return 0;
}

int setWaitEnabled(p_BOOL val) {
    lazor_config.wait = val;

    return 0;
}

int setUseGetEnabled(p_BOOL val) {
    lazor_config.useGet = val;

    return 0;
}


int getStatus() {
    return main_config.status;
}

int getTimeout() {
    return main_config.timeout;
}

int getNbThreads() {
    return main_config.nb_threads;
}

char* getTarget() {
    return lazor_config.target;
}

int getPort() {
    return lazor_config.port;
}

int getSpeed() {
    return lazor_config.speed;
}

char* getUdpMessage() {
    return lazor_config.message;
}

char* getSubsite() {
    return lazor_config.subsite;
}

int getProtocol() {
    return lazor_config.method;
}

p_BOOL isWaitEnabled() {
    return lazor_config.wait;
}

p_BOOL isRandomEnabled() {
    return lazor_config.random;
}

p_BOOL isGzipEnabled() {
    return lazor_config.gzip;
}

p_BOOL useSlowGet() {
    return lazor_config.useGet;
}


p_BOOL isSuperMode() {
    return main_config.super;
}




#ifdef SUPER_LOIC


    /* Super Mode */


    const char* getInterfaceName() {
        return network_config.interface_name;
    }

    u_char getInterfaceIndex() {
        return network_config.interface_index;
    }

    mac_address getInterfaceMac() {
        return network_config.interface_mac;
    }

    ip_address getInterfaceIp() {
        return network_config.interface_ip;
    }

    mac_address getRouterMac() {
        return network_config.router_mac;
    }

    simple_pkt_t* getPatternPacket() {
        return network_config.pattern_packet;
    }




    int setNetworkInterface(const char* input) {
        int index = atoi(input);
        if ( index > 0 && index < 64 ) {

            int ret = setInterfaceName( c_get_interface_name_from_index(index) );

            if ( ret == 0 ) {
                setInterfaceIndex( (u_char) index );
            }

            return ret;

        }
        else if ( strlen(input) > 0 ){
            return setInterfaceName(input);
        }
        else
            return 1;
    }

    int setInterfaceIndex(u_char index) {
        if (index > 0) {
            network_config.interface_index = index;
            return 0;
        }

        return 1;
    }


    int setInterfaceName(const char* input) {
        if (strlen(input) > 0) {
            strcpy(network_config.interface_name, input);
            return 0;
        }

        return 1;
    }


    int setInterfaceMac(mac_address mac) {

        network_config.interface_mac.byte1 = mac.byte1;
        network_config.interface_mac.byte2 = mac.byte2;
        network_config.interface_mac.byte3 = mac.byte3;
        network_config.interface_mac.byte4 = mac.byte4;
        network_config.interface_mac.byte5 = mac.byte5;
        network_config.interface_mac.byte6 = mac.byte6;

        return 0;
    }

    int setInterfaceIp(ip_address ip) {

        network_config.interface_ip.byte1 = ip.byte1;
        network_config.interface_ip.byte2 = ip.byte2;
        network_config.interface_ip.byte3 = ip.byte3;
        network_config.interface_ip.byte4 = ip.byte4;

        return 0;
    }

    int setRouterMac(mac_address mac) {

        network_config.router_mac.byte1 = mac.byte1;
        network_config.router_mac.byte2 = mac.byte2;
        network_config.router_mac.byte3 = mac.byte3;
        network_config.router_mac.byte4 = mac.byte4;
        network_config.router_mac.byte5 = mac.byte5;
        network_config.router_mac.byte6 = mac.byte6;

        return 0;
    }

    int setSpoofedIp(const char* input) {
        ip_address sip = string2ip(input);
        if (sip.byte1 > 0) {
            lazor_config.spoofed_ip = sip;
            lazor_config.spoof = TRUE;
            return 0;
        }
        else
            return 1;
    }

    int setNetworkReady() {
        network_config.configured = TRUE;

        return 0;
    }

    int setPatternPacket(const u_char* pkt_data, int size) {

        pthread_mutex_lock(&c_mutex);


            print_packet(pkt_data,size);

            network_config.pattern_packet->size = size;

            network_config.pattern_packet->pkt_data = (u_char*) malloc( sizeof(u_char) * (size) );

            int m;
            for (m=0; m < network_config.pattern_packet->size; m++) {
               network_config.pattern_packet->pkt_data[m] = pkt_data[m];
            }

            print_packet(network_config.pattern_packet->pkt_data, network_config.pattern_packet->size);


            if (network_config.pattern_packet->size == size) {

                setNetworkReady();

                pthread_mutex_unlock(&c_mutex);
                return 0;

            }

        pthread_mutex_unlock(&c_mutex);

        return 1;

    }

    int setSourcePort(int input) {
        if ( input > -2 && input < 25565 ) {
            lazor_config.sPort = input;
            return 0;
        }

        return 1;
    }




    int checkInterfaceMac(mac_address mac) {

        return (
               network_config.interface_mac.byte1 == mac.byte1
            && network_config.interface_mac.byte2 == mac.byte2
            && network_config.interface_mac.byte3 == mac.byte3
            && network_config.interface_mac.byte4 == mac.byte4
            && network_config.interface_mac.byte5 == mac.byte5
            && network_config.interface_mac.byte6 == mac.byte6
            );

    }

    int checkInterfaceIp(ip_address ip) {
        return (
               network_config.interface_ip.byte1 == ip.byte1
            && network_config.interface_ip.byte2 == ip.byte2
            && network_config.interface_ip.byte3 == ip.byte3
            && network_config.interface_ip.byte4 == ip.byte4
            );

    }

    int checkRouterMac(mac_address mac) {

        return (
               network_config.router_mac.byte1 == mac.byte1
            && network_config.router_mac.byte2 == mac.byte2
            && network_config.router_mac.byte3 == mac.byte3
            && network_config.router_mac.byte4 == mac.byte4
            && network_config.router_mac.byte5 == mac.byte5
            && network_config.router_mac.byte6 == mac.byte6
            );

    }

    p_BOOL isNetworkReady() {
        return network_config.configured;
    }

    p_BOOL isIPSpoofingEnabled() {
        return lazor_config.spoof;
    }

    ip_address getSpoofedIp() {
        if ( isIPSpoofingEnabled() )
            return lazor_config.spoofed_ip;
        else {
            ip_address rip;
            rip.byte1 = 0;
            return rip;
        }

    }

    int getSourcePort() {
        return lazor_config.sPort;
    }

    p_BOOL isRandomSourcePort() {
        if ( getSourcePort() == -1 ) {
            return TRUE;
        }

        return FALSE;
    }

    /* End Super Mode */
#endif
