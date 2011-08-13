#ifndef H_GTK_MODULE_H

    #define H_GTK_MODULE_H

    #include <stdlib.h>
    #include "const.h"
    #include "config.h"

    #define G_THREADS_ENABLED

    #ifdef WIN32
        #include <gdk/gdk.h>
        #include <gtk/gtk.h>
        #include <glib.h>
    #elif defined (linux)
        #include <gdk/gdk.h>
        #include <gtk/gtk.h>
        #include <glib.h>
    #endif

#ifdef SUPER_LOIC
    #include "superGears.h"
#endif


    p_BOOL build_window();
    void display_popup(const char message[256]);
    void status_message(const char message[256]);

    p_BOOL refresh_opts();
    p_BOOL refresh_infos();

    void sim_push_fire_button();


#endif
