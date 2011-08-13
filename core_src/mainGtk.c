
#include "config.h"
#include "overlord.h"
#include <time.h>
#include "gtk_module.h"


int main(int argc, char *argv[]) {

    srand(time(NULL));


    reset_config();

    init(); /* Use sockets > Windows */

    g_type_init();
    gdk_threads_init();

    gtk_init(&argc, &argv);
    gtk_rc_parse("gtkrc");

    build_window();

    gdk_threads_enter();
    gtk_main();
    gdk_threads_leave();


    end(); /* End of ' Use sockets > Windows ' */


    return EXIT_SUCCESS;
}
