#include "gears.h"


// Custom error, depending on source command.
void loic_error(const char* message, int ind) {

    printf("%s\n",message);

#ifdef GTK_GUI
    if ( ind == GRAVE ) {
        display_popup(message);
    }
    else if ( ind == MINOR ) {
        status_message(message);
    }
#else
    print_help();
    exit(errno);
#endif

}

// Thread sleep during x ms
void sleep_millis(int ms) {
    #ifdef WIN32
        Sleep(ms);
    #elif defined (linux)
        if ( ms*1000 < 1000000 )
            usleep(ms*1000);
        else
            sleep( (int) round(ms/1000) ) ;
    #endif
}

/* From Wikipedia (K and R) */

void reverse(char s[]) {
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}




void p_itoa(int n, char s[]) {

    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

/* End Wiki */

int p_strcmpi(const char* input1, const char* input2) {
    return strcmp(str_upper(input1),str_upper(input2));
}

// Convert speed (5-100%) to delay (in milliseconds)
// INPUT IS ALWAYS BETWEEN 5 and 100, OR IS 200.
int speed2delay(int speed) {

    int delay = 0;

    if (speed != 200) // If speed is 200, full speed mode (delay 0)
        delay = (int) round( (105 - speed) / 5.0) ;

    return delay;
}

int p_random(int min, int max) {

    return  rand()%( max - min ) + min;
    //return ( 1 + (int) (max*1.0 * (rand() / (RAND_MAX + min*1.0))) );
}

/* Obfuscators functions */
char* random_string() {
    char* accepted = "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int nb_accepted = strlen(accepted);

    int r_str_size = p_random(1,64);
    char* r_str = (char*) malloc( sizeof(char) * r_str_size );
    int i;
    for (i=0; i < r_str_size-1; i++) {
        r_str[i] = accepted[ p_random(0,nb_accepted+1) ];
    }
    r_str[ r_str_size-1 ] = '\0';

    return r_str;
}

// Convert a String to A STRING
char* str_upper(const char* str) {
    int i;
    char* str_u = (char*) malloc(sizeof(char) * 256);
    strcpy(str_u,str);

    for( i = 0; str_u[i]; i++)
      str_u[i] = toupper(str_u[i]);

    return str_u;
}

void print_help() {
    printf("Usage :\n");
    printf("cLoic(.exe) method [ -m method ] [ -nt number_of_threads ] [ -s speed_in_percent ] [ -t target ] [ -p port ] [ -to seconds ] [ -w ] [ -r ] [ -gz ] [ -get ] [ -hs subsite ] [ -h irc.hive.server [ port #channel ] ] target");
    printf("\n");
}

int string2argTab(const char* strIn, char** charTab) {

        int nb = 0;
        char delims[] = " ";
        char *result = NULL;
        result = strtok( (char*) strIn, delims );

        while( result != NULL && nb < 15 ) {
                charTab[nb] = (char*) malloc(sizeof(**charTab) * 100);
                strcpy(charTab[nb],result);
                result = strtok( NULL, delims );

                nb++;
        }

        return nb;

}

p_BOOL isHostname(const char* input) {

    struct hostent* hostinfo = NULL;
    hostinfo = gethostbyname(input);

    if ( hostinfo == NULL )
        return FALSE;
    else
        return TRUE;

    return FALSE;
}

int s_connect(SOCKET s, char* target, int port) {

    struct hostent* hostinfo = NULL;
    SOCKADDR_IN sin = { 0 }; // Init

    hostinfo = gethostbyname(target); // Host informations

    if (hostinfo == NULL) {
        char buffer[256];
        sprintf(buffer,"Unknown host %s.", target);
        loic_error( buffer, MINOR );
        return 2;
    }

    sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr; // Adress
    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;

    if(connect(s,(SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        loic_error("Unable to connect.", MINOR);
        return 1;
    }

    return 0;

}

int s_send(SOCKET s, char* msg) {

    char buffer[1024];
    strcpy(buffer,msg);


    if( send(s, buffer, strlen(buffer), 0) < 0 ) {
        loic_error("Error while sending data.", MINOR);

    }

    return 1;
}

void s_recv(SOCKET s, char* buffer, int size) {
    int n = 0;


    if ( (n = recv(s, buffer, size - 1, 0)) < 0 ) {
        loic_error("Error while receiving data.", MINOR);
    }

    buffer[n] = '\0';



}

char* p_host_from_url(char* url) {

    char* url_cpy =(char*) malloc( ( sizeof(char) * strlen(url) ) + 1 );


    if ( strstr(url,"http://") != NULL )
        p_substring(url,url_cpy,7,strlen(url)-7);
    else if ( strstr(url,"https://") != NULL )
        p_substring(url,url_cpy,8,strlen(url)-8);
    else {
        printf("nf\n");
        strcpy(url_cpy,url);
    }

    printf("--> %s\n",url_cpy);

    char delims[] = "/";
    char* host = NULL;
    char* subsite = NULL;

    printf(".\n");

    host = strtok( (char*) url_cpy, delims );
    subsite = strtok( (char*) NULL, delims );

    if (host == NULL || subsite == NULL) {
        host = (char*) malloc( ( sizeof(char) * strlen(url_cpy) ) + 1 );
        strcpy(host,url_cpy);
    }

    return host;
}

int p_substring(const char* str_in, char* str_out, size_t begin, size_t len) {

  str_out = (char*) malloc( sizeof(char) * ( strlen(str_in) + 1) );

  if ( str_in == 0 || strlen(str_in) == 0 || strlen(str_in) < begin || strlen(str_in) < (begin+len) )
    return 0;

  int i;
  for (i=0; i < len; i++) {
    str_out[i] = str_in[i+begin];
  }
  str_out[len] = '\0';

  return 0;

}

SOCKET createSocket(int protocol) {


    SOCKET sock = INVALID_SOCKET;

    switch (protocol) {
    case UDP:
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        break;
    default: // TCP
        sock = socket(AF_INET, SOCK_STREAM, 0);
        break;

    }

    if(sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
        return 0;
    }
    else
        return sock;

}

    void init(void)
    {
    #ifdef WIN32
        WSADATA wsa;
        int err = WSAStartup(MAKEWORD(2, 2), &wsa);
        if(err < 0)
        {
            puts("WSAStartup failed !");
            exit(EXIT_FAILURE);
        }
    #endif
    }

    void end(void)
    {
    #ifdef WIN32
        WSACleanup();
    #endif


    }


