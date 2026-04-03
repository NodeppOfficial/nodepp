/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_COOKIE
#define NODEPP_COOKIE

/*────────────────────────────────────────────────────────────────────────────*/

#include "regex.h"
#include "map.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using cookie_t = map_t< string_t, string_t >;
namespace cookie {

    inline query_t parse( string_t data ){
    thread_local static regex_t reg("([^= ;]+)=([^;]+)");

        if( data.empty() /*-----------*/ )
          { return nullptr; } query_t out;
        
        reg.search_all( data ); auto mem = reg.get_memory();
        reg.clear_memory();
        
        for( ulong x=0; x<mem.size(); x+=2 ){
             auto  y=mem.slice_view( x,x+2 );
        if ( y.size()!=2 ){ break; }
             out[ y[0] ] = y[1];
        }
        
        return out;
    }
    
    /*─······································································─*/
    
    inline string_t format( const cookie_t& data ){
        if ( data.empty() ){ return nullptr; } queue_t<string_t> out; 
        for( auto x:data.data() ){ out.push( x.first + "=" + x.second ); }   
        return string::format("%s",array_t<string_t>(out.data()).join("; ").c_str());
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif