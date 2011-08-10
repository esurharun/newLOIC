
#include "config.h"
#include "overlord.h"
#include <time.h>
#include "gtk_module.h"


int main(int argc, char *argv[]) {

    srand(time(NULL));


    reset_config();

    init(); /* Use sockets > Windows */



    gtk_init(&argc, &argv);
    gtk_rc_parse("gtkrc");

    build_window();

    gtk_main();


    end(); /* End of ' Use sockets > Windows ' */


    return EXIT_SUCCESS;
}
