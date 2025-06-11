#include <nodepp/nodepp.h>
#include <nodepp/json.h>

using namespace nodepp;

void onMain(){

    object_t ppt ({
        
        { "var1", 1000 },
        
        { "var2", true },

        { "var3", object_t({ 
            { "var1", 1000 },
            { "var2", true },
            { "var3", "Hello World!" }
        })},

        { "var4", array_t<int>({ 10, 20, 30, 40, 50 }) },

        { "var6", array_t<object_t>({
            object_t({
                { "var1", 1000 },
                { "var2", true },
                { "var3", "Hello World!" }
            }),
            object_t({
                { "var1", 1000 },
                { "var2", true },
                { "var3", "Hello World!" }
            }),
            object_t({
                { "var1", 1000 },
                { "var2", true },
                { "var3", "Hello World!" }
            }),
        })},

        { "var5", array_t<object_t>({
            object_t( "Bye World!" ),
            object_t( true ),
            object_t( 10 )
        })}
        
    });

    console::log( json::stringify( ppt ) );

}