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
#include "poll.h"
#include "dns.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class udp_t {
protected:

    struct NODE {
        int                       state = 0;
        int                       accept=-2;
        agent_t                   agent;
        poll_t                    poll ;
        function_t<void,socket_t> func ;
    };  ptr_t<NODE> obj;

public: udp_t() noexcept : obj( new NODE() ) {}

    event_t<socket_t>         onConnect;
    event_t<socket_t>         onSocket;
    event_t<>                 onClose;
    event_t<except_t>         onError;
    event_t<socket_t>         onOpen;

    /*─······································································─*/

    udp_t( decltype(NODE::func) _func, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr?agent_t():*opt;  }

   ~udp_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    /*─······································································─*/

    void     close() const noexcept { if(obj->state<=0){return;} obj->state=-1; onClose.emit(); }

    bool is_closed() const noexcept { return obj == nullptr ? 1 :obj->state<=0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, decltype(NODE::func) cb ) const noexcept {
        if( obj->state == 1 ) { return; } if( dns::lookup(host).empty() )
          { _EERROR(onError,"dns couldn't get ip"); close(); return; }

        auto self = type::bind( this ); obj->state = 1;

        socket_t sk;
                 sk.SOCK    = SOCK_DGRAM;
                 sk.IPPROTO = IPPROTO_UDP;
                 sk.socket( dns::lookup(host), port );
                 sk.set_sockopt( self->obj->agent );

        process::poll::add([=](){ int c = 0; 
            if( self->is_closed() || sk.is_closed() ){ return -1; }
        coStart

            coWait( (c=sk._bind())==-2 ); if( c<0 ){
                _EERROR(self->onError,"Error while binding UDP");
                self->close(); sk.close(); coEnd;
            }

            cb(sk); sk.onClose.once([=](){ self->close(); });
            self->onSocket.emit(sk); sk.onOpen.emit();
            self->onOpen.emit(sk); self->obj->func(sk);

        coStop
        });

    }

    /*─······································································─*/

    void connect( const string_t& host, int port, decltype(NODE::func) cb ) const noexcept {
        if( obj->state == 1 ){ return; } if( dns::lookup(host).empty() )
          { _EERROR(onError,"dns couldn't get ip"); close(); return; }

        auto self = type::bind( this ); obj->state = 1;

        socket_t sk;
                 sk.SOCK    = SOCK_DGRAM;
                 sk.IPPROTO = IPPROTO_UDP;
                 sk.socket( dns::lookup(host), port );
                 sk.set_sockopt( self->obj->agent );

        process::poll::add([=](){
        coStart

            cb(sk); sk.onClose.once([=](){ self->close(); });
            self->onSocket.emit(sk); sk.onOpen.emit();
            self->onOpen.emit(sk); self->obj->func(sk);

        coStop
        });

    }

    /*─······································································─*/

    void connect( const string_t& host, int port ) const noexcept {
         connect( host, port, [=]( socket_t ){} );
    }

    void listen( const string_t& host, int port ) const noexcept {
         listen( host, port, []( socket_t ){} );
    }

    /*─······································································─*/

    void free() const noexcept {
        if( is_closed() ){ return; } close();
        onConnect.clear(); onSocket.clear();
        onError  .clear(); onOpen  .clear();
    //  onClose  .clear(); 
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace udp {

    udp_t server( const udp_t& skt ){ skt.onSocket([=]( socket_t cli ){
    process::task::add([=](){
        skt.onConnect.once([=]( socket_t cli ){ stream::pipe(cli); });
        cli.onDrain  .once([=](){ cli.free(); });
        skt.onConnect.emit(cli);
    return -1; }); }); return skt; }

    /*─······································································─*/

    udp_t server( agent_t* opt=nullptr ){
        auto skt = udp_t( [=]( socket_t /*unused*/ ){}, opt );
        udp::server( skt ); return skt;
    }

    /*─······································································─*/

    udp_t client( const udp_t& skt ){ skt.onSocket.once([=]( socket_t cli ){
    process::task::add([=](){
        skt.onConnect.once([=]( socket_t cli ){ stream::pipe(cli); });
        cli.onDrain  .once([=](){ cli.free(); });
        skt.onConnect.emit(cli);
    return -1; }); }); return skt; }

    /*─······································································─*/

    udp_t client( agent_t* opt=nullptr ){
        auto skt = udp_t( [=]( socket_t /*unused*/ ){}, opt );
        udp::client( skt ); return skt;
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif
