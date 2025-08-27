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
#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {
    
    string_t hostname(){
        char buffer[UNBFF_SIZE]; DWORD bufferSize = UNBFF_SIZE;
        GetComputerNameA(buffer,&bufferSize); return string_t( buffer, bufferSize );
    }
    
    /*─······································································─*/

    string_t user(){ 
        char buffer[UNBFF_SIZE]; DWORD bufferSize = UNBFF_SIZE;
        GetUserNameA(buffer, &bufferSize); return string_t( buffer, bufferSize );
    }
    
    /*─······································································─*/

    string_t cwd(){ char buffer[ UNBFF_SIZE ];
        DWORD length = GetCurrentDirectoryA( UNBFF_SIZE, buffer );
        return string_t( buffer, length );
    }
    
    /*─······································································─*/

    uint cpus(){ 
        SYSTEM_INFO sysInfo; GetSystemInfo(&sysInfo);
        return sysInfo.dwNumberOfProcessors;
    }
    
    /*─······································································─*/

    string_t tmp(){ string_t tmp (MAX_PATH);
        GetTempPathA( MAX_PATH, tmp.data() );
        return tmp;
    }
    
    /*─······································································─*/

    int exec( string_t cmd ){ return ::system( cmd.get() ); }

    int call( string_t cmd ){ return ::system( cmd.get() ); }

    /*─······································································─*/

    uint pid(){ return GetCurrentProcessId(); }
    
    /*─······································································─*/

    DWORD error(){ return GetLastError(); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/