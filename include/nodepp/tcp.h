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
#include "poll.h"
#include "dns.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class tcp_t {
protected:

    struct NODE {
        int                       state = 0;
        int                       accept=-2;
        agent_t                   agent;
        poll_t                    poll ;
        function_t<void,socket_t> func ;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    template< class T > void add_socket( T& cli ) const noexcept {
        auto self = type::bind(this); process::poll::add([=](){
             self->onSocket.emit(cli); self->obj->func(cli);
        if ( cli.is_available() ){ self->onConnect.emit(cli); } 
        return -1; });
    }

    int next() const noexcept { if( obj->state==0 ){ return -1; }
          if( obj->poll.next()==-1 ){ return -1; } auto x = obj->poll.get_last_poll();
          if( x[0] >= 0 )
            { socket_t cli(x[1]); cli.set_sockopt(obj->agent); add_socket(cli); }
        else{ socket_t cli(x[1]); cli.free(); } return 1;
    }

public: tcp_t() noexcept : obj( new NODE() ) {}

    event_t<socket_t> onConnect;
    event_t<socket_t> onSocket;
    event_t<>         onClose;
    event_t<except_t> onError;
    event_t<socket_t> onOpen;

    /*─······································································─*/

    tcp_t( decltype(NODE::func) _func, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr?agent_t():*opt; }

   ~tcp_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    /*─······································································─*/

    void     close() const noexcept { if(obj->state<=0){return;} obj->state=-1; onClose.emit(); }
    bool is_closed() const noexcept { return obj == nullptr ? 1: obj->state<=0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, decltype(NODE::func) cb ) const noexcept {
        if( obj->state == 1 ) { return; } if( dns::lookup(host).empty() )
          { _EERROR(onError,"dns couldn't get ip"); close(); return; }

        auto self = type::bind( this ); obj->state = 1;

        socket_t sk;
                 sk.SOCK    = SOCK_STREAM;
                 sk.IPPROTO = IPPROTO_TCP;
                 sk.socket( dns::lookup(host), port );
                 sk.set_sockopt( self->obj->agent );

        if( sk.bind()  <0 ){ _EERROR(onError,"Error while binding TCP");   close(); sk.free(); return; }
        if( sk.listen()<0 ){ _EERROR(onError,"Error while listening TCP"); close(); sk.free(); return; }

        cb( sk ); onOpen.emit( sk ); 
        
        process::poll::add( coroutine::add( COROUTINE(){
        coBegin

            while( self->obj->accept == -2 ){
               if( self->is_closed()|| sk.is_closed() ){ coGoto(2); }
                   self->obj->accept = sk._accept();
               if( self->obj->accept!=-2 ){ break; } coYield(3);
            while( self->next()==1 ){ coStay(3); } coNext; }

            if( self->obj->accept<0 ){ _EERROR(self->onError,"Error while accepting TCP"); coGoto(2); }
            do{ if( self->obj->poll.push_read(self->obj->accept)==0 )
              { socket_t cli( self->obj->accept ); cli.free(); }
              } while(0); self->obj->accept=-2; coStay(0);

            coYield(2); self->close(); sk.free();

        coFinish
        }));

    }

    /*─······································································─*/

    void connect( const string_t& host, int port, decltype(NODE::func) cb ) const noexcept {
        if( obj->state == 1 ){ return; } if( dns::lookup(host).empty() )
          { _EERROR(onError,"dns couldn't get ip"); close(); return; }

        auto self = type::bind(this); obj->state = 1;

        socket_t sk;
                 sk.SOCK    = SOCK_STREAM;
                 sk.IPPROTO = IPPROTO_TCP;
                 sk.socket( dns::lookup(host), port );
                 sk.set_sockopt( self->obj->agent );

        process::poll::add( coroutine::add( COROUTINE(){
        coBegin

            coWait( sk._connect()==-2 ); if( sk._connect()<=0 ){
                _EERROR(self->onError,"Error while connecting TCP");
                self->close(); coEnd;
            }

            if( self->obj->poll.push_write( sk.get_fd() ) ==0 )
              { sk.free(); } while( self->obj->poll.next()==0 ){
            if( process::now() > sk.get_send_timeout() )
              { coEnd; } coNext; } cb( sk );

            sk.onDrain.once([=](){ self->close(); });
            self->onSocket.emit(sk); self->obj->func(sk);
            
            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

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

    tcp_t server( const tcp_t& skt ){ skt.onConnect([=]( socket_t cli ){
        cli.onDrain.once([=](){ cli.free(); }); stream::pipe(cli);
    }); return skt; }

    /*─······································································─*/

    tcp_t server( agent_t* opt=nullptr ){
        auto skt = tcp_t( [=]( socket_t ){}, opt );
        tcp::server( skt ); return skt;
    }

    /*─······································································─*/

    tcp_t client( const tcp_t& skt ){ skt.onConnect.once([=]( socket_t cli ){
        cli.onDrain.once([=](){ cli.free(); }); stream::pipe(cli);
    }); return skt; }

    /*─······································································─*/

    tcp_t client( agent_t* opt=nullptr ){
        auto skt = tcp_t( [=]( socket_t ){}, opt );
        tcp::client( skt ); return skt;
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif