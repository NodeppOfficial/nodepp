#include <nodepp/nodepp.h>
#include <nodepp/https.h>

using namespace nodepp;

void onMain(){

    file_t fin1 ( "LICENSE", "r" );
    file_t fin2 ( "LICENSE", "r" );
    file_t fin3 ( "LICENSE", "r" );
    file_t fout ( "OUTPUT" , "w" );

    fin2.onDrain([=](){
        fout.write( "\n---\n" );
        stream::pipe( fin3, fout );
        console::log( "fin3 -> fout" );
    });

    fin1.onDrain([=](){
        fout.write( "\n---\n" );
        stream::pipe( fin2, fout );
        console::log( "fin2 -> fout" );
    });

    fout.onClose([=](){
        console::log( "DONE" );
    });

    fout.write( "\n---\n" );
    stream::pipe( fin1, fout );
    console::log( "fin1 -> fout" );

}