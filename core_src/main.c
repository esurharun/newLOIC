#include "config.h"
#include "overlord.h"
#include <time.h>



int test_proxy() {

    SOCKET s = createSocket(TCP);

    s_connect(s,"189.11.196.220",3128);


    s_send(s,"CONNECT perdu.com:80 HTTP/1.1\n Host: perdu.com:80\nUser-Agent: TEST\n");
    s_send(s,"\n");

    char buffer[4096];
    s_recv(s,buffer,sizeof(buffer));

    printf("P--%s\n",buffer);

    s_send(s,"GET http://perdu.com/ HTTP/1.1\n Host: perdu.com\nUser-Agent: TEST\n\n");

    s_recv(s,buffer,sizeof(buffer));
    printf("PP----%s\n",buffer);

    return 0;
}

int main(int argc, const char** args) {

    srand( time(NULL) );

    reset_config();


    init(); /* Use sockets > Windows */



    if (argc > 1)
        config_from_args(args,argc,CONSOLE);
    else
        print_help();


    if ( !isSuperMode() ) {
        start_canon();
    }
    #ifdef SUPER_LOIC
        else {
            /* Super Mode */
            charge_superCanon();

            start_superCanon();
        /* End Super Mode */
        }
    #endif


    end(); /* End of ' Use sockets > Windows ' */

     return 0;
}

/* MEMO
    GET / HTTP/1.1\nHost: perdu.com\nReferer: http://example.com/\nUser-Agent: TEST\n\n
 */
