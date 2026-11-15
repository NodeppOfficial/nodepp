#include <nodepp/nodepp.h>
#include <nodepp/json.h>

using namespace nodepp;

void onMain(){

    map_t<string_t,int> map ({
        { "var1", 10 },
        { "var2", 20 },
        { "var3", 30 },
        { "var4", 40 },
        { "var5", 50 }
    });

    queue_t<void*> mem; auto &que = map.raw();

    auto x = que.last(); while( x!=nullptr ){
         mem.push( (void*) x ); 
    x = x->prev; }

    auto y = mem.last(); while( y!=nullptr ){
         que.swap( que.get( os::rand() ), que.as( y->data ) );
    y = y->prev; }

    console::log( json::stringify( map ) );

}