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
#include "expected.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class tls_t {
private:

    using NODE_CLB = function_t<void,ssocket_t>;
    enum STATE {
         TLS_STATE_UNKNOWN   = 0b00000000,
         TLS_STATE_USED      = 0b00000001,
         TLS_STATE_CLOSED    = 0b00000010
    };

protected:

    struct NODE {
        int state = 0;
        ssl_t     ctx;
        agent_t agent;
        NODE_CLB func;
    };  ptr_t<NODE> obj;

public:

    event_t<ptr_t<tls_t>,ssocket_t> onSocket ;
    event_t<ssocket_t>              onConnect;
    event_t<>                       onClose  ;
    event_t<except_t>               onError  ;
    event_t<ssocket_t>              onOpen   ;

    /*─······································································─*/

    tls_t( NODE_CLB _func, ssl_t* crt=nullptr, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->agent=opt==nullptr ? agent_t(): *opt; 
           obj->ctx  =crt==nullptr ? ssl_t()  : *crt; 
           obj->func = /*-------------------*/ _func; }

   ~tls_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tls_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    bool is_closed() const noexcept { return obj->state & STATE::TLS_STATE_CLOSED; }
    void     close() const noexcept { free(); }

    /*─······································································─*/

    expected_t<tls_t,except_t>
    listen( const dns_t& addr, int port, NODE_CLB clb=nullptr ) const noexcept {

        if( obj->state & STATE::TLS_STATE_CLOSED )
          { except_t err = "tls listener is closed"; onError.emit(err); return err; } 
        if( obj->state & STATE::TLS_STATE_USED )
          { except_t err = "tls listener is used"  ; onError.emit(err); return err; } 

        if( obj->ctx.create_server()==-1 ){ 
            except_t err = "Error Initializing SSL context"; 
            onError.emit(err); return err; 
        }

        ssocket_t sk; obj->state = STATE::TLS_STATE_USED;
        sk.AF      = addr.family ;
        sk.SOCK    = SOCK_STREAM ;
        sk.IPPROTO = IPPROTO_TCP ;

        if( sk.socket( addr.address, port )==-1 ){
            except_t err = "Error while creating TLS";
            onError.emit(err); return err; 
        }   sk.set_sockopt( obj->agent );

        if( sk.bind() == -1 ){
            except_t err = "Error while binding TLS";
            onError.emit(err); return err; 
        }

        if( sk.listen() == -1 ){ 
            except_t err = "Error while listening TLS";
            onError.emit(err); return err; 
        }   
        
        clb(sk); onOpen.emit(sk); 
        auto self= type::bind ( this ); 
        auto enb = ptr_t<uint>( 0UL, 0u );
            
        process::poll( sk, POLL_STATE::READ | POLL_STATE::EDGE, [=](){

            while( *enb > NODEPP_MAX_BATCH_SIZE ){ return 1; } int c=-1;

            if((c= sk._accept())==-2 ){ /*-----------------------------*/ return 1; }
            if( c==-1 ){ self->onError.emit("Error while accepting TLS"); return 1; }
            
            auto cli=ssocket_t( self->obj->ctx , c ); *enb += 1;
            /**/ cli.set_sockopt( self->obj->agent );
            
        stream::readable( cli, 0UL ).then([=]( ssocket_t cli ){

            self->onSocket.emit( self, cli ); 
            self->obj->func(cli);
            
            if( cli.is_available() ){ self->onConnect.emit(cli); }

        }).finally([=](){ *enb -= 1; }); return 1; });
        
    return *this; }

    expected_t<tls_t,except_t>
    listen( const string_t& host, int port, NODE_CLB clb=nullptr ) const noexcept {
    auto addr = dns::lookup( host, obj->agent.socket_family );
        if( addr.empty() ){ 
            except_t err = "dns address not found";
            onError.emit(err); return err; 
        }   return listen( addr[0], port, clb );
    }

    /*─······································································─*/

    expected_t<tls_t,except_t>
    connect( const dns_t& addr, int port, NODE_CLB clb=nullptr ) const noexcept {

        if( obj->state & STATE::TLS_STATE_CLOSED )
          { except_t err = "tls connector is closed"; onError.emit(err); return err; } 
        if( obj->state & STATE::TLS_STATE_USED )
          { except_t err = "tls connector is used"  ; onError.emit(err); return err; }

        if( obj->ctx.create_client()==-1 || addr.hostname.empty() ){ 
            except_t err = "Error Initializing SSL context";
            onError.emit(err); return err; 
        }

        ssocket_t sk; obj->state= STATE::TLS_STATE_USED;
        sk.AF      = addr.family;
        sk.SOCK    = SOCK_STREAM;
        sk.IPPROTO = IPPROTO_TCP;

        if( sk.socket( addr.address, port )==-1 ){
            except_t err = "Error while creating TLS";
            onError.emit(err); return err; 
        }   sk.set_sockopt( obj->agent );
        
        sk.ssl = new ssl_t( obj->ctx, sk.get_fd() );
        sk.ssl->set_hostname( addr.hostname );

        auto self = type::bind(this); process::add([=](){ int c=0;

            while( (c=sk._connect())==-2 ){ return 1; } if(c==-1){
                self->onError.emit( "Error while connecting TLS" );
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

    expected_t<tls_t,except_t>
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
        obj->state = STATE::TLS_STATE_CLOSED; 
        onClose  .emit (); onSocket.clear();
        onError  .clear(); onOpen  .clear();
        onConnect.clear(); onClose .clear();
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace tls {

    inline tls_t server( ssl_t* ssl=nullptr, agent_t* opt=nullptr ){ 
    auto   skt = tls_t( nullptr, ssl, opt ); return skt; }

    inline tls_t client( ssl_t* ssl=nullptr, agent_t* opt=nullptr ){
    auto   skt = tls_t( nullptr, ssl, opt ); return skt; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/