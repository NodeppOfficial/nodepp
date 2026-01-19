/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TLS
#define NODEPP_TLS

/*────────────────────────────────────────────────────────────────────────────*/

#include "dns.h"
#include "ssl.h"
#include "ssocket.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class tls_t {
private:

    using NODE_CLB = function_t<void,ssocket_t>;

protected:

    struct NODE {
        char     state= 0;
        agent_t  agent;
        ssl_t    ctx  ;
        NODE_CLB func ;
    };  ptr_t<NODE> obj;

public:

    event_t<ssocket_t> onConnect;
    event_t<ssocket_t> onSocket;
    event_t<>          onClose;
    event_t<except_t>  onError;
    event_t<ssocket_t> onOpen;

    /*─······································································─*/

    tls_t( NODE_CLB _func, ssl_t* crt=nullptr, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr ? agent_t(): *opt; 
           /*------------*/ obj->ctx  =crt==nullptr ? ssl_t()  : *crt; }

   ~tls_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tls_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void     close() const noexcept { if(obj->state<=0){return;} obj->state=-1; onClose.emit(); }
    bool is_closed() const noexcept { return obj == nullptr ? 1: obj->state<=0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
        
        if( obj->state == 1 ){ return; } if( obj->ctx.create_server() == -1 )
          { onError.emit("Error Initializing SSL context"); return; }
        if( dns::lookup(host).empty() ){ onError.emit("dns couldn't get ip"); return; }

        auto self = type::bind( this ); auto clb = [=](){
        auto btch = type::bind<int>( MAX_BATCH );

            ssocket_t sk; self->obj->state = 1;
                      sk.SOCK    = SOCK_STREAM;
                      sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(host), port )<0 ){
                self->onError.emit("Error while creating TLS"); 
                self->close(); sk.free(); return -1; 
            }   sk.set_sockopt( self->obj->agent );

            if( sk.bind()<0 ){
                self->onError.emit("Error while binding TLS"); 
                self->close(); sk.free(); return -1; 
            }

            if( sk.listen()<0 ){ 
                self->onError.emit("Error while listening TLS"); 
                self->close(); sk.free(); return -1; 
            }   cb( sk ); self->onOpen.emit( sk ); 
            
        process::poll( sk, POLL_STATE::READ | POLL_STATE::EDGE, coroutine::add( COROUTINE(){
        int c=-1; coBegin ; while( (*btch) --> 0 ){

            while((c=sk._accept()) == -2 ){ coSet(0); return 0; }
            
            if( c<0 ){ 
                self->onError.emit("Error while accepting TLS"); 
            coEnd; }

            ssocket_t cli( self->obj->ctx, c ); 

            cli.set_sockopt( self->obj->agent );
            self->onSocket.emit(cli); self->obj->func(cli);
            if( cli.is_available() ){ self->onConnect.emit(cli); }

        } *btch = MAX_BATCH; coGoto(0); coFinish })); return -1; }; process::add( clb );

    }

    /*─······································································─*/

    void connect( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {

        if( obj->state == 1 ){ return; } if( obj->ctx.create_client() == -1 )
          { onError.emit("Error Initializing SSL context"); return; }
        if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); return; }

        auto self = type::bind(this); auto clb = [=](){

            ssocket_t sk; self->obj->state = 1;
                      sk.SOCK    = SOCK_STREAM;
                      sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(host), port )<0 ){
                self->onError.emit("Error while creating TLS"); 
                self->close(); sk.free(); return -1; 
            }   sk.set_sockopt( self->obj->agent );

            sk.ssl = new ssl_t( self->obj->ctx, sk.get_fd() );
            sk.ssl->set_hostname( host );

        process::add( coroutine::add( COROUTINE(){
        int c=0; coBegin

            coWait((c=sk._connect()) == -2 ); if( c<=0 ){
                self->onError.emit("Error while connecting TLS");
                self->close(); 
            coEnd; }

            sk.onDrain.once([=](){ self->close(); }); cb(sk);
            self->onSocket.emit(sk); self->obj->func(sk);

            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

        coFinish })); return -1; }; process::add( clb );

    }

    /*─······································································─*/

    void free() const noexcept {
        if( is_closed() ){ return; }close();
        onConnect.clear(); onSocket.clear();
        onError  .clear(); onOpen  .clear();
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace tls {

    inline tls_t server( ssl_t* ssl=nullptr, agent_t* opt=nullptr ){
        auto skt = tls_t( nullptr, ssl, opt ); return skt;
    }

    inline tls_t client( ssl_t* ssl=nullptr, agent_t* opt=nullptr ){
        auto skt = tls_t( nullptr, ssl, opt ); return skt;
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/