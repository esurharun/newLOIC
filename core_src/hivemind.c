#include "hivemind.h"

int count = 0;

int done = 0;
int firing = 0;
int running = 0;

irc_session_t* s;


typedef struct
{
        char* channel;
        char* nick;

} irc_ctx_t;


void addlog (const char* fmt, ...)
{
/*
        FILE * fp;
        char buf[1024];
        va_list va_alist;

        va_start (va_alist, fmt);
#if defined (WIN32)
        _vsnprintf (buf, sizeof(buf), fmt, va_alist);
#else
        vsnprintf (buf, sizeof(buf), fmt, va_alist);
#endif
        va_end (va_alist);

        printf ("%s\n", buf);

        if ( (fp = fopen ("irctest.log", "ab")) != 0 )
        {
                fprintf (fp, "%s\n", buf);
                fclose (fp);
        }
 */
}



void dump_event (irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count) {

    /*
    char buf[512];
        int cnt;

        buf[0] = '\0';

        for ( cnt = 0; cnt < count; cnt++ )
        {
                if ( cnt )
                        strcat (buf, "|");

                strcat (buf, params[cnt]);
        }


        addlog ("Event \"%s\", origin: \"%s\", params: %d [%s]", event, origin ? origin : "NULL", cnt, buf);
     */

}


int lazor_command(const char* source_nick, const char* command, irc_session_t* session) {

        printf("Request by %s --- ,\n",source_nick);

        char* search = "!lazor ";
        if ( strncmp(command,search,strlen(search)) == 0 ) {

            char** irc_args = (char**) malloc(sizeof(*irc_args) * 512);
            int irc_argc = string2argTab(command,irc_args);

            if (irc_argc > 1)
                return config_from_args((const char**)irc_args,irc_argc,IRC_COMMAND);
            else
                return 1;

        }

        return 0;

}


void event_channel (irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count) {



    if (params[1][0] == '!')
            lazor_command(origin,params[1],session);
    /*

    char nickbuf[128];

        if ( count != 2 )
                return;


        printf ("'%s' said in channel %s: %s\n",
                origin ? origin : "someone",
                params[0], params[1] );




        if ( !origin )
                return;

        irc_target_get_nick (origin, nickbuf, sizeof(nickbuf));

        if ( !strcmp (params[1], "quit") )
                irc_cmd_quit (session, "of course, Master!");

        if ( !strcmp (params[1], "help") )
        {
                irc_cmd_msg (session, params[0], "quit, help, dcc chat, dcc send, ctcp");
        }

        if ( !strcmp (params[1], "ctcp") )
        {
                irc_cmd_ctcp_request (session, nickbuf, "PING 223");
                irc_cmd_ctcp_request (session, nickbuf, "FINGER");
                irc_cmd_ctcp_request (session, nickbuf, "VERSION");
                irc_cmd_ctcp_request (session, nickbuf, "TIME");
        }


        if ( !strcmp (params[1], "dcc chat") )
        {
                irc_dcc_t dccid;
                irc_dcc_chat (session, 0, nickbuf, dcc_recv_callback, &dccid);
                printf ("DCC chat ID: %d\n", dccid);
        }

        if ( !strcmp (params[1], "dcc send") )
        {
                irc_dcc_t dccid;
                irc_dcc_sendfile (session, 0, nickbuf, "irctest.c", dcc_file_recv_callback, &dccid);
                printf ("DCC send ID: %d\n", dccid);
        }


        if ( !strcmp (params[1], "topic") )
                irc_cmd_topic (session, params[0], 0);
        else if ( strstr (params[1], "topic ") == params[1] )
                irc_cmd_topic (session, params[0], params[1] + 6);

        if ( strstr (params[1], "mode ") == params[1] )
                irc_cmd_channel_mode (session, params[0], params[1] + 5);

        if ( strstr (params[1], "nick ") == params[1] )
                irc_cmd_nick (session, params[1] + 5);

        if ( strstr (params[1], "whois ") == params[1] )
                irc_cmd_whois (session, params[1] + 5);

                */
}

void event_numeric (irc_session_t* session, unsigned int event, const char* origin, const char** params, unsigned int count) {

    /*
        char buf[24];
        sprintf (buf, "%d", event);
        if (event == 332) {
                if (params[1][0] == '!')
                        lazor_command(params[1], session);
        }


        dump_event (session, buf, origin, params, count);
        */
}

void event_join (irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count) {

    dump_event (session, event, origin, params, count);


    irc_cmd_user_mode (session, "+i");
    /*
    irc_cmd_msg (session, params[0], "Hi all");


*/

}


void event_connect (irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count) {

        irc_ctx_t* ctx = (irc_ctx_t*) irc_get_ctx (session);
        dump_event (session, event, origin, params, count);

        irc_cmd_join (session, ctx->channel, 0);

        setStatus(HIVEMIND_READY);


}


void event_privmsg (irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count) {
    /*
        printf ("'%s' said me (%s): %s\n",
                origin ? origin : "someone",
                params[0], params[1] );

    */
}

int disconnect() {
    irc_cmd_quit (s, "Avec raison");

    return 0;
}

int start_hivemind(const char* server, int port, const char* chan) {

    setStatus(HIVEMIND_CONNECTING);


    if (port > 0 && server != NULL) {

        if (chan == NULL) {
            loic_error("Error : Bad channel.\n",CONSOLE);
        }

        irc_callbacks_t	callbacks;
        irc_ctx_t ctx;



        //printf ("Usage: %s <server> <nick> <channel>\n", argv[0]);

        memset (&callbacks, 0, sizeof(callbacks));

        callbacks.event_connect = event_connect;
        callbacks.event_join = event_join;
        callbacks.event_nick = dump_event;
        callbacks.event_quit = dump_event;
        callbacks.event_part = dump_event;
        callbacks.event_mode = dump_event;
        callbacks.event_topic = dump_event;
        callbacks.event_kick = dump_event;
        callbacks.event_channel = event_channel;
        callbacks.event_privmsg = event_privmsg;
        callbacks.event_numeric = event_numeric;


        callbacks.event_notice = dump_event;

        callbacks.event_invite = dump_event;

        callbacks.event_umode = dump_event;
        callbacks.event_ctcp_rep = dump_event;
        callbacks.event_ctcp_action = dump_event;
        callbacks.event_unknown = dump_event;

        /*
        callbacks.event_dcc_chat_req = irc_event_dcc_chat;
        callbacks.event_dcc_send_req = irc_event_dcc_send;
        */


        s = irc_create_session (&callbacks);

        if ( !s ) {
                printf ("Could not create session\n");
                return 1;
        }

        ctx.channel = (char*) malloc( sizeof(char) * (strlen(chan) + 1) );
        strcpy(ctx.channel,chan);

        ctx.nick = "Imnothere";
        irc_set_ctx (s, &ctx);

        printf("Connecting to %s with port %d ...\n",server,port);

        if ( irc_connect(s, server, port, 0, "Imnothere", 0, 0) ) {
                printf ("Could not connect : %s\n\n", irc_strerror (irc_errno(s)));
                return 1;
        }

        printf("Connected. Listening for !command messages ..\n");

        irc_run (s);

    }
    else
        loic_error("Bad Hivemind params.",CONSOLE);

    return 0;

}
