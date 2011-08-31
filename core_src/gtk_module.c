#include "gtk_module.h"


//p_BOOL refresh_opts();

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_hivemind;


GtkWidget* b_superOpts = NULL;
GtkWidget* b_dataOpts = NULL;


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
GtkWidget* b_QuickOpts;


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

GtkEntryBuffer* t_file_udpMessage;
GtkEntryBuffer* t_file_httpSubsite;






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
GtkWidget* combo_devices_SUDP = NULL;
GtkWidget* combo_devices_SYN = NULL;




GtkWidget* bu_fire = NULL;



GtkWidget* spinner_detection_SUDP = NULL;
GtkWidget* spinner_detection_SYN = NULL;

GtkWidget* spinner_hivemind = NULL;


GtkWidget* bu_auto_detect_SUDP = NULL;
GtkWidget* bu_auto_detect_SYN = NULL;



p_BOOL checkLoicCompatibility() {
    if ( getProtocol() == SUDP || getProtocol() == SYN ) {
        #ifdef SUPER_LOIC
            return TRUE;
        #else
            return FALSE;
        #endif
    }
    else
        return TRUE;
}

static gboolean display_popup_l(const char* message) {


    char* buffer = (char*) malloc( sizeof(char) * 1024 );
    strcpy(buffer, message);
    GtkWidget* popup =  gtk_message_dialog_new( GTK_WINDOW(mainWindow), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, buffer );

    gtk_dialog_run (GTK_DIALOG (popup));
    gtk_widget_destroy (popup);

    return FALSE;

}

static gboolean fileChoose( GtkWidget* file_chooser, GtkWidget* other_entry_buffer ) {

    gchar* uri = gtk_file_chooser_get_uri( GTK_FILE_CHOOSER(file_chooser) );
    gtk_entry_buffer_set_text( GTK_ENTRY_BUFFER(other_entry_buffer), uri, strlen(uri) );

    return FALSE;
}

static gboolean fileChange( GtkWidget* entry_buffer, GtkWidget* other_entry ) {

    if ( gtk_entry_buffer_get_length( GTK_ENTRY_BUFFER(entry_buffer) ) < 1 ) {

        gtk_widget_set_sensitive( other_entry, TRUE );

        return TRUE;

    }
    else {

        gtk_widget_set_sensitive( other_entry, FALSE );

        return TRUE;
    }

    return FALSE;
}

static gboolean iText( GtkWidget* entry_buffer, guint position, gchar* chars, guint n_chars, GtkWidget* custom) {

    return (fileChange(entry_buffer, custom));
}

static gboolean dText( GtkWidget* entry_buffer, guint position, guint n_chars, GtkWidget* custom) {

    return (fileChange(entry_buffer, custom));
}



void display_popup(const char* message) {

    g_idle_add( (GSourceFunc)display_popup_l, (gpointer) message);


}

static gboolean status_message_l(const char message[256]) {

    char* buffer = (char*) malloc( sizeof(char) * ( strlen(message) + 1) );
    strcpy(buffer, message);

    gtk_statusbar_push ( GTK_STATUSBAR(statusBar), 0, buffer);


    return FALSE;


}

void status_message(const char message[256]) {

    g_idle_add( (GSourceFunc)status_message_l, (gpointer) message);

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

static gboolean lock_panels_fire_l(GtkWidget* bu_fire) {

    gtk_button_set_label( GTK_BUTTON(bu_fire),"STOP FIRE");
    gtk_widget_set_sensitive(f_Mode, FALSE);
    gtk_widget_set_sensitive(f_Options, FALSE);
    gtk_widget_set_sensitive(b_QuickOpts, FALSE);
    gtk_widget_set_sensitive(bu_fire, TRUE);

    return FALSE;

}


static gboolean unlock_panels_fire_l(GtkWidget* bu_fire) {

    gtk_button_set_label(GTK_BUTTON(bu_fire),"FIRE !");

    gtk_widget_set_sensitive(f_Mode, TRUE);

    if ( !isInHivemind() ) {
        gtk_widget_set_sensitive(f_Options, TRUE);
        gtk_widget_set_sensitive(b_QuickOpts, TRUE);
    }

    gtk_widget_set_sensitive(bu_fire, TRUE);


    return FALSE;
}

static gboolean lock_panels_hivemind_l(GtkWidget* bu_hivemind) {

    gtk_button_set_label( GTK_BUTTON(bu_hivemind),"Disconnect");
    gtk_widget_set_sensitive(f_Options, FALSE);
    gtk_widget_set_sensitive(b_QuickOpts, FALSE);
    gtk_widget_set_sensitive(bu_hivemind, TRUE);


    return FALSE;

}


static gboolean unlock_panels_hivemind_l(GtkWidget* bu_hivemind) {

    gtk_button_set_label(GTK_BUTTON(bu_hivemind),"Connect");
    gtk_widget_set_sensitive(f_Mode, TRUE);
    gtk_widget_set_sensitive(f_Options, TRUE);
    gtk_widget_set_sensitive(b_QuickOpts, TRUE);
    gtk_widget_set_sensitive(bu_hivemind, TRUE);


    return FALSE;
}


int hivemind_from_gtk(GtkWidget* bu_hivemind) {

    g_idle_add( (GSourceFunc)lock_panels_hivemind_l, (gpointer) bu_hivemind);


    gtk_spinner_start( GTK_SPINNER(spinner_hivemind) );
    gtk_widget_show( GTK_WIDGET(spinner_hivemind) );

    int ret = start_hivemind( gtk_entry_buffer_get_text(t_irc_server),
                    atoi( gtk_entry_buffer_get_text(t_irc_port) ),
                    gtk_entry_buffer_get_text(t_irc_chan)
                    );

    setStatus( READY );
    imInHivemind( FALSE );
    g_idle_add( (GSourceFunc)unlock_panels_hivemind_l, (gpointer) bu_hivemind);



    return ret;
}




p_BOOL push_hivemind_button(GtkWidget* bu_hivemind, GtkWidget* spinner_hivemind) {


    if ( getStatus() == READY && isInHivemind() == FALSE ) {


        setStatus(HIVEMIND_CONNECTING);
        gtk_button_set_label( GTK_BUTTON(bu_hivemind),"Disconnect");


        if ( strlen( gtk_entry_buffer_get_text(t_irc_server) ) != 0
             && strlen( gtk_entry_buffer_get_text(t_irc_port) ) != 0
             && strlen( gtk_entry_buffer_get_text(t_irc_chan) ) != 0  ) {

            pthread_create(&thread_hivemind, NULL, (void*)hivemind_from_gtk, bu_hivemind);
        }
        else {
            loic_error("Bad Hivemind params.", GRAVE);
            g_idle_add( (GSourceFunc)unlock_panels_hivemind_l, (gpointer) bu_hivemind);
            setStatus( READY );
        }


    }
    else if ( getStatus() != HIVEMIND_CONNECTING || isInHivemind() == TRUE ) {
        if ( disconnect() == 0) {
            g_idle_add( (GSourceFunc)unlock_panels_hivemind_l, (gpointer) bu_hivemind);
        }
        else {
            loic_error("Error while disconnecting", MINOR);
            pthread_cancel( thread_hivemind );
        }

    }

    return TRUE;
}

p_BOOL fire_from_gtk(GtkWidget* bu_fire) {

    int ret = -1;

    #ifdef SUPER_LOIC
        if ( !isNetworkReady() ) {
            charge_superCanon();
        }
        ret = start_superCanon();
    #else
        ret = start_canon();
    #endif

    char* buffer = (char*) malloc( sizeof(char) * 1024 );

    switch( ret ) {
        case E_BUSY :
            loic_error("Not ready.", GRAVE);
            break;
        case E_UNKNOWN_HOST :
            sprintf(buffer,"Unknown host : %s", getTarget());
            loic_error(buffer, GRAVE);
            break;
        case E_NO_RESPONSE :
            sprintf(buffer,"No response : %s", getTarget());
            loic_error(buffer, GRAVE);
            break;
        default:
            break;
    }

    g_idle_add( (GSourceFunc)unlock_panels_fire_l, (gpointer) bu_fire);



    return ret;
}



#ifdef SUPER_LOIC

/* DEGUEULASSE : A REVOIR RAPIDEMENT (enfin, avant le reste kwa) */


static gboolean lock_detect_SYN_l( GtkWidget* b_detect ) {

    gtk_container_remove( GTK_CONTAINER(b_detect), GTK_WIDGET(bu_auto_detect_SYN) );
    gtk_container_add( GTK_CONTAINER(b_detect), GTK_WIDGET(spinner_detection_SYN) );
    gtk_spinner_start( GTK_SPINNER(spinner_detection_SYN) );
    gtk_widget_show( GTK_WIDGET(spinner_detection_SYN) );

    return FALSE;
}

static gboolean unlock_detect_SYN_l( GtkWidget* b_detect ) {


    gtk_spinner_stop( GTK_SPINNER(spinner_detection_SYN) );
    gtk_container_remove( GTK_CONTAINER(b_detect), GTK_WIDGET(spinner_detection_SYN) );
    gtk_container_add( GTK_CONTAINER(b_detect), GTK_WIDGET(bu_auto_detect_SYN) );


    return FALSE;
}

static gboolean lock_detect_SUDP_l( GtkWidget* b_detect ) {

    gtk_container_remove( GTK_CONTAINER(b_detect), GTK_WIDGET(bu_auto_detect_SUDP) );
    gtk_container_add( GTK_CONTAINER(b_detect), GTK_WIDGET(spinner_detection_SUDP) );
    gtk_spinner_start( GTK_SPINNER(spinner_detection_SUDP) );
    gtk_widget_show( GTK_WIDGET(spinner_detection_SUDP) );

    return FALSE;
}

static gboolean unlock_detect_SUDP_l( GtkWidget* b_detect ) {


    gtk_spinner_stop( GTK_SPINNER(spinner_detection_SUDP) );
    gtk_container_remove( GTK_CONTAINER(b_detect), GTK_WIDGET(spinner_detection_SUDP) );
    gtk_container_add( GTK_CONTAINER(b_detect), GTK_WIDGET(bu_auto_detect_SUDP) );


    return FALSE;
}

/* DEGUEULASSE : A REVOIR RAPIDEMENT */

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


    p_BOOL thread_autodetectSYN( GtkWidget* b_detect )  {

        g_idle_add( (GSourceFunc)lock_detect_SYN_l, (gpointer) b_detect);


        int my_index = -1;
        my_index = auto_detect_custom_device();

        if ( my_index != -1 ) {

            gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_devices_SYN), my_index );

            g_idle_add( (GSourceFunc)unlock_detect_SYN_l, (gpointer) b_detect);


            return TRUE;

        }

        return FALSE;

    }

    p_BOOL thread_autodetectSUDP( GtkWidget* b_detect )  {

        g_idle_add( (GSourceFunc)lock_detect_SUDP_l, (gpointer) b_detect);


        int my_index = -1;
        my_index = auto_detect_custom_device();

        if ( my_index != -1 ) {

            gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_devices_SUDP), my_index );


            g_idle_add( (GSourceFunc)unlock_detect_SUDP_l, (gpointer) b_detect);



            return TRUE;

        }

        return FALSE;

    }



    p_BOOL select_device( GtkWidget* combo_devices ) {


        int index = gtk_combo_box_get_active ( GTK_COMBO_BOX(combo_devices) );

        if ( index >= 0 ) {

            if ( combo_devices == combo_devices_SUDP ) {
                if ( gtk_combo_box_get_active ( GTK_COMBO_BOX(combo_devices_SYN) ) != index ) {
                    gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_devices_SYN), index );
                }
            }
            else if ( combo_devices == combo_devices_SYN ) {
                if ( gtk_combo_box_get_active ( GTK_COMBO_BOX(combo_devices_SUDP) ) != index ) {
                    gtk_combo_box_set_active ( GTK_COMBO_BOX(combo_devices_SUDP), index );

                }
            }

            char* buffer = (char*) malloc( sizeof(char) * 32 );
            strcpy(buffer,get_ip_from_custom_device( index ));

            gtk_entry_buffer_set_text( t_source_ip, buffer, strlen(buffer) );

        }
        else
            return FALSE;


        return TRUE;

    }
#endif

p_BOOL push_auto_button(GtkWidget* bu_autodetect, GtkWidget* b_detect) {


#ifdef SUPER_LOIC

    pthread_t thread_detect;

    if ( bu_autodetect == bu_auto_detect_SYN )
        pthread_create(&thread_detect, NULL, (void*)thread_autodetectSYN, b_detect);
    else
        pthread_create(&thread_detect, NULL, (void*)thread_autodetectSUDP, b_detect);


#else

    loic_error("Only available in superLOIC version.", GRAVE);

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


    if ( getStatus() == FIRING ) {

        int nb_packets = 0;

        int i;
        for ( i=0; i<getNbThreads(); i++ ) {
            nb_packets += getNbPacketsSent(i);
        }

        char* buffer = (char*) malloc( sizeof(char) * 64 );
        p_itoa(nb_packets, buffer);
        strcat(buffer," packets sent !");
        status_message ( buffer );


        printf("maj (%d)\n",nb_packets);
    }


    return TRUE;
}


static gboolean push_fire_button(GtkWidget* bu_fire) {



    p_BOOL ret = TRUE;

    g_idle_add( (GSourceFunc)lock_panels_fire_l, (gpointer) bu_fire);


    if ( getStatus() == FIRING ) {
        setStatus( STOPPING );

        ret = FALSE;
    }
    else if ( setTarget( gtk_entry_buffer_get_text(t_target) ) != 0)
        ret = FALSE;
    else if ( setPort( atoi( gtk_entry_buffer_get_text(t_port) )) != 0)
        ret = FALSE;
    else if ( setTimeout( atoi( gtk_entry_buffer_get_text(t_timeout) )) != 0)
        ret = FALSE;
    else if ( setNbThreads( atoi( gtk_entry_buffer_get_text(t_nb_threads) )) != 0)
        ret = FALSE;

    if ( gtk_entry_buffer_get_length(t_file_udpMessage) > 0 ) {

        char* buffer = (char*) malloc( sizeof(char) * (gtk_entry_buffer_get_length(t_file_udpMessage) + 1) );
        strcpy(buffer,gtk_entry_buffer_get_text(t_file_udpMessage));
        removeFilePrefix( buffer );

        if ( setUdpMessage ( buffer, S_MULTIPLE ) != 0 ) {
            ret = FALSE;
        }
    }
    else if ( setUdpMessage ( gtk_entry_buffer_get_text(t_udp_message), S_SIMPLE ) != 0) {
        ret = FALSE;
    }


    if ( gtk_entry_buffer_get_length(t_file_httpSubsite) > 0 ) {

        char* buffer = (char*) malloc( sizeof(char) * (gtk_entry_buffer_get_length(t_file_httpSubsite) + 1) );
        strcpy(buffer,gtk_entry_buffer_get_text(t_file_httpSubsite));
        removeFilePrefix( buffer );


        if ( setSubsite ( buffer, S_MULTIPLE ) != 0) {
            ret = FALSE;
        }
    }
    else if ( setSubsite ( gtk_entry_buffer_get_text(t_http_subsite), S_SIMPLE ) != 0) {
        ret = FALSE;
    }

    if ( ret != FALSE ) {

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


            if ( checkLoicCompatibility() ) {
                pthread_t thread_canon;
                pthread_create( &thread_canon, NULL, (void*)fire_from_gtk, bu_fire );
            }
            else {
                ret = FALSE;
                loic_error( "Only available in superLOIC version !", GRAVE );
            }


        }
        else if ( isInHivemind() ) {
            loic_error( "You have to define the target before start firing !", MINOR );
        }
        else {
            loic_error( "You haven't defined the target ..", GRAVE );
            ret = FALSE;
        }

    }
    else {
        printf("Canceled\n");
        g_idle_add( (GSourceFunc)unlock_panels_fire_l, (gpointer) bu_fire);

    }

    return FALSE;
}


void sim_push_fire_button() {

    g_idle_add( (GSourceFunc)push_fire_button, (gpointer) bu_fire);

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

            GtkWidget* fcb_data = gtk_file_chooser_button_new( "From file", GTK_FILE_CHOOSER_ACTION_OPEN );
            GtkWidget* fce_data = gtk_entry_new_with_buffer( t_file_udpMessage );


            cb_randCharsUDP = gtk_check_button_new_with_label("Random Mode ?");
        GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_cData, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_cData, FALSE, TRUE, 5 );

        gtk_box_pack_start ( GTK_BOX(b_dataOpts), fcb_data, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), fce_data, FALSE, TRUE, 5 );

        gtk_box_pack_start ( GTK_BOX(b_dataOpts), cb_randCharsUDP, FALSE, TRUE, 5 );

        cb_waitForReplyUDP = gtk_check_button_new_with_label("Wait for Reply ?");
        GtkWidget* b_checkOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_checkOpts), cb_waitForReplyUDP, FALSE, TRUE, 5 );



        b_OptionsUDP = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsUDP), l_description, FALSE, TRUE, 5);
        gtk_box_pack_start ( GTK_BOX(b_OptionsUDP), b_dataOpts, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsUDP), b_checkOpts, FALSE, TRUE, 0 );



        g_object_ref(b_OptionsUDP);

        g_signal_connect( G_OBJECT(fcb_data), "selection-changed", G_CALLBACK(fileChoose), t_file_udpMessage );
        g_signal_connect( G_OBJECT(t_file_udpMessage), "deleted-text", G_CALLBACK(dText), e_cData);
        g_signal_connect( G_OBJECT(t_file_udpMessage), "inserted-text", G_CALLBACK(iText), e_cData);



    }
    else if  (method == TCP ) {

        GtkWidget* l_description = gtk_label_new("TCP : Open TCP sockets (both with 3-ways handhsake)");



            l_cData = gtk_label_new(" TCP / UDP message :");
            e_cData = gtk_entry_new_with_buffer(t_udp_message);
            gtk_entry_set_width_chars( GTK_ENTRY(e_cData), 40);
            cb_randCharsTCP = gtk_check_button_new_with_label("Random Mode ?");
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

            GtkWidget* fcb_httpSub = gtk_file_chooser_button_new( "From file", GTK_FILE_CHOOSER_ACTION_OPEN );
            GtkWidget* fce_httpSub = gtk_entry_new_with_buffer( t_file_httpSubsite );


            cb_randCharsHTTP = gtk_check_button_new_with_label("Random Mode ?");
        GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_httpSub, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_httpSub, FALSE, TRUE, 5 );

        gtk_box_pack_start ( GTK_BOX(b_dataOpts), fcb_httpSub, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), fce_httpSub, FALSE, TRUE, 5 );

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


        g_signal_connect( G_OBJECT(fcb_httpSub), "selection-changed", G_CALLBACK(fileChoose), t_file_httpSubsite );
        g_signal_connect( G_OBJECT(t_file_httpSubsite), "deleted-text", G_CALLBACK(dText), e_httpSub);
        g_signal_connect( G_OBJECT(t_file_httpSubsite), "inserted-text", G_CALLBACK(iText), e_httpSub);


    }
    else if  (method == SLOW_HTTP ) {



        GtkWidget* l_description = gtk_label_new("Slowloic Mode : Like Slowloris perl program. Make half-connections to HTTP server usefull in \"simple\" DOS, not DDOS");

            l_httpSub = gtk_label_new(" HTTP URL Subsite :");
            e_httpSub = gtk_entry_new_with_buffer( t_http_subsite );
            gtk_entry_set_width_chars( GTK_ENTRY(e_httpSub),15 );
            cb_randCharsSLOW_HTTP = gtk_check_button_new_with_label("Random Mode ?");
        GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_httpSub, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_httpSub, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), cb_randCharsSLOW_HTTP, FALSE, TRUE, 5 );


            cb_useGzipSLOW_HTTP = gtk_check_button_new_with_label("Use Gzip ?");
            cb_waitForReplySLOW_HTTP = gtk_check_button_new_with_label("Wait for Reply ?");
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


         GtkWidget* b_superOpts = gtk_hbox_new( FALSE, 0 );

#ifdef SUPER_LOIC

         GtkWidget* l_choose_device = gtk_label_new(" Device");


         gtk_box_pack_start ( GTK_BOX(b_superOpts), l_choose_device, FALSE, TRUE, 5 );
         gtk_box_pack_start ( GTK_BOX(b_superOpts), combo_devices_SUDP, FALSE, TRUE, 5 );
#endif

         GtkWidget* b_detect = gtk_vbox_new( FALSE, 0 );


         gtk_box_pack_start ( GTK_BOX(b_detect), bu_auto_detect_SUDP, FALSE, TRUE, 5 );
         gtk_box_pack_start ( GTK_BOX(b_superOpts), b_detect, FALSE, TRUE, 5 );



            l_ipSource = gtk_label_new(" Source IP address :");
            e_ipSource = gtk_entry_new_with_buffer( t_source_ip );
            gtk_entry_set_width_chars( GTK_ENTRY(e_ipSource),15 );
            cb_randIPSUDP = gtk_check_button_new_with_label("Use random source IP address ?");


        GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_ipSource, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_ipSource, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_dataOpts), cb_randIPSUDP, FALSE, TRUE, 5 );


        g_signal_connect( G_OBJECT(bu_auto_detect_SUDP), "clicked", G_CALLBACK(push_auto_button), b_detect );



        b_OptionsSUDP = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSUDP), l_description, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSUDP), b_superOpts, FALSE, TRUE, 5 );
        gtk_box_pack_start ( GTK_BOX(b_OptionsSUDP), b_dataOpts, FALSE, TRUE, 5 );


        g_signal_connect( G_OBJECT(cb_randIPSUDP), "clicked", G_CALLBACK(randomIPchecked), e_ipSource );


        g_object_ref(b_OptionsSUDP);


    }
    else if  (method == SYN ) {

        GtkWidget* l_description = gtk_label_new(" SYN Flood Mode : Avalaible in superLOIC version. Send SYN requests to a server.");






         GtkWidget* b_superOpts = gtk_hbox_new( FALSE, 0 );

#ifdef SUPER_LOIC

         GtkWidget* l_choose_device = gtk_label_new(" Device");


         gtk_box_pack_start ( GTK_BOX(b_superOpts), l_choose_device, FALSE, TRUE, 5 );
         gtk_box_pack_start ( GTK_BOX(b_superOpts), combo_devices_SYN, FALSE, TRUE, 5 );

#endif


         GtkWidget* b_detect = gtk_vbox_new( FALSE, 0 );


         gtk_box_pack_start ( GTK_BOX(b_detect), bu_auto_detect_SYN, FALSE, TRUE, 5 );
         gtk_box_pack_start ( GTK_BOX(b_superOpts), b_detect, FALSE, TRUE, 5 );



            l_ipSource = gtk_label_new(" Source IP address :");
            e_ipSource = gtk_entry_new_with_buffer( t_source_ip );
            gtk_entry_set_width_chars( GTK_ENTRY(e_ipSource), 15);
            cb_randIPSYN = gtk_check_button_new_with_label("Use random source IP address ?");


            GtkWidget* b_dataOpts = gtk_hbox_new( FALSE, 0 );
            gtk_box_pack_start ( GTK_BOX(b_dataOpts), l_ipSource, FALSE, TRUE, 5 );
            gtk_box_pack_start ( GTK_BOX(b_dataOpts), e_ipSource, FALSE, TRUE, 5 );
            gtk_box_pack_start ( GTK_BOX(b_dataOpts), cb_randIPSYN, FALSE, TRUE, 5 );


            g_signal_connect( G_OBJECT(bu_auto_detect_SYN), "clicked", G_CALLBACK(push_auto_button), b_detect );




            b_OptionsSYN = gtk_vbox_new( FALSE, 0 );
            gtk_box_pack_start ( GTK_BOX(b_OptionsSYN), l_description, FALSE, TRUE, 5 );
            gtk_box_pack_start ( GTK_BOX(b_OptionsSYN), b_superOpts, FALSE, TRUE, 5 );
            gtk_box_pack_start ( GTK_BOX(b_OptionsSYN), b_dataOpts, FALSE, TRUE, 5 );



            g_signal_connect( G_OBJECT(cb_randIPSYN), "clicked", G_CALLBACK(randomIPchecked), e_ipSource );


        g_object_ref(b_OptionsSYN);
    }



    return TRUE;
}

p_BOOL paintOptionsPanel(GtkWidget* sender, gpointer data) {

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

        currentOptionPanel = b_OptionsTCP;


    }
    else if ( method == UDP ) {

        currentOptionPanel = b_OptionsUDP;

    }
    else if ( method == HTTP ) {

        currentOptionPanel = b_OptionsHTTP;

    }
    else if ( method == SLOW_HTTP ) {

        currentOptionPanel = b_OptionsSLOW_HTTP;

    }
    else if ( method == SYN ) {

        currentOptionPanel = b_OptionsSYN;

    }
    else if ( method == SUDP ) {

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




                            GtkWidget* b_Target = NULL;
                                GtkWidget* l_url = NULL;
                                GtkWidget* e_url = NULL;
                                GtkWidget* bu_url = NULL;

                            GtkWidget* b_Method = NULL;
                                GtkWidget* l_method = NULL;






                        /*
                    GtkWidget* f_Options = NULL;
                    GtkWidget* b_Options = NULL;
                    */


                GtkWidget* f_Status = NULL;


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

    t_file_udpMessage = gtk_entry_buffer_new("",0);
    t_file_httpSubsite = gtk_entry_buffer_new("",0);



    /* Main Window */
    mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(mainWindow), "delete-event", G_CALLBACK(gtk_main_quit), NULL);



    char title[256];
    #ifdef SUPER_LOIC
        strcpy(title,"SuperLoic");
    #else
        strcpy(title,"SimpleLoic");
    #endif


    gtk_window_set_title( GTK_WINDOW(mainWindow), title );
    gtk_window_set_default_size( GTK_WINDOW(mainWindow), 640, 400 );
    gtk_window_set_resizable ( GTK_WINDOW(mainWindow), TRUE );
    gtk_window_set_icon_from_file( GTK_WINDOW(mainWindow), "ressources/ugly_icon.png", NULL );

    /* End Main Window */

    spinner_detection_SUDP = gtk_spinner_new();
    spinner_detection_SYN = gtk_spinner_new();


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


                bu_Hivemind = gtk_button_new_with_label("Connect");

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

                /*
                gtk_container_add ( GTK_CONTAINER(b_Target), bu_url );
                */


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
                //gtk_widget_set_usize(bb_Options,-1,300);
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

            gtk_widget_set_usize(statusBar,200,30);

            gtk_statusbar_push ( GTK_STATUSBAR(statusBar), 0, "Ready.");


            f_Status = gtk_frame_new("Status");

            gtk_container_add ( GTK_CONTAINER(f_Status), statusBar );


        b_Right = gtk_vbox_new( FALSE, 0 );

        gtk_box_pack_start ( GTK_BOX(b_Right), f_Mode, FALSE, FALSE, 10 );
        gtk_box_pack_start ( GTK_BOX(b_Right), f_Action, FALSE, FALSE, 10 );
        gtk_box_pack_start ( GTK_BOX(b_Right), f_Options, FALSE, FALSE, 10 );
        gtk_box_pack_start ( GTK_BOX(b_Right), f_Status, FALSE, FALSE, 10 );


        GtkWidget* imgL = gtk_image_new_from_file ("ressources/psfire.png");
        GtkWidget* url_github = gtk_link_button_new_with_label( "https://github.com/Peacemaking/newLOIC",
                                                                "https://github.com/Peacemaking/newLOIC");


        b_Left = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start ( GTK_BOX(b_Left), imgL, FALSE, FALSE, 10 );
        gtk_box_pack_start ( GTK_BOX(b_Left), url_github, FALSE, FALSE, 0 );



    b_Main = gtk_hbox_new( FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX(b_Main), b_Left, FALSE, FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX(b_Main), b_Right, FALSE, FALSE, 10 );


    gtk_container_add ( GTK_CONTAINER(mainWindow), b_Main );


#ifdef SUPER_LOIC

    combo_devices_SUDP = create_interface_list();
    g_signal_connect( G_OBJECT( combo_devices_SUDP ), "changed", G_CALLBACK(select_device), NULL);
    g_object_ref(combo_devices_SUDP);


    combo_devices_SYN = create_interface_list();
    g_signal_connect( G_OBJECT( combo_devices_SYN ), "changed", G_CALLBACK(select_device), NULL);
    g_object_ref(combo_devices_SYN);
#endif


    bu_auto_detect_SUDP = gtk_button_new_with_label("Auto-Detect");
    bu_auto_detect_SYN = gtk_button_new_with_label("Auto-Detect");


    g_object_ref(bu_auto_detect_SUDP);
    g_object_ref(bu_auto_detect_SYN);

    g_object_ref(spinner_detection_SUDP);
    g_object_ref(spinner_detection_SYN);



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
