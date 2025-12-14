#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain() {

    conio::background( conio::color::cyan | conio::color::bold );
    console::log( "hello world!" );
    conio::reset();
    
    conio::foreground( conio::color::red | conio::color::bold );
    console::log( "hello world!" );
    conio::reset();

    conio::background( conio::color::magenta | conio::color::bold );
    console::log( "hello world!" );
    conio::reset();
    
    conio::foreground( conio::color::blue | conio::color::bold );
    console::log( "hello world!" );
    conio::reset();

    conio::background( conio::color::yellow | conio::color::bold );
    console::log( "hello world!" );
    conio::reset();
    
    conio::foreground( conio::color::green | conio::color::bold );
    console::log( "hello world!" );
    conio::reset();

}