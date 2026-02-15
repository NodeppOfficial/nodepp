#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    conio::foreground( conio::color::green | conio::color::bold );
    conio::log(" Hello World! \n");

    conio::foreground( conio::color::red | conio::color::bold );
    conio::log(" Hello World! \n");

    conio::foreground( conio::color::yellow | conio::color::bold );
    conio::log(" Hello World! \n");

    conio::foreground( conio::color::cyan | conio::color::bold );
    conio::log(" Hello World! \n");

    conio::foreground( conio::color::magenta | conio::color::bold );
    conio::log(" Hello World! \n");

    conio::foreground( conio::color::white | conio::color::bold );
    conio::log(" Hello World! \n");

    conio::foreground( conio::color::black | conio::color::bold );
    conio::background( conio::color::white );
    conio::log(" Hello World! \n");

    conio::log(" Hello World! \n");

}