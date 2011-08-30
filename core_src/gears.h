#ifndef H_GEARS_H

    #define H_GEARS_H

    #include "const.h"
    #include "math.h"

    #include <time.h>


    typedef struct {
        u_char* pkt_data;
        int size;
    } simple_pkt_t;

    typedef struct {
        char** strings;
        int nb;
        int offset;
        u_char type;
    } string_list_t;


    #ifdef GTK_GUI
        #include "gtk_module.h"
    #endif



    int s_connect(SOCKET s, char* target, int port);
    int s_send(SOCKET s, char* msg);
    void s_recv(SOCKET s, char* buffer, int size);
    SOCKET createSocket(int protocol);

    int p_strcmpi(const char* input1, const char* input2);
    void sleep_millis(int ms);
    int speed2delay(int speed);
    char* str_upper(const char* str);
    int string2argTab(const char* strIn, char** charTab);
    void p_itoa(int n, char s[]);

    char* random_string();
    int p_random(int min, int max);


    char* p_host_from_url(char* url);
    int p_substring(const char* str_in, char* str_out, size_t begin, size_t len);

    void loic_error(const char* message, int from);
    void print_help();

    void init(void);
    void end(void);

    p_BOOL isHostname(const char* input);


    int select_from_list(int* list, int nb);
    string_list_t string_list_from_file( const char* path );

#endif
