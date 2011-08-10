#ifndef H_GTK_MODULE_H

    #define H_GTK_MODULE_H

    #include <stdlib.h>
    #include "const.h"
    #include "config.h"


    #ifdef WIN32
        #include <gtk/gtk.h>
    #elif defined (linux)
        #include <gtk/gtk.h>
    #endif

#ifdef SUPER_LOIC
    #include "superGears.h"
#endif


    p_BOOL build_window();
    void display_popup(const char message[256]);
    void status_message(const char message[256]);

    p_BOOL refresh_opts();
    p_BOOL refresh_infos();


#endif
