#include "overlord.h"


int getWebPage(char* url, int port, char* buffer_response, int size) {

    char sent_data[1024];

    SOCKET s = createSocket(TCP);
    char* host = p_host_from_url(url);
    s_connect(s,host,port);


    sprintf(sent_data,"GET %s HTTP/1.1\nHost: %s\nReferer: %s\nUser-Agent: %s\n\n",
                        url,host,"http://exame.com","Mozilla/5.0 (Windows NT 6.1; WOW64; rv:5.0) Gecko/20100101 Firefox/5.0");

    //printf("Send : %s\n",sent_data);
    if( send(s, sent_data, strlen(sent_data), 0) < 0 ) {
        printf("Connection error.\n");
        return 1;
    }

    s_recv(s,buffer_response,size);


    closesocket(s);


    return 0;

}
