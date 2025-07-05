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

#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

/*────────────────────────────────────────────────────────────────────────────*/

#define C_BLACK   0x00
#define C_WHITE   0x01
#define C_GREEN   0x02
#define C_RED     0x03
#define C_BLUE    0x04
#define C_CYAN    0x05
#define C_YELLOW  0x06
#define C_MAGENTA 0x07
#define C_BOLD    0x10

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace conio {

    /*─······································································─*/

    int perr( const string_t& args ){ 
        return ::write( STDERR_FILENO, args.get(), args.size() );
    }
    
    int pout( const string_t& args ){
        return ::write( STDOUT_FILENO, args.get(), args.size() );
    }

    template< class V, class... T >
    int scan( const V& argc, const T&... args ){ 
        auto bff = string::buffer( UNBFF_SIZE );
        auto len = ::read( STDIN_FILENO, bff.get(), bff.size() );
        auto data= string_t( &bff, len );
        return string::parse( data.get(), argc, args... );
    }

    /*─······································································─*/

    template< class... T >
    int log( const T&... args ){
        int last = sizeof...( args ), size = 0;
        string::map([&]( string_t arg ){ 
            size += pout( arg + ( --last<1 ? "" : " " ) ); 
        },  args... ); 
            size += pout("\033[0m"); 
        return size;
    }

    template< class... T >
    int err( const T&... args ){
        int last = sizeof...( args ), size = 0;
        string::map([&]( string_t arg ){ 
            size += perr( arg + ( --last<1 ? "" : " " ) ); 
        },  args... ); 
            size += perr("\033[0m"); 
        return size;
    }

    /*─······································································─*/
    
    int set_position( int x, int y ){ return pout(string::format("\033[%d;%dH",x,y)); }

    /*─······································································─*/

    int gotoxy( int x, int y ){ return set_position( x, y ); }
    int undescore(){ return pout("\033[4m"); }
    int inverse(){ return pout("\033[7m"); }
    int reset(){ return pout("\033[0m"); }
    int clear(){ return pout("\033c\n"); }

    /*─······································································─*/

    int background( int color ){ 
        if( color & 0x10 ){ pout("\033[1m"); color &= 0x0f; }
        switch( color ) {
            case C_BLACK:   return pout("\033[40m"); break;
            case C_WHITE:   return pout("\033[47m"); break;
            case C_GREEN:   return pout("\033[42m"); break;
            case C_RED:     return pout("\033[41m"); break;
            case C_BLUE:    return pout("\033[44m"); break;
            case C_CYAN:    return pout("\033[46m"); break;
            case C_YELLOW:  return pout("\033[43m"); break;
            case C_MAGENTA: return pout("\033[45m"); break;
        }   return -1;
    }

    int foreground( int color ){
        if( color & 0x10 ){ pout("\033[1m"); color &= 0x0f; }
        switch( color ) {
            case C_BLACK:   return pout("\033[30m"); break;
            case C_WHITE:   return pout("\033[37m"); break;
            case C_GREEN:   return pout("\033[32m"); break;
            case C_RED:     return pout("\033[31m"); break;
            case C_BLUE:    return pout("\033[34m"); break;
            case C_CYAN:    return pout("\033[36m"); break;
            case C_YELLOW:  return pout("\033[33m"); break;
            case C_MAGENTA: return pout("\033[35m"); break;
        }   return -1;
    }

    /*─······································································─*/

    int error( const char* msg ){ foreground( C_RED    | C_BOLD ); return log( msg ); }
    int  info( const char* msg ){ foreground( C_CYAN   | C_BOLD ); return log( msg ); }
    int  done( const char* msg ){ foreground( C_GREEN  | C_BOLD ); return log( msg ); }
    int  warn( const char* msg ){ foreground( C_YELLOW | C_BOLD ); return log( msg ); }

}}