#include <nodepp/nodepp.h>
#include <nodepp/map.h>

using namespace nodepp;

void onMain(){

    map_t<string_t,int> counter ({
        { "var1", 3   },
        { "var2", 90  },
        { "var3", 100 },
        { "var4", 90  },
    });

    auto raw = array_t( counter.raw().data() ).sort([&]( 
        pair_t<string_t,int> a,
        pair_t<string_t,int> b 
    ){ return a.second >= b.second; });

    for( auto x: raw ){ console::log( x.first, x.second ); }

}