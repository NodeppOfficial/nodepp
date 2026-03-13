/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_UDP
#define NODEPP_UDP

/*────────────────────────────────────────────────────────────────────────────*/

#include "socket.h"
#include "dns.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class udp_t {
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

    udp_t( NODE_CLB _func, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr ? agent_t() : *opt; }

   ~udp_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    udp_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void     close() const noexcept { if(obj->state<=0){return;} obj->state=-1; onClose.emit(); }
    bool is_closed() const noexcept { return obj == nullptr ? 1: obj->state<=0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
        if( obj->state == 1 ) { return; } if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); close(); return; }

        socket_t sk; obj->state=1;
        sk.SOCK    = SOCK_DGRAM  ;
        sk.IPPROTO = IPPROTO_UDP ;

        if( sk.socket( dns::lookup(host), port )<0 ){
            onError.emit("Error while creating UDP"); 
            close(); sk.free(); return; 
        }   sk.set_sockopt( obj->agent );

        if( sk.bind()<0 ){
            onError.emit("Error while binding UDP"); 
            close(); sk.free(); return; 
        }

        auto self = type::bind( this );
        sk.onDrain.once([=](){ self->close(); });

        process::add([=](){

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

    void connect( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
        if( obj->state == 1 ){ return; } if( dns::lookup(host).empty() )
          { onError.emit("dns couldn't get ip"); return; }

        socket_t sk; obj->state=1;
        sk.SOCK    = SOCK_DGRAM  ;
        sk.IPPROTO = IPPROTO_UDP ;

        if( sk.socket( dns::lookup(host), port )<0 ){
            onError.emit("Error while creating UDP"); 
            close(); sk.free(); return; 
        }   
        
        sk.set_sockopt( obj->agent );
        auto self = type::bind( this );
        sk.onDrain.once([=](){ self->close(); });

        process::add([=](){
        
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

namespace udp {

    inline udp_t server( agent_t* opt=nullptr ){
        auto skt = udp_t( nullptr, opt ); return skt;
    }

    inline udp_t client( agent_t* opt=nullptr ){
        auto skt = udp_t( nullptr, opt ); return skt;
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif