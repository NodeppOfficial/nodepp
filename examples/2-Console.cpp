#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    console::log("this is a simple message");
    
    console::info("this is a message info");
    
    console::done("this is a done message");
    
    console::error("this is an error message");
    
    console::success("this is a done message");

    console::warning("this is a warning message");

}