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

namespace nodepp { namespace conio { enum color {
    black = 0x00, white  = 0x01,
    green = 0x02, red    = 0x03,
    blue  = 0x04, cyan   = 0x05,
    yellow= 0x06, magenta= 0x07,
    bold  = 0x10  /*----------*/
};}}

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
        auto data = string::join( " ", args... ) + "\033[0m";
        pout( data ); return data.size();
    }

    template< class... T >
    int err( const T&... args ){
        auto data = string::join( " ", args... ) + "\033[0m";
        perr( data ); return data.size();
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

    int background( int state ){ 
        if( state & 0x10 ){ pout("\033[1m"); state &= 0x0f; }
        switch( state )   {
            case color::black:   return pout("\033[40m"); break;
            case color::white:   return pout("\033[47m"); break;
            case color::green:   return pout("\033[42m"); break;
            case color::red:     return pout("\033[41m"); break;
            case color::blue:    return pout("\033[44m"); break;
            case color::cyan:    return pout("\033[46m"); break;
            case color::yellow:  return pout("\033[43m"); break;
            case color::magenta: return pout("\033[45m"); break;
        }   return -1;
    }

    int foreground( int state ){
        if( state & 0x10 ){ pout("\033[1m"); state &= 0x0f; }
        switch( state )   {
            case color::black:   return pout("\033[30m"); break;
            case color::white:   return pout("\033[37m"); break;
            case color::green:   return pout("\033[32m"); break;
            case color::red:     return pout("\033[31m"); break;
            case color::blue:    return pout("\033[34m"); break;
            case color::cyan:    return pout("\033[36m"); break;
            case color::yellow:  return pout("\033[33m"); break;
            case color::magenta: return pout("\033[35m"); break;
        }   return -1;
    }

    /*─······································································─*/

    int error( string_t msg ){ foreground( color::red    | color::bold ); return log( msg ); }
    int  info( string_t msg ){ foreground( color::cyan   | color::bold ); return log( msg ); }
    int  done( string_t msg ){ foreground( color::green  | color::bold ); return log( msg ); }
    int  warn( string_t msg ){ foreground( color::yellow | color::bold ); return log( msg ); }

}}