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

#include "ssocket.h"
#include "dns.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class tls_t {
private:

    using CALLBACK = function_t<void,ssocket_t>;

protected:

    struct NODE {
        int      state= 0;
        agent_t  agent;
        ssl_t    ctx  ;
        CALLBACK func ;
    };  ptr_t<NODE> obj;

public:

    event_t<ssocket_t> onConnect;
    event_t<ssocket_t> onSocket;
    event_t<>          onClose;
    event_t<except_t>  onError;
    event_t<ssocket_t> onOpen;

    /*─······································································─*/

    tls_t( CALLBACK _func, const ssl_t& ssl, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr ? agent_t() : *opt; obj->ctx = ssl; }

   ~tls_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tls_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void     close() const noexcept { if(obj->state<=0){return;} obj->state=-1; onClose.emit(); }
    bool is_closed() const noexcept { return obj == nullptr ? 1: obj->state<=0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, CALLBACK cb ) const noexcept {
        if( obj->state == 1 ){ return; } if( obj->ctx.create_server() == -1 )
          { onError.emit("Error Initializing SSL context"); close(); return; }
        if( dns::lookup(host).empty() ){ onError.emit("dns couldn't get ip"); close(); return; }

        auto self = type::bind( this ); auto clb = [=](){

            ssocket_t sk; self->obj->state = 1;
                      sk.SOCK    = SOCK_STREAM;
                      sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(host), port )<0 ){
                self->onError.emit("Error while creating TLS"); 
                self->close(); sk.free(); return; 
            }   sk.set_sockopt( self->obj->agent );

            if( sk.bind()<0 ){
                self->onError.emit("Error while binding TLS"); 
                self->close(); sk.free(); return; 
            }

            if( sk.listen()<0 ){ 
                self->onError.emit("Error while listening TLS"); 
                self->close(); sk.free(); return; 
            }   cb( sk ); self->onOpen.emit( sk ); 
            
            process::add( coroutine::add( COROUTINE(){
            int c=-1; coBegin; coWait(!limit::fileno_ready());

                while(!self->is_closed() && !sk.is_closed() ){
                   if((c=sk._accept())!=-2 ){ break; }
                coNext; }

                if( c<0 ){ 
                    self->onError.emit("Error while accepting TLS");
                coEnd; }
                
            process::add( coroutine::add( COROUTINE(){
                ssocket_t cli(self->obj->ctx,c); 
                cli.set_sockopt(self->obj->agent);
                self->onSocket.emit(cli); self->obj->func (cli);
            if( cli.is_available() ){ self->onConnect.emit(cli); }
            return -1; }));

            coStay(0); coFinish
            }));

        };

        process::add( coroutine::add( COROUTINE(){
        coBegin; coWait( !limit::fileno_ready() );
                 clb();
        coFinish
        }));

    }

    /*─······································································─*/

    void connect( const string_t& host, int port, CALLBACK cb ) const noexcept {
        if( obj->state == 1 ){ return; } if( obj->ctx.create_client() == -1 )
          { onError.emit("Error Initializing SSL context"); close(); return; }
        if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); close(); return; }

        auto self = type::bind(this); auto clb = [=](){

            ssocket_t sk; self->obj->state = 1;
                      sk.SOCK    = SOCK_STREAM;
                      sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(host), port )<0 ){
                self->onError.emit("Error while creating TLS"); 
                self->close(); sk.free(); return; 
            }   sk.set_sockopt( self->obj->agent );

            sk.ssl = new ssl_t( self->obj->ctx, sk.get_fd() );
            sk.ssl->set_hostname( host );

            process::add( coroutine::add( COROUTINE(){
            int c=0; coBegin; coWait(!limit::fileno_ready());

                coWait( (c=sk._connect())==-2 ); if( c<=0 ){
                    self->onError.emit("Error while connecting TLS");
                    self->close(); coEnd;
                }

                coWait( (c=sk.ssl->_connect())==-2 ); if( c<=0 ){
                    self->onError.emit("Error while handshaking TLS");
                coEnd; } 
                
            process::add( coroutine::add( COROUTINE(){
                
                sk.onDrain.once([=](){ self->close(); }); cb(sk);
                self->onSocket.emit(sk); self->obj->func(sk);

                if( sk.is_available() ){ 
                    sk.onOpen      .emit(  );
                    self->onOpen   .emit(sk); 
                    self->onConnect.emit(sk); 
                }

            return -1; }));

            coFinish
            }));

        };

        process::add( coroutine::add( COROUTINE(){
        coBegin; coWait( !limit::fileno_ready() );
                 clb();
        coFinish
        }));

    }

    /*─······································································─*/

    void connect( const string_t& host, int port ) const noexcept {
         connect( host, port, []( ssocket_t ){} );
    }

    void listen( const string_t& host, int port ) const noexcept {
         listen( host, port, []( ssocket_t ){} );
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

    tls_t server( const ssl_t& ssl, agent_t* opt=nullptr ){
        auto skt = tls_t( [=]( ssocket_t ){}, ssl, opt ); return skt;
    }

    tls_t client( const ssl_t& ssl, agent_t* opt=nullptr ){
        auto skt = tls_t( [=]( ssocket_t ){}, ssl, opt ); return skt;
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif