/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WSS
#define NODEPP_WSS
#define SECRET "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/*────────────────────────────────────────────────────────────────────────────*/

#include "https.h"
#include "crypto.h"
#include "generator.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class wss_t : public ssocket_t {
protected:

    struct NODE {
        _ws_::write write;
        _ws_::read  read ;
    };  ptr_t<NODE> ws;

public:

    template< class... T > 
    wss_t( const T&... args ) noexcept : ssocket_t( args... ), ws( new NODE() ){}

    virtual int _write( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; }
        while( ws->write( this, bf, sx )==1 ){ return -2; }
        return ws->write.data==0 ? -2 : ws->write.data;
    }

    virtual int _read( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; }
        while( ws->read( this, bf, sx )==1 ){ return -2; }
        return ws->read.data==0 ? -2 : ws->read.data;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace wss {

    tls_t server( const tls_t& srv ){ srv.onSocket([=]( ssocket_t cli ){
        auto hrv = type::cast<https_t>(cli);
        if ( !_ws_::server( hrv ) ){ return; }

        cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); }); 
        ptr_t<_file_::read> _read = new _file_::read;
        cli.set_timeout(0);

        srv.onConnect.once([=]( wss_t ctx ){ process::poll::add([=](){ 
            if(!cli.is_available() )    { cli.close(); return -1; }
            if((*_read)(&ctx)==1 )      { return 1; }
            if(  _read->state<=0 )      { return 1; }
            ctx.onData.emit(_read->data); return 1;
        }); });

        process::task::add([=](){
            cli.resume(); srv.onConnect.emit(cli); return -1;
        });

    }); return srv; }

    /*─······································································─*/

    tls_t server( const ssl_t* ssl, agent_t* opt=nullptr ){
        auto server = https::server( [=]( https_t /*unused*/ ){}, ssl, opt );
                        wss::server( server ); return server;     
    }

    /*─······································································─*/

    tls_t client( const string_t& uri, const ssl_t* ssl, agent_t* opt=nullptr ){
    tls_t srv ( [=]( ssocket_t /*unused*/ ){}, ssl, opt ); 
        srv.connect( url::hostname(uri), url::port(uri) );
        srv.onSocket.once([=]( ssocket_t cli ){
            auto hrv = type::cast<https_t>(cli);
            if ( !_ws_::client( hrv, uri ) ){ return; }
            
            cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); });
            ptr_t<_file_::read> _read = new _file_::read;
            cli.set_timeout(0);

            srv.onConnect.once([=]( wss_t ctx ){ process::poll::add([=](){
                if(!cli.is_available() )    { cli.close(); return -1; }
                if((*_read)(&ctx)==1 )      { return 1; }
                if(  _read->state<=0 )      { return 1; }
                ctx.onData.emit(_read->data); return 1;
            }); });

            process::task::add([=](){
                cli.resume(); srv.onConnect.emit(cli); return -1;
            });
            
        });
    
    return srv; }

}}

#undef SECRET
#endif