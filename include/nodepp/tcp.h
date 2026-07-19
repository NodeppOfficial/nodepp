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

#include "expected.h"
#include "socket.h"
#include "dns.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class tcp_t {
private:

    using NODE_CLB = function_t<void,socket_t>;
    enum STATE {
         TCP_STATE_UNKNOWN   = 0b00000000,
         TCP_STATE_USED      = 0b00000001,
         TCP_STATE_CLOSED    = 0b00000010
    };

protected:

    struct NODE {
        int state = 0; 
        agent_t agent;
        NODE_CLB func;
    };  ptr_t<NODE> obj;

public:

    event_t<socket_t>              onConnect;
    event_t<ptr_t<tcp_t>,socket_t> onSocket ;
    event_t<>                      onClose  ;
    event_t<except_t>              onError  ;
    event_t<socket_t>              onOpen   ;

    /*─······································································─*/

    tcp_t( NODE_CLB _func, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr ? agent_t() : *opt; }

   ~tcp_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tcp_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    bool is_closed() const noexcept { return obj->state & STATE::TCP_STATE_CLOSED; }
    void     close() const noexcept { free(); }

    /*─······································································─*/

    expected_t<tcp_t,except_t>
    listen( const dns_t& addr, int port, NODE_CLB clb=nullptr ) const noexcept {

        if( obj->state & STATE::TCP_STATE_CLOSED )
          { except_t err = "tcp listener is closed"; onError.emit(err); return err; } 
        if( obj->state & STATE::TCP_STATE_USED )
          { except_t err = "tcp listener is used"; onError.emit(err); return err; } 

        socket_t sk; obj->state = STATE::TCP_STATE_USED;
        sk.AF      = addr.family;
        sk.SOCK    = SOCK_STREAM;
        sk.IPPROTO = IPPROTO_TCP;

        if( sk.socket( addr.address, port )==-1 ){
            except_t err = "Error while creating TCP";
            onError.emit(err); return err; 
        }   sk.set_sockopt( obj->agent );

        if( sk.bind() == -1 ){
            except_t err = "Error while binding TCP";
            onError.emit(err); return err; 
        }

        if( sk.listen() == -1 ){ 
            except_t err = "Error while listening TCP";
            onError.emit(err); return err; 
        }   
        
        clb(sk); onOpen.emit(sk); 
        auto self= type::bind ( this ); 
        auto enb = ptr_t<uint>( 0UL, 0u );

        process::poll( sk, POLL_STATE::READ | POLL_STATE::EDGE, [=](){

            while( *enb > NODEPP_MAX_BATCH_SIZE ){ return 1; } int c=-1;

            if((c= sk._accept())==-2 ){ /*-----------------------------*/ return 1; }
            if( c==-1 ){ self->onError.emit("Error while accepting TCP"); return 1; }
            
            auto cli=socket_t(c); *enb += 1;
            /**/ cli.set_sockopt( self->obj->agent );
        
        stream::readable( cli, 0UL ).then([=]( socket_t cli ){

            self->onSocket.emit( self, cli ); 
            self->obj->func(cli);

            if( cli.is_available() ){ self->onConnect.emit(cli); }

        }).finally([=](){ *enb -= 1; }); return 1; });

    return *this; }

    expected_t<tcp_t,except_t>
    listen( const string_t& host, int port, NODE_CLB clb=nullptr ) const noexcept {
    auto addr = dns::lookup( host, obj->agent.socket_family );
        if( addr.empty() ){ 
            except_t err = "dns address not found";
            onError.emit(err); return err;
        }   return listen( addr[0], port, clb );
    }

    /*─······································································─*/

    expected_t<tcp_t,except_t>
    connect( const dns_t& addr, int port, NODE_CLB clb=nullptr ) const noexcept {

        if( obj->state & STATE::TCP_STATE_CLOSED )
          { except_t err = "tcp connector is closed"; onError.emit(err); return err; } 
        if( obj->state & STATE::TCP_STATE_USED )
          { except_t err = "tcp connector is used"  ; onError.emit(err); return err; } 

        socket_t sk; obj->state = STATE::TCP_STATE_USED;
        sk.AF      = addr.family;
        sk.SOCK    = SOCK_STREAM;
        sk.IPPROTO = IPPROTO_TCP;

        if( sk.socket( addr.address, port )==-1 ){
            except_t err = "Error while creating TCP";
            onError.emit(err); return err; 
        }   sk.set_sockopt( obj->agent );
        
        auto self = type::bind(this); process::add([=](){ int c=0;

            while( (c=sk._connect())==-2 ){ return 1; } if(c==-1){
                self->onError.emit( "Error while connecting TCP" );
            return -1; }

            clb(sk); self->onSocket.emit( self, sk );
            /*----*/ self->obj->func(sk);

            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

        return -1; }); 

    return *this; }

    expected_t<tcp_t,except_t>
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
        obj->state = STATE::TCP_STATE_CLOSED; 
        onClose  .emit (); onSocket.clear();
        onError  .clear(); onOpen  .clear();
        onConnect.clear(); onClose .clear();
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace tcp {

    inline tcp_t server( agent_t* opt=nullptr ){
    auto skt = tcp_t( nullptr, opt ); return skt; }

    inline tcp_t client( agent_t* opt=nullptr ){
    auto skt = tcp_t( nullptr, opt ); return skt; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/