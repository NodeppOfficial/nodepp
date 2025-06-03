/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TLS
#define NODEPP_TLS

/*────────────────────────────────────────────────────────────────────────────*/

#include "ssocket.h"
#include "poll.h"
#include "dns.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class tls_t {
protected:

    struct NODE {
        int                        state = 0;
        agent_t                    agent;
        ssl_t                      ctx  ;
        poll_t                     poll ;
        function_t<void,ssocket_t> func ;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    template< class T > void add_socket( T& cli ) const noexcept {
        auto self = type::bind( this ); process::poll::add([=](){
             self->onSocket.emit(cli); self->obj->func(cli); return -1;
        });
    }

    int next() const noexcept {
          if( obj->poll.emit()==-1 ){ return -1; } auto x = obj->poll.get_last_poll();
          if( x[0] >= 0 )
            { ssocket_t cli( obj->ctx, x[1] ); cli.set_sockopt( obj->agent ); add_socket(cli); }
        else{ socket_t cli( x[1] ); cli.free(); } return 1;
    }

public: tls_t() noexcept : obj( new NODE() ) {}

    event_t<ssocket_t> onConnect;
    event_t<ssocket_t> onSocket;
    event_t<>          onClose;
    event_t<except_t>  onError;
    event_t<ssocket_t> onOpen;

    /*─······································································─*/

    tls_t( decltype(NODE::func) _func, const ssl_t* xtc, agent_t* opt=nullptr ) noexcept : obj( new NODE() ){
    if( xtc == nullptr ) process::error("Invalid SSL Contenx");
        obj->agent = opt==nullptr ? agent_t():*opt;
        obj->ctx   = xtc==nullptr ? ssl_t():  *xtc;
        obj->func  = _func;
    }

    /*─······································································─*/

    void     close() const noexcept { if( obj->state<=0 ){ return; } obj->state=-1; onClose.emit(); }
    bool is_closed() const noexcept { return obj == nullptr ? 1 : obj->state <= 0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, decltype(NODE::func) cb ) const noexcept {
        if( obj->state == 1 ){ return; } if( obj->ctx.create_server() == -1 )
          { _EERROR(onError,"Error Initializing SSL context"); close(); return; }
        if( dns::lookup(host).empty() ){ _EERROR(onError,"dns couldn't get ip"); close(); return; }

        auto self = type::bind( this ); obj->state = 1;

        ssocket_t sk;
                  sk.SOCK    = SOCK_STREAM;
                  sk.IPPROTO = IPPROTO_TCP;
                  sk.socket( dns::lookup(host), port );
                  sk.set_sockopt( self->obj->agent );

        if( sk.bind()   < 0 ){ _EERROR(onError,"Error while binding TLS");   close(); sk.free(); return; }
        if( sk.listen() < 0 ){ _EERROR(onError,"Error while listening TLS"); close(); sk.free(); return; }

        cb( sk ); onOpen.emit( sk ); process::poll::add([=](){
            static int _accept = -2;
        coStart

            while( _accept == -2 ){
               if( self->is_closed() || sk.is_closed() ) { coGoto(2); }
                   _accept = sk._accept(); if( _accept!=-2 ) { break; }
            while( self->next()==1 ){ coTry(3); coYield(3); } coNext; }

            if( _accept < 0 ){ _EERROR(self->onError,"Error while accepting TLS"); coGoto(2); }
            do{ if( self->obj->poll.push_read(_accept)==0 )
              { socket_t cli( _accept ); cli.free(); }
              } while(0); _accept=-2; coTry(0);

            coYield(2); self->close(); sk.free();

        coStop
        });

    }

    void listen( const string_t& host, int port ) const noexcept {
         listen( host, port, []( ssocket_t ){} );
    }

    /*─······································································─*/

    void connect( const string_t& host, int port, decltype(NODE::func) cb  ) const noexcept {
        if( obj->state == 1 ){ return; } if( obj->ctx.create_client() == -1 )
          { _EERROR(onError,"Error Initializing SSL context"); close(); return; }
        if( dns::lookup(host).empty() )
          { _EERROR(onError,"dns couldn't get ip"); close(); return; }

        auto self = type::bind(this); obj->state = 1;

        ssocket_t sk;
                  sk.SOCK    = SOCK_STREAM;
                  sk.IPPROTO = IPPROTO_TCP;
                  sk.socket( dns::lookup(host), port );
                  sk.set_sockopt( self->obj->agent );

        sk.ssl = new ssl_t( obj->ctx, sk.get_fd() );
        sk.ssl->set_hostname( host );

        process::poll::add([=](){
            if( self->is_closed() || sk.is_closed() ){ return -1; }
        coStart

            while( sk._connect() == -2 ){ coNext; }
            if   ( sk._connect()  <  0 ){
                _EERROR(self->onError,"Error while connecting TLS");
            coEnd; }

            if( self->obj->poll.push_write(sk.get_fd())==0 )
              { sk.free(); } while( self->obj->poll.emit()==0 ){
                   if( process::now() > sk.get_send_timeout() )
                     { coEnd; } coNext; }

            while( sk.ssl->_connect() == -2 ){ coNext; }
            if   ( sk.ssl->_connect() <=  0 ){
                _EERROR(self->onError,"Error while handshaking TLS");
            coEnd; } cb( sk );

            sk.onClose.once([=](){ self->close(); });
            self->onSocket.emit(sk); sk.onOpen.emit();
            self->onOpen.emit(sk); self->obj->func(sk);

        coStop
        });

    }

    void connect( const string_t& host, int port ) const noexcept {
         connect( host, port, []( ssocket_t ){} );
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace tls {

    tls_t server( const tls_t& server ){ server.onSocket([=]( ssocket_t cli ){
        cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); });
        ptr_t<_file_::read> _read = new _file_::read;

        server.onConnect.once([=]( ssocket_t cli ){ process::poll::add([=](){
            if(!cli.is_available() )    { cli.close(); return -1; }
            if((*_read)(&cli)==1 )      { return 1; }
            if(  _read->state<=0 )      { return 1; }
            cli.onData.emit(_read->data); return 1;
        }) ; });

        process::task::add([=](){
            server.onConnect.emit(cli); return -1;
        });

    }); return server; }

    /*─······································································─*/

    tls_t server( const ssl_t* ctx, agent_t* opt=nullptr ){
        auto server = tls_t( [=]( ssocket_t /*unused*/ ){}, ctx, opt );
        tls::server( server ); return server;
    }

    /*─······································································─*/

    tls_t client( const tls_t& client ){ client.onOpen.once([=]( ssocket_t cli ){
        cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); });
        ptr_t<_file_::read> _read = new _file_::read;

        process::poll::add([=](){
            if(!cli.is_available() )    { cli.close(); return -1; }
            if((*_read)(&cli)==1 )      { return 1; }
            if(  _read->state<=0 )      { return 1; }
            cli.onData.emit(_read->data); return 1;
        });

    }); return client; }

    /*─······································································─*/

    tls_t client( const ssl_t* ctx, agent_t* opt=nullptr ){
        auto client = tls_t( [=]( ssocket_t /*unused*/ ){}, ctx, opt );
        tls::client( client ); return client;
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif