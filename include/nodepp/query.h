/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_QUERY
#define NODEPP_QUERY

/*────────────────────────────────────────────────────────────────────────────*/

#include "regex.h"
#include "map.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using query_t = map_t< string_t, string_t >;
namespace query  {

    query_t parse( string_t data ){
        static regex_t reg("[?&]([^= ]+)=([^?&]+)");

        if( data.empty() || data[0]!='?' ){ return query_t(); } query_t out;
        
        reg.search_all( data ); auto mem = reg.get_memory();
        reg.clear_memory();

        while( !mem.empty() ){ auto data = mem.splice( 0, 2 );
           if( data.size()!=2 ){ break; } 
               out[ data[0] ] = data[1];
        }  return out;
    }
    
    /*─······································································─*/
    
    string_t format( const query_t& data ){ 
        if( data.empty() ){ return nullptr; } /*------*/
        array_t<string_t> out; for( auto x:data.data() ) 
             { out.push( x.first + "=" + x.second ); }
        return string::format("?%s",out.join("&").c_str());
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif