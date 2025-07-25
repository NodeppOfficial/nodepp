#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    conio::foreground( C_GREEN | C_BOLD );
    conio::log(" Hello World! \n");

    conio::foreground( C_RED | C_BOLD );
    conio::log(" Hello World! \n");

    conio::foreground( C_YELLOW | C_BOLD );
    conio::log(" Hello World! \n");

    conio::foreground( C_CYAN | C_BOLD );
    conio::log(" Hello World! \n");

    conio::foreground( C_MAGENTA | C_BOLD );
    conio::log(" Hello World! \n");

    conio::foreground( C_WHITE | C_BOLD );
    conio::log(" Hello World! \n");

    conio::foreground( C_BLACK | C_BOLD );
    conio::background( C_WHITE );
    conio::log(" Hello World! \n");

    conio::log(" Hello World! \n");

}