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

#include "expected.h"
#include "socket.h"
#include "dns.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class udp_t {
private:

    using NODE_CLB = function_t<void,socket_t>;
    enum STATE {
         UDP_STATE_UNKNOWN   = 0b00000000,
         UDP_STATE_USED      = 0b00000001,
         UDP_STATE_CLOSED    = 0b00000010
    };

protected:

    struct NODE {
        int state = 0; 
        agent_t agent;
        NODE_CLB func;
    };  ptr_t<NODE> obj;

public:

    event_t<ptr_t<udp_t>,socket_t> onSocket ;
    event_t<socket_t>              onConnect;
    event_t<>                      onClose  ;
    event_t<except_t>              onError  ;
    event_t<socket_t>              onOpen   ;

    /*─······································································─*/

    udp_t( NODE_CLB _func, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr ? agent_t() : *opt; }

   ~udp_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    udp_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    bool is_closed() const noexcept { return obj->state & STATE::UDP_STATE_CLOSED; }
    void     close() const noexcept { free(); }

    /*─······································································─*/

    expected_t<udp_t,except_t>
    listen( const dns_t& addr, int port, NODE_CLB clb=nullptr ) const noexcept {

        if( obj->state & STATE::UDP_STATE_CLOSED )
          { except_t err = "udp listener is closed"; onError.emit(err); return err; } 
        if( obj->state & STATE::UDP_STATE_USED )
          { except_t err = "udp listener is used"  ; onError.emit(err); return err; } 

        socket_t sk; obj->state = STATE::UDP_STATE_USED;
        sk.AF      = addr.family;
        sk.SOCK    = SOCK_DGRAM ;
        sk.IPPROTO = IPPROTO_UDP;

        if( sk.socket( addr.address, port )==-1 ){
            except_t err = "Error while creating UDP";
            onError.emit(err); return err; 
        }   sk.set_sockopt( obj->agent );

        if( sk.bind() == -1 ){
            except_t err = "Error while binding UDP";
            onError.emit(err); return err; 
        }

        auto self = type::bind(this); process::add([=](){

            clb(sk); self->onSocket.emit( self, sk );
            /*----*/ self->obj->func(sk);
                
            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

        return -1; });

    return *this; }

    expected_t<udp_t,except_t>
    listen( const string_t& host, int port, NODE_CLB clb=nullptr ) const noexcept {
    auto addr = dns::lookup( host, obj->agent.socket_family );
        if( addr.empty() ){ 
            except_t err = "dns address not found";
            onError.emit(err); return err; 
        }   return listen( addr[0], port, clb );
    }

    /*─······································································─*/

    expected_t<udp_t,except_t>
    connect( const dns_t& addr, int port, NODE_CLB clb=nullptr ) const noexcept {

        if( obj->state & STATE::UDP_STATE_CLOSED )
          { except_t err = "udp connector is closed"; onError.emit(err); return err; } 
        if( obj->state & STATE::UDP_STATE_USED )
          { except_t err = "udp connector is used"  ; onError.emit(err); return err; } 

        socket_t sk; obj->state = STATE::UDP_STATE_USED;
        sk.AF      = addr.family;
        sk.SOCK    = SOCK_DGRAM ;
        sk.IPPROTO = IPPROTO_UDP;

        if( sk.socket( addr.address, port )==-1 ){
            except_t err = "Error while creating UDP";
            onError.emit(err); return err; 
        }   sk.set_sockopt( obj->agent );
        
        auto self = type::bind(this); process::add([=](){

            clb(sk); self->onSocket.emit( self, sk );
            /*----*/ self->obj->func(sk);

            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

        return -1; });

    return *this; }

    expected_t<udp_t,except_t>
    connect( const string_t& host, int port, NODE_CLB clb=nullptr ) const noexcept {
    auto addr = dns::lookup( host, obj->agent.socket_family );
        if( addr.empty() ){ 
            except_t err = "dns address not found";
            onError.emit(err); return err;
        }   return connect( addr[0], port, clb );
    }

    /*─······································································─*/

    void free() const noexcept {
        if( is_closed() ){ return; }
        obj->state = STATE::UDP_STATE_CLOSED; 
        onClose  .emit (); onSocket.clear();
        onError  .clear(); onOpen  .clear();
        onConnect.clear(); onClose .clear();
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace udp {

    inline udp_t server( agent_t* opt=nullptr ){
        auto skt = udp_t( nullptr, opt ); return skt;
    }

    inline udp_t client( agent_t* opt=nullptr ){
        auto skt = udp_t( nullptr, opt ); return skt;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/