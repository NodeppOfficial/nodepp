#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain() {

    process::add([=](){ 
        
        process::clear(); // <- this must crash the program | bug fixed :)

        process::add([=](){
            console::log( "AAA", process::size() );
        return 1; });

        process::add([=](){
            console::log( "BBB", process::size() );
        return 1; });

        process::add([=](){
            console::log( "CCC", process::size() );
        return 1; });

        process::add([=](){
            console::log( "DDD", process::size() );
            process::delay(1000);
        return 1; });

        console::log( process::size() );

    return -1; });

    process::add([=](){ console::log( "---" ); return 1; });

}