/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WS
#define NODEPP_WS
#define NODEPP_WS_MASK   0x8000
#define NODEPP_WS_SECRET "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/*────────────────────────────────────────────────────────────────────────────*/

#include "http.h"
#include "crypto.h"
#include "generator.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class ws_t : public socket_t {
protected:

    struct NODE {
        generator::ws::read  read ; uchar_32 mask;
        generator::ws::write write;
    };  ptr_t<NODE> ws;

public:

    template< class... T >
    ws_t( const T&... args ) noexcept : socket_t( args... ), ws( new NODE() ){}

    /*─······································································─*/

    virtual int _write( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return  0; }
        while( ws->write( this, bf, sx )==1 )/*--*/{ return -2; }
        return ws->write.data==0 ? -1 : ws->write.data;
    }

    virtual int _read ( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return  0; }
        while( ws->read( this, bf, sx )==1 )/*---*/{ return -2; }
        return ws->read.data==0 ? -1 : ws->read.data;
    }

    /*─······································································─*/

    void set_frame_mask( bool state ) const noexcept { 
        if( state ){ obj->state |= NODEPP_WS_MASK; }
        else /*-*/ { obj->state &=~NODEPP_WS_MASK; }
    }

    uchar_32 get_frame_mask() const noexcept {
        uchar_64 raw = (uchar_64) this; 
        uchar_32 out = raw ^ (raw>>32);
        return (obj->state&NODEPP_WS_MASK) ? out : 0UL;
    } 

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace ws {

    inline tcp_t server( const tcp_t& skt ){ 
    skt.onSocket([=]( ptr_t<tcp_t> self, socket_t raw ){

        http_t hrv = raw;
        ws_t   cli = raw; cli.set_frame_mask( false );

        if( !generator::ws::server( hrv ) )
          { self->onConnect.skip(); return; }

        process::add([=](){ 
            cli.set_timeout(0); cli.resume();
            self->onConnect.resume( );
            self->onConnect.emit(cli);
            stream::pipe /*--*/ (cli);
        return -1; });

    }); return skt; }

    /*─······································································─*/

    inline tcp_t server( agent_t* opt=nullptr ){
    auto skt = http::server( nullptr, opt );
                 ws::server( skt ); return skt;
    }

    /*─······································································─*/

    inline tcp_t client( const string_t& uri, agent_t* opt=nullptr ){
        auto skt = tcp::client( opt ); 
    skt.onSocket.once([=]( ptr_t<tcp_t> self, socket_t raw ){

        http_t hrv = raw;
        ws_t   cli = raw; cli.set_frame_mask( true );

        if( !generator::ws::client( hrv, uri ) )
          { self->onConnect.skip(); return; }

        process::add([=](){ 
            cli.set_timeout(0); cli.resume();
            self->onConnect.resume( );
            self->onConnect.emit(cli);
            stream::pipe /*--*/ (cli);
        return -1; });

    }); skt.connect( url::rawname(uri), url::port(uri) ); return skt; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#undef NODEPP_WS_SECRET
#undef NODEPP_WS_MASK
#endif

/*────────────────────────────────────────────────────────────────────────────*/