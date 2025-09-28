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
#define NODEPP_WS_SECRET "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/*────────────────────────────────────────────────────────────────────────────*/

#include "http.h"
#include "crypto.h"
#include "generator.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class ws_t : public socket_t {
protected:

    struct NODE {
        generator::ws::write write;
        generator::ws::read  read ;
    };  ptr_t<NODE> ws;

public:

    virtual ~ws_t() noexcept {}

    template< class... T >
    ws_t( const T&... args ) noexcept : socket_t( args... ), ws( new NODE() ){}

    virtual int _write( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; }
        while( ws->write( this, bf, sx )==1 ){ return -2; }
        return ws->write.data==0 ? -1 : ws->write.data;
    }

    virtual int _read ( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; }
        while( ws->read( this, bf, sx )==1 ){ return -2; }
        return ws->read.data==0 ? -1 : ws->read.data;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace ws {

    tcp_t server( const tcp_t& skt ){ skt.onSocket([=]( socket_t cli ){

        auto hrv = type::cast<http_t>(cli);
        if( !generator::ws::server( hrv ) )
          { skt.onConnect.skip(); return; }   

        process::foop([=](){ 
            skt.onConnect.resume( );
            skt.onConnect.emit(cli); 
            return -1;
        });

    }); skt.onConnect([=]( ws_t cli ){
        cli.onDrain.once([=](){ cli.free(); });
        cli.set_timeout(0); cli.resume(); stream::pipe(cli); 
    }); return skt; }

    /*─······································································─*/

    tcp_t server( agent_t* opt=nullptr ){
    auto skt = http::server( [=]( http_t ){}, opt );
                 ws::server( skt ); return skt;
    }

    /*─······································································─*/

    tcp_t client( const string_t& uri, agent_t* opt=nullptr ){
    tcp_t skt   ( [=]( socket_t ){}, opt );
    skt.onSocket.once([=]( socket_t cli ){

        auto hrv = type::cast<http_t> (cli);
        if( !generator::ws::client( hrv, uri ) )
          { skt.onConnect.skip(); return; }   

        process::foop([=](){ 
            skt.onConnect.resume( );
            skt.onConnect.emit(cli); 
            return -1;
        });

    }); skt.onConnect.once([=]( ws_t cli ){
        cli.onDrain.once([=](){ cli.free(); });
        cli.set_timeout(0); cli.resume(); stream::pipe(cli); 
    }); skt.connect( url::hostname(uri), url::port(uri) ); return skt; }

}}

#undef NODEPP_WS_SECRET
#endif