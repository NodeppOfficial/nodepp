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

namespace nodepp {

    using query_t = map_t< string_t, string_t >;

    namespace query {

        query_t parse( string_t data ){
            if ( data.empty() || data[0]!='?' ){ return query_t(); } query_t out;
            auto mem = regex::get_memory( data, "[?&]([^= ]+)=([^?&]+)" );
            auto beg = mem.begin();while( beg != mem.end() ){
                 out[ *beg ] = *( beg+1 );
            ++beg; ++beg; } return out;
        }
        
        /*─······································································─*/
        
        string_t format( const query_t& data ){ 
            array_t<string_t> out; for( auto x:data.data() ) 
                   out.push( x.first + "=" + x.second );
            return string::format("?%s",out.join("&").c_str());
        }

    }

}

/*────────────────────────────────────────────────────────────────────────────*/

#endif