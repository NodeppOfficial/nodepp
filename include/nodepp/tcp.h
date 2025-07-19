/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TCP
#define NODEPP_TCP

/*────────────────────────────────────────────────────────────────────────────*/

#include "socket.h"
#include "dns.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class tcp_t {
private:

    using CALLBACK = function_t<void,socket_t>;

protected:

    struct NODE {
        int      state= 0;
        agent_t  agent;
        CALLBACK func ;
    };  ptr_t<NODE> obj;

public: tcp_t() noexcept : obj( new NODE() ) {}

    event_t<socket_t> onConnect;
    event_t<socket_t> onSocket;
    event_t<>         onClose;
    event_t<except_t> onError;
    event_t<socket_t> onOpen;

    /*─······································································─*/

   ~tcp_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tcp_t( CALLBACK _func, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr ? agent_t() : *opt; }

    /*─······································································─*/

    void     close() const noexcept { if(obj->state<=0){return;} obj->state=-1; onClose.emit(); }
    bool is_closed() const noexcept { return obj == nullptr ? 1: obj->state<=0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, CALLBACK cb ) const noexcept {
        if( obj->state == 1 ) { return; } if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); close(); return; }

        auto self = type::bind( this ); auto clb = [=](){

            socket_t sk; self->obj->state = 1;
                     sk.SOCK    = SOCK_STREAM;
                     sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(host), port )<0 ){
                self->onError.emit("Error while creating TCP"); 
                self->close(); sk.free(); return; 
            }   sk.set_sockopt( self->obj->agent );

            if( sk.bind()<0 ){
                self->onError.emit("Error while binding TCP"); 
                self->close(); sk.free(); return; 
            }

            if( sk.listen()<0 ){ 
                self->onError.emit("Error while listening TCP"); 
                self->close(); sk.free(); return; 
            }   cb( sk ); self->onOpen.emit( sk ); 
            
            process::add( coroutine::add( COROUTINE(){
            int c=-1; coBegin; coWait(!limit::fileno_ready());

                while(!self->is_closed() && !sk.is_closed() ){ 
                   if((c=sk._accept())!=-2 ){ break; }
                coNext; }

                if( c<0 ){ 
                    self->onError.emit("Error while accepting TCP"); 
                coEnd; }
                
            process::add( coroutine::add( COROUTINE(){
                socket_t cli(c); 
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
        if( obj->state == 1 ){ return; } if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); close(); return; }

        auto self = type::bind(this); auto clb = [=](){

            socket_t sk; self->obj->state = 1;
                     sk.SOCK    = SOCK_STREAM;
                     sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(host), port )<0 ){
                self->onError.emit("Error while creating TCP"); 
                self->close(); sk.free(); return; 
            }   sk.set_sockopt( self->obj->agent );

            process::add( coroutine::add( COROUTINE(){
            int c=0; coBegin; coWait(!limit::fileno_ready());

                coWait( (c=sk._connect())==-2 ); if( c<=0 ){
                    self->onError.emit("Error while connecting TCP");
                    self->close(); coEnd;
                }
                
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
         connect( host, port, []( socket_t ){} );
    }

    void listen( const string_t& host, int port ) const noexcept {
         listen( host, port, []( socket_t ){} );
    }

    /*─······································································─*/

    void free() const noexcept {
        if( is_closed() ){ return; }close();
        onConnect.clear(); onSocket.clear();
        onError  .clear(); onOpen  .clear();
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace tcp {

    tcp_t server( agent_t* opt=nullptr ){
        auto skt = tcp_t([=]( socket_t ){}, opt ); return skt;
    }

    tcp_t client( agent_t* opt=nullptr ){
        auto skt = tcp_t([=]( socket_t ){}, opt ); return skt;
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif