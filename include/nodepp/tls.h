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
    enum STATE {
         TLS_STATE_UNKNOWN   = 0b00000000,
         TLS_STATE_USED      = 0b00000001,
         TLS_STATE_CLOSED    = 0b00000010
    };

protected:

    struct NODE {
        int  state= 0;
        ssl_t     ctx;
        agent_t agent;
        NODE_CLB func;
    };  ptr_t<NODE> obj;

public:

    event_t<ssocket_t> onConnect;
    event_t<ssocket_t> onSocket;
    event_t<>          onClose;
    event_t<except_t>  onError;
    event_t<ssocket_t> onOpen;

    /*─······································································─*/

    tls_t( NODE_CLB _func, ssl_t* crt=nullptr, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->agent=opt==nullptr ? agent_t(): *opt; 
           obj->ctx  =crt==nullptr ? ssl_t()  : *crt; 
           obj->func = /*-------------------*/ _func; }

   ~tls_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tls_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    bool is_closed() const noexcept { return obj->state & STATE::TLS_STATE_CLOSED; }
    void     close() const noexcept { 
        if( is_closed() ){ return; } 
        obj->state = STATE::TLS_STATE_CLOSED; 
        onClose.emit(); 
    }

    /*─······································································─*/

    void listen( const dns_t& addr, int port, NODE_CLB cb=nullptr ) const noexcept {

        if( obj->state & STATE::TLS_STATE_CLOSED )
          { onError.emit("tls listener is closed"); return; } 
        if( obj->state & STATE::TLS_STATE_USED )
          { onError.emit("tls listener is used");   return; } 

        if( obj->ctx.create_server()==-1 )
          { onError.emit("Error Initializing SSL context"); return; }

        ssocket_t sk; obj->state= STATE::TLS_STATE_USED;
        sk.AF      = addr.family ;
        sk.SOCK    = SOCK_STREAM ;
        sk.IPPROTO = IPPROTO_TCP ;

        if( sk.socket( addr.address, port )==-1 ){
            onError.emit("Error while creating TLS"); 
            close(); sk.free(); return; 
        }   sk.set_sockopt( obj->agent );

        if( sk.bind() == -1 ){
            onError.emit("Error while binding TLS"); 
            close(); sk.free(); return; 
        }

        if( sk.listen() == -1 ){ 
            onError.emit("Error while listening TLS"); 
            close(); sk.free(); return; 
        }   
        
        auto self=type::bind( this );
        cb( sk );  onOpen.emit( sk ); 
        sk.onDrain.once([=](){ self->close(); });
            
        process::poll( sk, POLL_STATE::READ | POLL_STATE::EDGE, [=](){
        int c=-1; while( self.count() < MAX_BATCH ) {

            while((c=sk._accept())==-2){ return 0; } if(c==-1){ 
                self->onError.emit("Error while accepting TLS");
            return -1; }
            
            auto cli   = ssocket_t( self->obj->ctx, c ); 
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

    void listen( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
    auto addr = dns::lookup( host, obj->agent.socket_family );
         if( addr.empty() ){ onError.emit( "dns address not found" ); return; }
         listen( addr[0], port, cb );
    }

    /*─······································································─*/

    void connect( const dns_t& addr, int port, NODE_CLB cb=nullptr ) const noexcept {

        if( obj->state & STATE::TLS_STATE_CLOSED )
          { onError.emit("tls listener is closed"); return; } 
        if( obj->state & STATE::TLS_STATE_USED )
          { onError.emit("tls listener is used");   return; }

        if( obj->ctx.create_client()==-1 )
          { onError.emit("Error Initializing SSL context"); return; }

        ssocket_t sk; obj->state= STATE::TLS_STATE_USED;
        sk.AF      = addr.family ;
        sk.SOCK    = SOCK_STREAM ;
        sk.IPPROTO = IPPROTO_TCP ;

        if( sk.socket( addr.address, port )==-1 ){
            onError.emit("Error while creating TLS"); 
            close(); sk.free(); return; 
        }   sk.set_sockopt( obj->agent );
        
        sk.ssl = new ssl_t( obj->ctx, sk.get_fd() );
        sk.ssl->set_hostname( addr.hostname );

        auto self = type::bind( this ); 
        sk.onDrain.once([=](){ self->close(); }); 

        process::add([=](){ int c=0;

            while( (c=sk._connect())==-2 ){ return 1; } if(c==-1){
                self->onError.emit("Error while connecting TLS");
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

    void connect( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
    auto addr = dns::lookup( host, obj->agent.socket_family );
         if( addr.empty() ){ onError.emit( "dns address not found" ); return; }
         connect( addr[0], port, cb );
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
    auto   skt = tls_t( nullptr, ssl, opt ); return skt; }

    inline tls_t client( ssl_t* ssl=nullptr, agent_t* opt=nullptr ){
    auto   skt = tls_t( nullptr, ssl, opt ); return skt; }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/