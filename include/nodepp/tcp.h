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

    using NODE_CLB = function_t<void,socket_t>;

protected:

    struct NODE {
        char     state= 0;
        agent_t  agent;
        NODE_CLB func ;
    };  ptr_t<NODE> obj;

public:

    event_t<socket_t> onConnect;
    event_t<socket_t> onSocket;
    event_t<>         onClose;
    event_t<except_t> onError;
    event_t<socket_t> onOpen;

    /*─······································································─*/

    virtual ~tcp_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tcp_t( NODE_CLB _func, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr ? agent_t() : *opt; }

    tcp_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void     close() const noexcept { if(obj->state<=0){return;} obj->state=-1; onClose.emit(); }
    bool is_closed() const noexcept { return obj == nullptr ? 1: obj->state<=0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, NODE_CLB cb ) const noexcept {
        if( obj->state == 1 ){ return; } if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); return; }

        auto self = type::bind( this ); auto clb = [=](){

            socket_t sk; self->obj->state = 1;
                     sk.SOCK    = SOCK_STREAM;
                     sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(host), port )<0 ){
                self->onError.emit("Error while creating TCP"); 
                self->close(); sk.free(); return -1; 
            }   sk.set_sockopt( self->obj->agent );

            if( sk.bind()<0 ){
                self->onError.emit("Error while binding TCP"); 
                self->close(); sk.free(); return -1; 
            }

            if( sk.listen()<0 ){ 
                self->onError.emit("Error while listening TCP"); 
                self->close(); sk.free(); return -1; 
            }   cb( sk ); self->onOpen.emit( sk ); 
            
        process::poll( sk, POLL_STATE::READ, coroutine::add( COROUTINE(){
        int c=-1; coBegin

            coWait((c=sk._accept()) == -2 ); if( c<0 ){ 
                self->onError.emit("Error while accepting TCP"); 
            coEnd; }
            
            socket_t cli(c); 

            cli.set_sockopt( self->obj->agent );
            self->onSocket.emit(cli); self->obj->func(cli);
            if( cli.is_available() ){ self->onConnect.emit(cli); }

        coStay(0); coFinish })); return -1; }; process::foop( clb );

    }

    /*─······································································─*/

    void connect( const string_t& host, int port, NODE_CLB cb ) const noexcept {
        if( obj->state == 1 ){ return; } if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); return; }

        auto self = type::bind(this); auto clb = [=](){

            socket_t sk; self->obj->state = 1;
                     sk.SOCK    = SOCK_STREAM;
                     sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(host), port )<0 ){
                self->onError.emit("Error while creating TCP"); 
                self->close(); sk.free(); return -1; 
            }   sk.set_sockopt( self->obj->agent );

        process::foop( coroutine::add( COROUTINE(){
        int c=0; coBegin

            coWait((c=sk._connect()) == -2 ); if( c<=0 ){
                self->onError.emit("Error while connecting TCP");
            coEnd; }

            sk.onDrain.once([=](){ self->close(); }); cb(sk);
            self->onSocket.emit(sk); self->obj->func(sk);

            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

        coFinish })); return -1; }; process::foop( clb );

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