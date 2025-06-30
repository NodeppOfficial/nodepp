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
    
    string_t hostname(){ char buff[UNBFF_SIZE]; ::gethostname(buff,UNBFF_SIZE); return buff; }
    
    /*─······································································─*/

    string_t cwd(){ char buff[UNBFF_SIZE]; ::getcwd(buff,UNBFF_SIZE); return buff; }
    
    /*─······································································─*/

    uint cpus(){ return ::sysconf( _SC_NPROCESSORS_ONLN ); }
    
    /*─······································································─*/

    void exec( string_t cmd ){ ::system( cmd.get() ); }

    void call( string_t cmd ){ ::system( cmd.get() ); }
    
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