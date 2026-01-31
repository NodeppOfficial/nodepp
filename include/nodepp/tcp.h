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
        char state= 0; 
        agent_t agent;
        NODE_CLB func;
    };  ptr_t<NODE> obj;

public:

    event_t<socket_t> onConnect;
    event_t<socket_t> onSocket;
    event_t<>         onClose;
    event_t<except_t> onError;
    event_t<socket_t> onOpen;

    /*─······································································─*/

    tcp_t( NODE_CLB _func, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr ? agent_t() : *opt; }

   ~tcp_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tcp_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void     close() const noexcept { if(obj->state<=0){return;} obj->state=-1; onClose.emit(); }
    bool is_closed() const noexcept { return obj == nullptr ? 1: obj->state<=0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
        if( obj->state == 1 ){ return; } if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); return; }

        socket_t sk; obj->state=1;
        sk.SOCK    = SOCK_STREAM ;
        sk.IPPROTO = IPPROTO_TCP ;

        if( sk.socket( dns::lookup(host), port )<0 ){
            onError.emit("Error while creating TCP"); 
            close(); sk.free(); return; 
        }   sk.set_sockopt( obj->agent );

        if( sk.bind()<0 ){
            onError.emit("Error while binding TCP"); 
            close(); sk.free(); return; 
        }

        if( sk.listen()<0 ){ 
            onError.emit("Error while listening TCP"); 
            close(); sk.free(); return; 
        }   
        
        auto self=type::bind( this );
        cb( sk );  onOpen.emit( sk );
        sk.onDrain.once([=](){ self->close(); });
        
        process::poll( sk, POLL_STATE::READ | POLL_STATE::EDGE, [=](){
        int c=-1; while( self.count() < MAX_BATCH ) {

            while( (c=sk._accept())==-2 ){ return 0; } if(c<0) { 
                self->onError.emit("Error while accepting TCP");
            return -1; }

            auto cli   = socket_t(c);
            cli.set_sockopt( self->obj->agent );
            auto _read = type::bind( generator::file::read() );

        process::poll( cli, POLL_STATE::READ | POLL_STATE::EDGE, [=](){

            if( (*_read)(&cli)==1  ){ return  0; }
            if(!cli.is_available() ){ return -1; }
                
            cli.set_borrow(_read->data); self->onSocket.emit(cli);
            /*------------------------*/ self->obj->func(cli);
            if( cli.is_available() ){ self->onConnect.emit(cli); }

            return -1; }, self->obj->agent.conn_timeout );
        }   return  1; }); 

    }

    /*─······································································─*/

    void connect( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
        if( obj->state == 1 ){ return; } if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); return; }

        socket_t sk; obj->state=1;
        sk.SOCK    = SOCK_STREAM ;
        sk.IPPROTO = IPPROTO_TCP ;

        if( sk.socket( dns::lookup(host), port )<0 ){
            onError.emit("Error while creating TCP"); 
            close(); sk.free(); return; 
        }   
        
        sk.set_sockopt( obj->agent );
        auto self = type::bind( this ); 
        sk.onDrain.once([=](){ self->close(); }); 
        
        process::add([=](){ int c=0;

            while( (c=sk._connect())==-2 ){ return 1; } if(c<=0){
                self->onError.emit("Error while connecting TCP");
            return -1; }

            cb(sk); self->onSocket.emit(sk);
            /*---*/ self->obj->func(sk);

            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

        return -1; });

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

    inline tcp_t server( agent_t* opt=nullptr ){
    auto skt = tcp_t( nullptr, opt ); return skt; }

    inline tcp_t client( agent_t* opt=nullptr ){
    auto skt = tcp_t( nullptr, opt ); return skt; }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/