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
    enum STATE {
         UDP_STATE_UNKNOWN   = 0b00000000,
         UDP_STATE_USED      = 0b00000001,
         UDP_STATE_CLOSED    = 0b00000010
    };

protected:

    struct NODE {
        int  state= 0; 
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

    bool is_closed() const noexcept { return obj->state & STATE::UDP_STATE_CLOSED; }
    void     close() const noexcept { free(); }

    /*─······································································─*/

    void listen( const dns_t& addr, int port, NODE_CLB cb=nullptr ) const noexcept {

        if( obj->state & STATE::UDP_STATE_CLOSED )
          { onError.emit("udp listener is closed"); return; } 
        if( obj->state & STATE::UDP_STATE_USED )
          { onError.emit("udp listener is used");   return; } 

        socket_t sk; obj->state = STATE::UDP_STATE_USED;
        sk.AF      = addr.family;
        sk.SOCK    = SOCK_DGRAM ;
        sk.IPPROTO = IPPROTO_UDP;

        if( sk.socket( addr.address, port )==-1 ){
            onError.emit( "Error while creating UDP" ); return; 
        }   sk.set_sockopt( obj->agent );

        if( sk.bind() == -1 ){
            onError.emit( "Error while binding UDP" ); return; 
        }

        auto self = type::bind(this); process::add([=](){

            cb(sk); self->onSocket.emit(sk);
            /*---*/ self->obj->func(sk);
                
            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

        return -1; });

    }

    void listen( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
    auto addr = dns::lookup( host, obj->agent.socket_family );
         if( addr.empty() ){ onError.emit( "dns address not found" ); return; }
         listen( addr[0], port, cb );
    }

    /*─······································································─*/

    void connect( const dns_t& addr, int port, NODE_CLB cb=nullptr ) const noexcept {

        if( obj->state & STATE::UDP_STATE_CLOSED )
          { onError.emit( "udp listener is closed" ); return; } 
        if( obj->state & STATE::UDP_STATE_USED )
          { onError.emit( "udp listener is used" );   return; } 

        socket_t sk; obj->state = STATE::UDP_STATE_USED;
        sk.AF      = addr.family;
        sk.SOCK    = SOCK_DGRAM ;
        sk.IPPROTO = IPPROTO_UDP;

        if( sk.socket( addr.address, port )==-1 ){
            onError.emit( "Error while creating UDP" ); return; 
        }   sk.set_sockopt( obj->agent );
        
        auto self = type::bind(this); process::add([=](){
        
            cb(sk); self->onSocket.emit(sk);
            /*---*/ self->obj->func(sk);

            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

        return -1; });

    }

    void connect( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
    auto addr = dns::lookup( host, obj->agent.socket_family );
         if( addr.empty() ){ onError.emit( "dns address not found" ); return; }
         connect( addr[0], port, cb );
    }

    /*─······································································─*/

    void free() const noexcept {
        if( is_closed() ){ return; }
        obj->state = STATE::UDP_STATE_CLOSED; 
        onConnect.clear(); onSocket.clear();
        onError  .clear(); onOpen  .clear();
        onClose  .emit ();
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

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/