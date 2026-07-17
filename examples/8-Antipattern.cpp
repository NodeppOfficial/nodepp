#include <nodepp/nodepp.h>
#include <nodepp/event.h>

using namespace nodepp;

class antipattern_t { public:

    event_t<> onSomething;

    antipattern_t(){ console::log( "constructor" ); }
   ~antipattern_t(){ console::log( "destructor"  ); }

    void print() const noexcept { console::log( "hello world!" ); }

};

void onMain(){

    ptr_t<antipattern_t> ptr ( 0UL );

    ptr->onSomething([=](){ 
        ptr->print(); // <- antipattern - circular dependency
    }); 

    ptr->onSomething.emit();

    // never prints destructor ( memory leak )

}

/* SOLUTION 

#include <nodepp/nodepp.h>
#include <nodepp/event.h>

using namespace nodepp;

class goodpattern_t { public:

    event_t<ptr_t<goodpattern_t>> onSomething;

    goodpattern_t(){ console::log( "constructor" ); }
   ~goodpattern_t(){ 
        onSomething.clear(); // <- clear circular references
        console::log( "destructor"  ); 
    }

    void print() const noexcept { console::log( "hello world!" ); }

};

void onMain(){

    ptr_t<goodpattern_t> ptr ( 0UL );

    ptr->onSomething([=]( const goodpattern_t* self ){ 
        self->print(); // <- no circular dependency
    }); 

    ptr->onSomething.emit( ptr );

    // it must prints destructor

}

*/