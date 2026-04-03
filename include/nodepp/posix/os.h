/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_OS
#define NODEPP_POSIX_OS

/*────────────────────────────────────────────────────────────────────────────*/

#include <unistd.h>
#include <cerrno>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {
    
    inline string_t hostname(){ char buff[UNBFF_SIZE]; return ::gethostname(buff,UNBFF_SIZE)==0 ? buff : nullptr; }
    
    /*─······································································─*/

    inline string_t cwd(){ char buff[UNBFF_SIZE]; return ::getcwd(buff,UNBFF_SIZE)==nullptr ? nullptr : buff; }
    
    /*─······································································─*/

    inline uint cpus(){ return ::sysconf( _SC_NPROCESSORS_ONLN ); }
    
    /*─······································································─*/

    inline int exec( string_t cmd ){ return ::system( cmd.get() ); }

    inline int call( string_t cmd ){ return ::system( cmd.get() ); }
    
    /*─······································································─*/

    inline string_t user(){ return ::getlogin(); }
    
    /*─······································································─*/

    inline string_t tmp(){ return "/tmp"; }

    /*─······································································─*/

    inline uint pid(){ return ::getpid(); }

    /*─······································································─*/

    inline uint error(){ return errno; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/