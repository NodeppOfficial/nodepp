/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once

#include <unistd.h>
#include <cerrno>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {
    
    string_t hostname(){ char buff[UNBFF_SIZE]; return ::gethostname(buff,UNBFF_SIZE)==0 ? buff : nullptr; }
    
    /*─······································································─*/

    string_t cwd(){ char buff[UNBFF_SIZE]; return ::getcwd(buff,UNBFF_SIZE)==nullptr ? nullptr : buff; }
    
    /*─······································································─*/

    uint cpus(){ return ::sysconf( _SC_NPROCESSORS_ONLN ); }
    
    /*─······································································─*/

    int exec( string_t cmd ){ return ::system( cmd.get() ); }

    int call( string_t cmd ){ return ::system( cmd.get() ); }
    
    /*─······································································─*/

    string_t user(){ return ::getlogin(); }
    
    /*─······································································─*/

    string_t tmp(){ return "/tmp"; }

    /*─······································································─*/

    uint pid(){ return ::getpid(); }

    /*─······································································─*/

    uint error(){ return errno; }

}}

/*────────────────────────────────────────────────────────────────────────────*/