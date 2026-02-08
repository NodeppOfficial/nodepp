/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_ENV
#define NODEPP_WINDOWS_ENV

/*────────────────────────────────────────────────────────────────────────────*/

#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { namespace env {

    inline int    set( const string_t& name, const string_t& value ){ return SetEnvironmentVariableA( name.c_str(), value.c_str() ); }
    
    inline int remove( const string_t& name ){ return SetEnvironmentVariableA( name.c_str(), nullptr ); }

    inline string_t get( const string_t& name ){ ptr_t<char> buffer ( UNBFF_SIZE );
        auto x = GetEnvironmentVariableA( name.c_str(), &buffer, buffer.size() );
        return string_t( &buffer, (ulong) x );
    }

    inline int init( const string_t& path ){ try {

        thread_local static regex_t reg( "^([^ =]+)[= \"]+([^\n#\"]+)" );
        auto file = file_t( path, "r" );

        while( !file.is_closed() ){
            /*--------*/ reg.match_all( file.read_line() );
            auto match = reg.get_memory  ();
            /*--------*/ reg.clear_memory();

            if ( match.size() != 2 ){ continue; } 
            set( match[0], match[1] );
        }
        
    } catch(...) { return -1; } return 1; }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    inline bool  is_child(){ return !env::get("CHILD").empty(); }

    inline bool is_parent(){ return  env::get("CHILD").empty(); }

    inline string_t  home(){ return  env::get("USERPROFILE"); }

    inline string_t shell(){ return  env::get("COMSPEC");     }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/