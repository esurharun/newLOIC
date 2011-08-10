#include "gtk_module.h"


//p_BOOL refresh_opts();

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_hivemind;


GtkWidget* mainWindow = NULL;


GtkWidget* f_Mode = NULL;


GtkWidget* f_Options = NULL;
GtkWidget* b_OptionsTCP = NULL;
GtkWidget* b_OptionsUDP = NULL;
GtkWidget* b_OptionsHTTP = NULL;
GtkWidget* b_OptionsSLOW_HTTP = NULL;
GtkWidget* b_OptionsSYN = NULL;
GtkWidget* b_OptionsSUDP = NULL;
GtkWidget* bb_Options = NULL;

GtkWidget* currentOptionPanel = NULL;


GtkWidget* b_allMethods = NULL;


GtkEntryBuffer* t_target;
GtkEntryBuffer* t_port;
GtkEntryBuffer* t_irc_server;
GtkEntryBuffer* t_irc_port;
GtkEntryBuffer* t_irc_chan;
GtkEntryBuffer* t_nb_threads;
GtkEntryBuffer* t_timeout;
GtkEntryBuffer* t_udp_message;
GtkEntryBuffer* t_http_subsite;


GtkEntryBuffer* t_source_ip;




GtkWidget* cb_randCharsTCP;
GtkWidget* cb_waitForReplyTCP;
GtkWidget* cb_randCharsUDP;
GtkWidget* cb_waitForReplyUDP;
GtkWidget* cb_randCharsHTTP;
GtkWidget* cb_waitForReplyHTTP;
GtkWidget* cb_randCharsSLOW_HTTP;
GtkWidget* cb_waitForReplySLOW_HTTP;
GtkWidget* cb_useGzipHTTP;
GtkWidget* cb_useGzipSLOW_HTTP;

GtkWidget* cb_randIPSUDP;
GtkWidget* cb_randIPSYN;



GtkWidget* statusBar = NULL;
GtkWidget* t_nbPacketsSent = NULL;

GtkWidget* s_speed = NULL;

GtkWidget* combo_method = NULL;
GtkWidget* combo_devices = NULL;




GtkWidget* spinner_detection = NULL;
GtkWidget* spinner_hivemind = NULL;


GtkWidget* bu_auto_detect = NULL;


void display_popup(const char* message) {


    char* buffer = (char*) malloc( sizeof(char) * 1024 );
    strcpy(buffer, message);
    GtkWidget* popup =  gtk_message_dialog_new( GTK_WINDOW(mainWindow), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, buffer );

    gtk_dialog_run (GTK_DIALOG (popup));
    gtk_widget_destroy (popup);

}

void status_message(const char message[256]) {

    char* buffer = (char*) malloc( sizeof(char) * ( strlen(message) + 1) );
    strcpy(buffer, message);

    gtk_statusbar_push ( GTK_STATUSBAR(statusBar), 0, buffer);

}

p_BOOL scale_speed(GtkWidget* scaleBar) {

    gint t = gtk_range_get_value( GTK_RANGE(scaleBar) );

    pthread_mutex_lock(&g_mutex);
        setSpeed(t);
    pthread_mutex_unlock(&g_mutex);

    return TRUE;
}

p_BOOL lock_target(GtkWidget* bu_lock, GtkWidget* bu_fire) {

    return TRUE;
}

p_BOOL randomIPchecked(GtkWidget* checkBox, GtkWidget* ipEntry) {

    gtk_widget_set_sensitive( ipEntry, !gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(checkBox) ) );
    return TRUE;
}


p_BOOL free_hivemind_from_gtk(GtkWidget* bu_hivemind) {

    gtk_widget_set_sensitive( f_Options, TRUE );
    gtk_button_set_label( GTK_BUTTON(bu_hivemind),"Connect");
    setStatus(READY);
    printf("Disconnected\n");
    gtk_spinner_stop( GTK_SPINNER(spinner_hivemind) );
    gtk_widget_show( bu_hivemind );

    return TRUE;
}

int hivemind_from_gtk(GtkWidget* bu_hivemind) {

    gtk_widget_set_sensitive( f_Options, FALSE );

    gtk_widget_set_sensitive( s_speed, TRUE );



    gtk_spinner_start( GTK_SPINNER(spinner_hivemind) );
    gtk_widget_show( GTK_WIDGET(spinner_hivemind) );

    int ret = start_hivemind( gtk_entry_buffer_get_text(t_irc_server),
                    atoi( gtk_entry_buffer_get_text(t_irc_port) ),
                    gtk_entry_buffer_get_text(t_irc_chan)
                    );

    free_hivemind_from_gtk(bu_hivemind);

    return ret;
}




p_BOOL push_hivemind_button(GtkWidget* bu_hivemind) {


    if ( getStatus() == READY ) {


        setStatus(HIVEMIND_CONNECTING);
        gtk_button_set_label( GTK_BUTTON(bu_hivemind),"Disconnect");


        if ( strlen( gtk_entry_buffer_get_text(t_irc_server) ) != 0
             && strlen( gtk_entry_buffer_get_text(t_irc_port) ) != 0
             && strlen( gtk_entry_buffer_get_text(t_irc_chan) ) != 0  ) {

            pthread_create(&thread_hivemind, NULL, (void*)hivemind_from_gtk, bu_hivemind);
        }
        else {
            loic_error("Bad Hivemind params.", GRAVE);
            free_hivemind_from_gtk(bu_hivemind);

        }


    }
    else if ( getStatus() != HIVEMIND_CONNECTING ){
        if ( disconnect() == 0) {
            free_hivemind_from_gtk(bu_hivemind);
        }
    }

    return TRUE;
}

p_BOOL fire_from_gtk(GtkWidget* bu_fire) {


    gtk_button_set_label( GTK_BUTTON(bu_fire),"STOP FIRE");
    gtk_widget_set_sensitive(f_Mode, FALSE);
    gtk_widget_set_sensitive(f_Options, FALSE);
    gtk_widget_set_sensitive(bu_fire, TRUE);

    int ret = -1;

    #ifdef SUPER_LOIC
        if ( !isNetworkReady() ) {
            charge_superCanon();
        }
        ret = start_superCanon();
    #else
        ret = start_canon();
    #endif

    gtk_widget_set_sensitive(bu_fire, FALSE);
    gtk_button_set_label(GTK_BUTTON(bu_fire),"FIRE !");
    gtk_widget_set_sensitive(f_Mode, TRUE);
    gtk_widget_set_sensitive(f_Options, TRUE);
    gtk_widget_set_sensitive(bu_fire, TRUE);

    return ret;
}


#ifdef SUPER_LOIC

    GtkWidget* create_interface_list() {

        GtkWidget* combo_dev = gtk_combo_box_text_new();
        char* buffer = (char*) malloc( sizeof(char) * 512 );

        int i;
        for (i=0; i<get_custom_device_count(); i++) {
            strcpy(buffer, get_custom_string_from_device( i ));
            gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo_dev), buffer );
        }

        return combo_dev;
    }


    p_BOOL thread_autodetect( GtkWidget* b_detect )  {

        gtk_container_remove( GTK_CONTAINER(b_detect), GTK_WIDGET(bu_auto_detect) );
        gtk_container_add( GTK_CONTAINER(b_detect), GTK_WIDGET(spinner_detection) );
        gtk_spinner_start( GTK_SPINNER(spinner_detection) );
        gtk_widget_show( GTK_WIDGET(spinner_detection) );

        int my_index = -1;
        my_index = auto_detect_custom_device();

        if ( my_index != -1 ) {

            gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_devices), my_index );

            gtk_spinner_stop( GTK_SPINNER(spinner_detection) );
            gtk_container_remove( GTK_CONTAINER(b_detect), GTK_WIDGET(spinner_detection) );
            gtk_container_add( GTK_CONTAINER(b_detect), GTK_WIDGET(bu_auto_detect) );


            return TRUE;

        }

        return FALSE;

    }



    p_BOOL select_device( GtkWidget* combo_devices ) {


        int index = gtk_combo_box_get_active ( GTK_COMBO_BOX(combo_devices) );

        if ( index >= 0 ) {

            char* buffer = (char*) malloc( sizeof(char) * 32 );
            strcpy(buffer,get_ip_from_custom_device( index ));

            gtk_entry_buffer_set_text( t_source_ip, buffer, strlen(buffer) );

            printf("Changed to %s\n",buffer);
        }
        else
            return FALSE;


        return TRUE;

    }
#endif

p_BOOL push_auto_button(GtkWidget* bu_autodetect, GtkWidget* b_detect) {


#ifdef SUPER_LOIC


    pthread_t thread_detect;
    pthread_create(&thread_detect,NULL,(void*)thread_autodetect,b_detect);

#endif




    return TRUE;

}

p_BOOL refresh_opts() {

    char buffer[256];

    if ( getProtocol() == TCP ) {
        gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_method), 1 );
    }
    else if ( getProtocol() == UDP ) {
        gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_method), 0 );
    }
    else if ( getProtocol() == SLOW_HTTP ) {
        gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_method), 3 );
    }
    else if ( getProtocol() == HTTP ) {
        gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_method), 2 );
    }
    else if ( getProtocol() == SUDP ) {
        gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_method), 4 );
    }
    else if ( getProtocol() == SYN ) {
        gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_method), 5 );
    }


    strcpy(buffer,getTarget());
    gtk_entry_buffer_set_text( t_target, buffer, strlen(buffer) );


    p_itoa(getPort(),buffer);
    gtk_entry_buffer_set_text( t_port, buffer, strlen(buffer) );

    p_itoa(getNbThreads(),buffer);
    gtk_entry_buffer_set_text( t_nb_threads, buffer, strlen(buffer) );

    p_itoa(getTimeout(),buffer);
    gtk_entry_buffer_set_text( t_timeout, buffer, strlen(buffer) );

    strcpy(buffer,getUdpMessage());
    gtk_entry_buffer_set_text( t_udp_message, buffer, strlen(buffer) );

    strcpy(buffer,getSubsite());
    gtk_entry_buffer_set_text( t_http_subsite, buffer, strlen(buffer) );


    gtk_range_set_value( GTK_RANGE(s_speed), getSpeed() );



    return TRUE;
}

p_BOOL refresh_infos() {


    /* BUUGG
    if ( getStatus() == FIRING ) {

        int nb_packets = 0;

        int i;
        for ( i=0; i<getNbThreads(); i++ ) {
            nb_packets += getNbPacketsSent(i);
        }

        char buffer[64];
        p_itoa(nb_packets, buffer);
        strcat(buffer," packets sent !");
        gtk_statusbar_push ( GTK_STATUSBAR(statusBar), 0, buffer);


        printf("maj (%d)\n",nb_packets);
    }

    */
    return TRUE;
}

p_BOOL push_fire_button(GtkWidget* bu_fire) {

    p_BOOL ret = FALSE;
    gtk_widget_set_sensitive(bu_fire, FALSE);


    if ( getStatus() == FIRING ) {
        setStatus( STOPPING );

        ret = FALSE;
    }
    else if ( setTarget( gtk_entry_buffer_get_text(t_target) ) != 0)
        ret = FALSE;
    else if ( setPort( atoi( gtk_entry_buffer_get_text(t_port) )) != 0)
        ret = FALSE;
    else if ( setUdpMessage ( gtk_entry_buffer_get_text(t_udp_message) ) != 0)
        ret = FALSE;
    else if ( setSubsite( gtk_entry_buffer_get_text(t_http_subsite) ) != 0)
        ret = FALSE;
    else if ( setTimeout( atoi( gtk_entry_buffer_get_text(t_timeout) )) != 0)
        ret = FALSE;
    else if ( setNbThreads( atoi( gtk_entry_buffer_get_text(t_nb_threads) )) != 0)
        ret = FALSE;
    else {

        printf("Charging...\n");

        switch ( getProtocol() ) {
            case TCP :
                setRandomCharsEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_randCharsTCP) ) );
                setWaitEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_waitForReplyTCP) ) );
                break;
            case UDP :
                setRandomCharsEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_randCharsUDP) ) );
                setWaitEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_waitForReplyUDP) ) );
                break;
            case HTTP :
                setRandomCharsEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_randCharsHTTP) ) );
                setWaitEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_waitForReplyHTTP) ) );
                setGzipEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_useGzipHTTP) ) );
                break;
            case SLOW_HTTP :
                setRandomCharsEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_randCharsSLOW_HTTP) ) );
                setWaitEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_waitForReplySLOW_HTTP) ) );
                setGzipEnabled( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(cb_useGzipSLOW_HTTP) ) );
                break;
#ifdef SUPER_LOIC
            case SUDP :
                setSpoofedIp( gtk_entry_buffer_get_text(t_source_ip) );
                break;
            case SYN :
                setSpoofedIp( gtk_entry_buffer_get_text(t_source_ip) );
                break;
#endif
            default :
                break;
        }

        if ( strlen( getTarget() ) != 0 ) {

            printf("Canon started\n");

            ret = TRUE;

            pthread_t thread_canon;
            pthread_create( &thread_canon, NULL, (void*)fire_from_gtk, bu_fire );
        }
        else {
            loic_error( "You haven't defined the target ..", GRAVE );
            ret = FALSE;
        }

    }


    if ( ret == FALSE ) {
        printf("Canceled\n");
        gtk_widget_set_sensitive(bu_fire, TRUE);
    }

    return ret;
}


p_BOOL setOptionPanel(int method) {

    GtkWidget* l_httpSub;
    GtkWidget* e_httpSub;

    GtkWidget* l_cData;
    GtkWidget* e_cData;

    GtkWidget* l_ipSource;
    GtkWidget* e_ipSource;




    if ( method == UDP ) {

        GtkWidget* l_description = gtk_label_new("UDP : Send UDP packets.");


            l_cData = gtk_label_new(" TCP / UDP message :");
            e_cData = gtk_entry_new_with_buffer(t_udp_message);
            gtk_entry_set_width_chars( GTK_ENTRY(e_cData), 40);
            cb_randCharsUDP = gtk_check_button_new_with_label("Append random chars ?");
        GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_cData, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_cData, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), cb_randCharsUDP, FALSE, TRUE, 5 );

        cb_waitForReplyUDP = gtk_check_button_new_with_label("Wait for Reply ?");
        GtkWidget* b_checkOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_checkOpts), cb_waitForReplyUDP, FALSE, TRUE, 5 );


        b_OptionsUDP = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsUDP), l_description, FALSE, TRUE, 5);
        gtk_box_pack_start ( GTK_BOX(b_OptionsUDP), b_dataOpts, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsUDP), b_checkOpts, FALSE, TRUE, 0 );



        g_object_ref(b_OptionsUDP);




    }
    else if  (method == TCP ) {

        GtkWidget* l_description = gtk_label_new("TCP : Open TCP sockets (both with 3-ways handhsake)");



            l_cData = gtk_label_new(" TCP / UDP message :");
            e_cData = gtk_entry_new_with_buffer(t_udp_message);
            gtk_entry_set_width_chars( GTK_ENTRY(e_cData), 40);
            cb_randCharsTCP = gtk_check_button_new_with_label("Append random chars ?");
        GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_cData, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_cData, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), cb_randCharsTCP, FALSE, TRUE, 5 );

            cb_waitForReplyTCP = gtk_check_button_new_with_label("Wait for Reply ?");
            GtkWidget* b_checkOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_checkOpts), cb_waitForReplyTCP, FALSE, TRUE, 5 );

        b_OptionsTCP = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsTCP), l_description, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsTCP), b_dataOpts, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsTCP), b_checkOpts, FALSE, TRUE, 0 );


        g_object_ref(b_OptionsTCP);


    }
    else if  (method == HTTP ) {

        GtkWidget* l_description = gtk_label_new("HTTP : Use TCP packets with HTTP requests (like yout Web Browser)");




            l_httpSub = gtk_label_new(" HTTP URL Subsite :");
            e_httpSub = gtk_entry_new_with_buffer(t_http_subsite);
            gtk_entry_set_width_chars( GTK_ENTRY(e_httpSub),15);
            cb_randCharsHTTP = gtk_check_button_new_with_label("Append random chars ?");
        GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_httpSub, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_httpSub, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), cb_randCharsHTTP, FALSE, TRUE, 5 );

            cb_useGzipHTTP = gtk_check_button_new_with_label("Use Gzip ?");
            cb_waitForReplyHTTP = gtk_check_button_new_with_label("Wait for Reply ?");
        GtkWidget* b_checkOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_checkOpts), cb_waitForReplyHTTP, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_checkOpts), cb_useGzipHTTP, FALSE, TRUE, 5 );


        b_OptionsHTTP = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsHTTP), l_description, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsHTTP), b_dataOpts, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsHTTP), b_checkOpts, FALSE, TRUE, 0 );


        g_object_ref(b_OptionsHTTP);


    }
    else if  (method == SLOW_HTTP ) {

        GtkWidget* l_description = gtk_label_new("Slowloic Mode : Like Slowloris perl program. Make half-connections to HTTP server usefull in \"simple\" DOS, not DDOS");




            l_httpSub = gtk_label_new(" HTTP URL Subsite :");
            e_httpSub = gtk_entry_new_with_buffer(t_http_subsite);
            gtk_entry_set_width_chars( GTK_ENTRY(e_httpSub),15);
            cb_randCharsSLOW_HTTP = gtk_check_button_new_with_label("Append random chars ?");
        GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_httpSub, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_httpSub, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), cb_randCharsSLOW_HTTP, FALSE, TRUE, 5 );

            cb_useGzipSLOW_HTTP = gtk_check_button_new_with_label("Use Gzip ?");
            cb_waitForReplyHTTP = gtk_check_button_new_with_label("Wait for Reply ?");
        GtkWidget* b_checkOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_checkOpts), cb_waitForReplySLOW_HTTP, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_checkOpts), cb_useGzipSLOW_HTTP, FALSE, TRUE, 5 );


        b_OptionsSLOW_HTTP = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSLOW_HTTP), l_description, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSLOW_HTTP), b_dataOpts, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSLOW_HTTP), b_checkOpts, FALSE, TRUE, 0 );


        g_object_ref(b_OptionsSLOW_HTTP);


    }
    else if  (method == SUDP ) {

            GtkWidget* l_description = gtk_label_new(" Super UDP Mode : Avalaible in superLOIC version. (packet-forging, ip spoofing, etc)");


            GtkWidget* l_choose_device = gtk_label_new(" Device");



         GtkWidget* b_superOpts = gtk_hbox_new( FALSE, 0 );
         gtk_box_pack_start ( GTK_BOX(b_superOpts), l_choose_device, FALSE, TRUE, 5 );
         gtk_box_pack_start ( GTK_BOX(b_superOpts), combo_devices, FALSE, TRUE, 5 );


         GtkWidget* b_detect = gtk_vbox_new( FALSE, 0 );


         gtk_box_pack_start ( GTK_BOX(b_detect), bu_auto_detect, FALSE, TRUE, 5 );
         gtk_box_pack_start ( GTK_BOX(b_superOpts), b_detect, FALSE, TRUE, 5 );



            l_ipSource = gtk_label_new(" Source IP address :");
            e_ipSource = gtk_entry_new_with_buffer( t_source_ip );
            gtk_entry_set_width_chars( GTK_ENTRY(e_ipSource),15);
            cb_randIPSUDP = gtk_check_button_new_with_label("Use random source IP address ?");


        GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_ipSource, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_ipSource, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), cb_randIPSUDP, FALSE, TRUE, 5 );


        g_signal_connect( G_OBJECT(bu_auto_detect), "clicked", G_CALLBACK(push_auto_button), b_detect );




        b_OptionsSUDP = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSUDP), l_description, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSUDP), b_superOpts, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSUDP), b_dataOpts, FALSE, TRUE, 5 );




        g_signal_connect( G_OBJECT(cb_randIPSUDP), "clicked", G_CALLBACK(randomIPchecked), e_ipSource );



        g_object_ref(b_OptionsSUDP);

    }
    else if  (method == SYN ) {

        GtkWidget* l_description = gtk_label_new(" SYN Flood Mode : Avalaible in superLOIC version. Send SYN requests to a server.");

        b_OptionsSYN = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSYN), l_description, FALSE, TRUE, 5 );

        g_object_ref(b_OptionsSYN);
    }



    return TRUE;
}

p_BOOL paintOptionsPanel(GtkWidget* sender, gpointer data) {

    printf("Click :\n");


    int method = -1;

    if (sender == NULL)
        method = (int) data;
    else {
        gchar* txt = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT(sender) );
        if ( strcmp(txt,"UDP") == 0 )
            method = UDP;
        else if ( strcmp(txt,"TCP") == 0 )
            method = TCP;
        else if ( strcmp(txt,"HTTP") == 0 )
            method = HTTP;
        else if ( strcmp(txt,"SLOWLOIC") == 0 )
            method = SLOW_HTTP;
        else if ( strcmp(txt,"TCP SYN FLOOD (super Mode)") == 0 )
            method = SYN;
        else if ( strcmp(txt,"UDP (super Mode)") == 0 )
            method = SUDP;
    }

    if (currentOptionPanel != NULL)
        gtk_container_remove( GTK_CONTAINER(bb_Options), GTK_WIDGET(currentOptionPanel) );


    if ( method == TCP ) {

        printf("TCP !\n");
        currentOptionPanel = b_OptionsTCP;


    }
    else if ( method == UDP ) {

        printf("UDP !\n");
        currentOptionPanel = b_OptionsUDP;

    }
    else if ( method == HTTP ) {

        printf("HTTP !\n");
        currentOptionPanel = b_OptionsHTTP;

    }
    else if ( method == SLOW_HTTP ) {

        printf("SLOW_HTTP !\n");
        currentOptionPanel = b_OptionsSLOW_HTTP;

    }
    else if ( method == SYN ) {

        printf("SYN !\n");
        currentOptionPanel = b_OptionsSYN;

    }
    else if ( method == SUDP ) {

        printf("SUDP !\n");
        currentOptionPanel = b_OptionsSUDP;

    }
    else
        return FALSE;



    if ( currentOptionPanel != NULL )
        gtk_box_pack_start( GTK_BOX(bb_Options), currentOptionPanel, FALSE, FALSE, 0 );


    if ( method != -1 ) {
        setProtocoli(method);
    }

    gtk_widget_show_all(f_Options);


    return TRUE;
}

p_BOOL build_window() {

    /* Vars */

        GtkWidget* b_Main = NULL;


            GtkWidget* b_Left = NULL;
            GtkWidget* b_Right = NULL;

                GtkWidget* b_Mode = NULL;
                GtkWidget* bb_Mode = NULL;

                    GtkWidget* r_manual = NULL;
                    GtkWidget* r_hivemind = NULL;


                    GtkWidget* b_Server = NULL;
                        GtkWidget* l_server = NULL;
                        GtkWidget* e_server = NULL;

                    GtkWidget* b_Port = NULL;
                        GtkWidget* l_port = NULL;
                        GtkWidget* e_port = NULL;

                    GtkWidget* b_Chan = NULL;
                        GtkWidget* l_chan = NULL;
                        GtkWidget* e_chan = NULL;

                    GtkWidget* bu_Hivemind = NULL;




                    GtkWidget* f_Action = NULL;
                    GtkWidget* b_Action = NULL;
                    GtkWidget* bb_Action = NULL;


                        GtkWidget* b_QuickOpts;


                            GtkWidget* b_Target = NULL;
                                GtkWidget* l_url = NULL;
                                GtkWidget* e_url = NULL;
                                GtkWidget* bu_url = NULL;

                            GtkWidget* b_Method = NULL;
                                GtkWidget* l_method = NULL;

                        GtkWidget* bu_fire = NULL;





                        /*
                    GtkWidget* f_Options = NULL;
                    GtkWidget* b_Options = NULL;
                    */


                GtkWidget* f_Status = NULL;
                GtkWidget* b_Status = NULL;
/*
GtkWidget* b_Status = NULL;
                    GtkWidget* e_sent = NULL;
                    */



#ifdef SUPER_LOIC
    generate_devices_list();
#endif

    t_target = gtk_entry_buffer_new("",0);
    t_port = gtk_entry_buffer_new("80",2);
    t_irc_server = gtk_entry_buffer_new("",0);
    t_irc_port = gtk_entry_buffer_new("",0);
    t_irc_chan = gtk_entry_buffer_new("",0);
    t_nb_threads = gtk_entry_buffer_new("10",2);
    t_timeout = gtk_entry_buffer_new("3600",4);
    t_udp_message = gtk_entry_buffer_new("",0);
    t_http_subsite = gtk_entry_buffer_new("/",1);

    t_source_ip = gtk_entry_buffer_new("0.0.0.0",7);


    /* Main Window */
    mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(mainWindow), "delete-event", G_CALLBACK(gtk_main_quit), NULL);

    gtk_window_set_title( GTK_WINDOW(mainWindow), "Test" );
    gtk_window_set_default_size( GTK_WINDOW(mainWindow), 400, 480 );
    gtk_window_set_resizable ( GTK_WINDOW(mainWindow), TRUE );
    gtk_window_set_icon_from_file( GTK_WINDOW(mainWindow), "icon.png", NULL );

    /* End Main Window */

    spinner_detection = gtk_spinner_new();
    spinner_hivemind = gtk_spinner_new();


                r_manual = gtk_radio_button_new_with_label(NULL, "Manual Mode");
                r_hivemind = gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON(r_manual), "HiveMind Mode !" );


                b_Server = gtk_vbox_new( FALSE, 0 );
                l_server = gtk_label_new("Server");
                e_server = gtk_entry_new_with_buffer(t_irc_server);
                gtk_entry_set_width_chars( GTK_ENTRY(e_server),20);

                gtk_box_pack_start ( GTK_BOX(b_Server), l_server, TRUE, FALSE, 0 );
                gtk_box_pack_start ( GTK_BOX(b_Server), e_server, TRUE, FALSE, 0 );



                b_Port = gtk_vbox_new( FALSE, 0 );
                l_port = gtk_label_new("Port");
                e_port = gtk_entry_new_with_buffer(t_irc_port);
                gtk_entry_set_width_chars( GTK_ENTRY(e_port),5);


                gtk_box_pack_start ( GTK_BOX(b_Port), l_port, TRUE, FALSE, 0 );
                gtk_box_pack_start ( GTK_BOX(b_Port), e_port, TRUE, FALSE, 0 );


                b_Chan = gtk_vbox_new( FALSE, 0 );
                l_chan = gtk_label_new("Channel");
                e_chan = gtk_entry_new_with_buffer(t_irc_chan);
                gtk_entry_set_width_chars( GTK_ENTRY(e_chan),15);


                gtk_box_pack_start ( GTK_BOX(b_Chan), l_chan, TRUE, FALSE, 0 );
                gtk_box_pack_start ( GTK_BOX(b_Chan), e_chan, TRUE, FALSE, 0 );


                bu_Hivemind = gtk_button_new_with_label("Go");

                g_signal_connect( G_OBJECT( bu_Hivemind ), "clicked", G_CALLBACK(push_hivemind_button), NULL);




            b_Mode = gtk_hbox_new( FALSE, 5 );
            bb_Mode = gtk_hbox_new( FALSE, 5 );

            gtk_box_pack_start ( GTK_BOX(b_Mode), r_manual, FALSE, FALSE, 10 );
            gtk_box_pack_start ( GTK_BOX(b_Mode), r_hivemind, FALSE, FALSE, 10 );
            gtk_box_pack_start ( GTK_BOX(b_Mode), b_Server, FALSE, FALSE, 10 );
            gtk_box_pack_start ( GTK_BOX(b_Mode), b_Port, FALSE, FALSE, 10 );
            gtk_box_pack_start ( GTK_BOX(b_Mode), b_Chan, FALSE, FALSE, 10 );
            gtk_box_pack_start ( GTK_BOX(b_Mode), bu_Hivemind, FALSE, FALSE, 10 );
            gtk_box_pack_start ( GTK_BOX(b_Mode), spinner_hivemind, FALSE, FALSE, 10 );


            f_Mode = gtk_frame_new("Mode");

            gtk_box_pack_start ( GTK_BOX(bb_Mode), b_Mode, FALSE, FALSE, 10 );

            gtk_container_add ( GTK_CONTAINER(f_Mode), bb_Mode );
            gtk_frame_set_shadow_type( GTK_FRAME(f_Mode), GTK_SHADOW_OUT);






                    l_url = gtk_label_new("Target IP/URL ");
                    e_url = gtk_entry_new_with_buffer(t_target);
                    gtk_entry_set_width_chars( GTK_ENTRY(e_url),50);

                    bu_url = gtk_button_new_with_label("Lock On");


                b_Target = gtk_hbox_new( FALSE, 0 );
                gtk_container_add ( GTK_CONTAINER(b_Target), l_url );
                gtk_container_add ( GTK_CONTAINER(b_Target), e_url );
                gtk_container_add ( GTK_CONTAINER(b_Target), bu_url );



                    l_method = gtk_label_new("Select a Lazor");

                    combo_method = gtk_combo_box_text_new();

                        gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo_method), "UDP");
                        gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo_method), "TCP");
                        gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT (combo_method), "HTTP");
                        gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo_method), "SLOWLOIC");
                        gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT (combo_method), "TCP SYN FLOOD (super Mode)");
                        gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT (combo_method), "UDP (super Mode)");




                    g_signal_connect( G_OBJECT( combo_method ), "changed", G_CALLBACK(paintOptionsPanel), NULL);
                    g_signal_connect( G_OBJECT( bu_url ), "clicked", G_CALLBACK(gtk_widget_set_sensitive), FALSE);



                b_Method = gtk_hbox_new( TRUE, 0 );
                gtk_box_pack_start ( GTK_BOX(b_Method), l_method , FALSE, FALSE, 0 );
                gtk_box_pack_start ( GTK_BOX(b_Method), combo_method, FALSE, FALSE, 10 );

                bu_fire = gtk_button_new_with_label("FIRE");
                g_signal_connect( G_OBJECT(bu_fire), "clicked", G_CALLBACK(push_fire_button), NULL );
                gtk_widget_set_usize(bu_fire,150,50);


                b_QuickOpts = gtk_vbox_new( FALSE, 0 );
                gtk_box_pack_start ( GTK_BOX(b_QuickOpts), b_Target, FALSE, FALSE, 10 );
                gtk_box_pack_start ( GTK_BOX(b_QuickOpts), b_Method, FALSE, FALSE, 0 );


                b_Action = gtk_hbox_new( FALSE, 0 );
                bb_Action = gtk_hbox_new( FALSE, 0 );

                gtk_box_pack_start ( GTK_BOX(b_Action), b_QuickOpts, FALSE, FALSE, 10 );
                gtk_box_pack_start ( GTK_BOX(b_Action), bu_fire, FALSE, TRUE, 30 );


                gtk_box_pack_start ( GTK_BOX(bb_Action), b_Action, FALSE, FALSE, 30 );

                f_Action = gtk_frame_new("Configuration");
                gtk_container_add ( GTK_CONTAINER(f_Action), bb_Action );



                f_Options = gtk_frame_new("Options");
                gtk_frame_set_shadow_type( GTK_FRAME(f_Options), GTK_SHADOW_IN);

                bb_Options = gtk_vbox_new( FALSE, 0 );
                gtk_widget_set_usize(bb_Options,-1,300);
                gtk_container_add ( GTK_CONTAINER(f_Options), bb_Options );



            /* OPTIONS */

                b_allMethods = gtk_hbox_new( FALSE, 0 );

                GtkWidget* l_tport = NULL;
                GtkWidget* e_tport = NULL;

                GtkWidget* l_timeout = NULL;
                GtkWidget* e_timeout = NULL;

                GtkWidget* l_threads = NULL;
                GtkWidget* e_threads = NULL;

                GtkWidget* l_speed = NULL;



                l_tport = gtk_label_new(" Port");
                e_tport = gtk_entry_new_with_buffer(t_port);
                gtk_entry_set_width_chars( GTK_ENTRY(e_tport),5);

                l_timeout = gtk_label_new(" Timeout");
                e_timeout = gtk_entry_new_with_buffer(t_timeout);
                gtk_entry_set_width_chars( GTK_ENTRY(e_timeout),5);


                l_threads = gtk_label_new(" Threads");
                e_threads = gtk_entry_new_with_buffer(t_nb_threads);
                gtk_entry_set_width_chars( GTK_ENTRY(e_threads),5);


                l_speed = gtk_label_new(" Speed");
                s_speed = gtk_hscale_new_with_range( 5, 100, 1 );
                gtk_range_set_value( GTK_RANGE(s_speed), getSpeed() );




                gtk_box_pack_start ( GTK_BOX(b_allMethods), l_tport, FALSE, TRUE, 5 );
                gtk_box_pack_start ( GTK_BOX(b_allMethods), e_tport, FALSE, TRUE, 5 );

                gtk_box_pack_start ( GTK_BOX(b_allMethods), l_threads, FALSE, TRUE, 5 );
                gtk_box_pack_start ( GTK_BOX(b_allMethods), e_threads, FALSE, TRUE, 5 );

                gtk_box_pack_start ( GTK_BOX(b_allMethods), l_timeout, FALSE, TRUE, 5 );
                gtk_box_pack_start ( GTK_BOX(b_allMethods), e_timeout, FALSE, TRUE, 5 );


                gtk_box_pack_start ( GTK_BOX(b_allMethods), l_speed, FALSE, TRUE, 5 );
                gtk_box_pack_start ( GTK_BOX(b_allMethods), s_speed, TRUE, TRUE, 5 );




                gtk_box_pack_start ( GTK_BOX(bb_Options), b_allMethods, FALSE, FALSE, 10 );


                gtk_widget_set_usize(f_Options,200,300);

            /* END OPTIONS */

                currentOptionPanel = NULL;


            statusBar = gtk_statusbar_new();

            b_Status = gtk_hbox_new( FALSE, 0 );
            gtk_box_pack_start ( GTK_BOX(b_Status), statusBar, FALSE, FALSE, 10 );

            gtk_widget_set_usize(b_Status,-1,30);
            gtk_widget_set_usize(statusBar,480,30);

            gtk_statusbar_push ( GTK_STATUSBAR(statusBar), 0, "Ready.");


            f_Status = gtk_frame_new("Status");

            gtk_container_add ( GTK_CONTAINER(f_Status), b_Status );


        b_Right = gtk_vbox_new( FALSE, 0 );

        gtk_box_pack_start ( GTK_BOX(b_Right), f_Mode, FALSE, FALSE, 10 );
        gtk_box_pack_start ( GTK_BOX(b_Right), f_Action, FALSE, FALSE, 10 );
        gtk_box_pack_start ( GTK_BOX(b_Right), f_Options, FALSE, FALSE, 10 );
        gtk_box_pack_start ( GTK_BOX(b_Right), f_Status, FALSE, FALSE, 10 );


        GtkWidget* imgL = gtk_image_new_from_file ("img.png");

        b_Left = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start ( GTK_BOX(b_Left), imgL, FALSE, FALSE, 0 );





    b_Main = gtk_hbox_new( FALSE, 0 );



    gtk_box_pack_start ( GTK_BOX(b_Main), b_Left, FALSE, FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX(b_Main), b_Right, FALSE, FALSE, 10 );


    gtk_container_add ( GTK_CONTAINER(mainWindow), b_Main );





#ifdef SUPER_LOIC

    combo_devices = create_interface_list();
    g_signal_connect( G_OBJECT( combo_devices ), "changed", G_CALLBACK(select_device), NULL);
    g_object_ref(combo_devices);
#endif


    bu_auto_detect = gtk_button_new_with_label("Auto-Detect");

    g_object_ref(bu_auto_detect);
    g_object_ref(spinner_detection);
    g_object_ref(spinner_hivemind);




    setOptionPanel(TCP);
    setOptionPanel(UDP);
    setOptionPanel(HTTP);
    setOptionPanel(SLOW_HTTP);
    setOptionPanel(SUDP);
    setOptionPanel(SYN);


    setStatus(READY);
    g_signal_connect( G_OBJECT(s_speed), "value-changed", G_CALLBACK(scale_speed), NULL );


    /* Go */
    gtk_widget_show_all(mainWindow);

    gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_method), 0 );



    return TRUE;
}
