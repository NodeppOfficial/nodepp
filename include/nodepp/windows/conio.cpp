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

namespace nodepp { namespace conio { enum color {
    black = 0x00, white  = 0x01,
    green = 0x02, red    = 0x03,
    blue  = 0x04, cyan   = 0x05,
    yellow= 0x06, magenta= 0x07,
    bold  = 0x10  /*----------*/
};}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace conio { WORD attr = 0, dflt = 7;

    /*─······································································─*/

    int perr( const string_t& args ){ DWORD len=0;
        auto fd = GetStdHandle( STD_ERROR_HANDLE );
        WriteFile( fd, args.get(), args.size(), &len, NULL ); return len;
    }
    
    int pout( const string_t& args ){ DWORD len=0;
        auto fd = GetStdHandle( STD_OUTPUT_HANDLE );
        WriteFile( fd, args.get(), args.size(), &len, NULL ); return len;
    }

    template< class V, class... T >
    int scan( const V& argc, const T&... args ){ DWORD len=0;
        auto bff = string::buffer( UNBFF_SIZE );
        auto fd  = GetStdHandle( STD_INPUT_HANDLE );
        ReadFile( fd, bff.get(), bff.size(), &len, NULL ); 
        auto data= string_t( &bff, len );
        return string::parse( data.get(), argc, args... );
    }

    /*─······································································─*/

    template< class... T >
    int log( const T&... args ){ if( attr != 0 ){
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), attr ); }

        auto data = string::join( " ", args... );
        pout( data );
        
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), dflt ); 
        attr = 0; return data.size();
    }

    template< class... T >
    int err( const T&... args ){ if( attr != 0 ){
        SetConsoleTextAttribute( GetStdHandle( STD_ERROR_HANDLE ), attr ); }

        auto data = string::join( " ", args... );
        perr( data );
        
        SetConsoleTextAttribute( GetStdHandle( STD_ERROR_HANDLE ), dflt ); 
        attr = 0; return data.size();
    }
    
    /*─······································································─*/

    int set_position( int x, int y ){ COORD coord; coord.X = x; coord.Y = y;
        return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    /*─······································································─*/

    int inverse(){ attr |= COMMON_LVB_REVERSE_VIDEO; return 1; }
    int underscore(){ attr |= COMMON_LVB_UNDERSCORE; return 1; }
    int gotoxy( int x, int y ){ return set_position( x, y ); }
    int clear(){ return system("cls"); }
    int reset(){ attr = 0; return 1; }
    
    /*─······································································─*/

    int background( int state ){
        if( state & 0x10 ){ attr |= BACKGROUND_INTENSITY; state &= 0x0f; }
        switch( state )   {
            case color::black:   attr |= 0; return 1; break;
            case color::white:   attr |= BACKGROUND_BLUE | BACKGROUND_GREEN| BACKGROUND_RED; return 1; break;
            case color::cyan:    attr |= BACKGROUND_GREEN| BACKGROUND_BLUE; return 1; break;
            case color::yellow:  attr |= BACKGROUND_GREEN| BACKGROUND_RED;  break;
            case color::magenta: attr |= BACKGROUND_BLUE | BACKGROUND_RED;  break;
            case color::green:   attr |= BACKGROUND_GREEN; return 1; break;
            case color::red:     attr |= BACKGROUND_RED;   return 1; break;
            case color::blue:    attr |= BACKGROUND_BLUE;  return 1; break;
        }   return -1;
    }

    int foreground( int state ){
        if( state & 0x10 ){ attr |= FOREGROUND_INTENSITY; state &= 0x0f; }
        switch( state )   {
            case color::black:   attr |= 0; return 1; break;
            case color::white:   attr |= FOREGROUND_BLUE | FOREGROUND_GREEN| FOREGROUND_RED; return 1; break;
            case color::cyan:    attr |= FOREGROUND_GREEN| FOREGROUND_BLUE; return 1; break;
            case color::yellow:  attr |= FOREGROUND_GREEN| FOREGROUND_RED;  break;
            case color::magenta: attr |= FOREGROUND_BLUE | FOREGROUND_RED;  break;
            case color::green:   attr |= FOREGROUND_GREEN; return 1; break;
            case color::red:     attr |= FOREGROUND_RED;   return 1; break;
            case color::blue:    attr |= FOREGROUND_BLUE;  return 1; break;
        }   return -1;
    }
    
    /*─······································································─*/

    int error( string_t msg ){ foreground( color::red    | color::bold ); return log( msg ); }
    int  info( string_t msg ){ foreground( color::cyan   | color::bold ); return log( msg ); }
    int  done( string_t msg ){ foreground( color::green  | color::bold ); return log( msg ); }
    int  warn( string_t msg ){ foreground( color::yellow | color::bold ); return log( msg ); }

}}