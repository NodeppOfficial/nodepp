/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
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
        int                       state =  0;
        agent_t                   agent;
        poll_t                    poll ;
        function_t<void,socket_t> func ;
    };  ptr_t<NODE> obj;
    
public: udp_t() noexcept : obj(new NODE()) {}

    event_t<socket_t>         onConnect;
    event_t<socket_t>         onSocket;
    event_t<>                 onClose;
    event_t<except_t>         onError;
    event_t<socket_t>         onOpen;
    
    /*─······································································─*/

    udp_t( decltype(NODE::func) _func, agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr?agent_t():*opt;  }
    
    /*─······································································─*/
    
    void     close() const noexcept { if( obj->state<=0 ){ return; } obj->state=-1; onClose.emit(); }
    
    bool is_closed() const noexcept { return obj == nullptr ? 1 : obj->state <= 0; }
    
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

        process::poll::add([=](){
            int c = 0; if( self->is_closed() ){ return -1; }
        coStart

            while( (c=sk._bind())==-2 ){ coNext; } if( c < 0 ){ 
                _EERROR(self->onError,"Error while binding UDP"); 
                self->close(); sk.close(); coEnd; 
            }
            
            cb(sk); sk.onClose.once([=](){ self->close(); }); 
            self->onSocket.emit(sk); sk.onOpen.emit();
            self->onOpen.emit(sk); self->obj->func(sk);

        coStop
        });

    }

    void listen( const string_t& host, int port ) const noexcept { 
         listen( host, port, []( socket_t ){} );
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

    void connect( const string_t& host, int port ) const noexcept { 
         connect( host, port, [=]( socket_t ){} ); 
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace udp {

    udp_t server( const udp_t& server ){ server.onSocket([=]( socket_t cli ){
        cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); });
        ptr_t<_file_::read> _read = new _file_::read;

        server.onConnect.once([=]( socket_t cli ){ process::poll::add([=](){
            if(!cli.is_available() )    { cli.close(); return -1; }
            if((*_read)(&cli)==1 )      { return 1; }
            if(  _read->state<=0 )      { return 1; }
            cli.onData.emit(_read->data); return 1;
        }) ; });

        process::task::add([=](){
            server.onConnect.emit(cli); return -1;
        });

    }); return server; }

    /*─······································································─*/

    udp_t server( agent_t* opt=nullptr ){
        auto server = udp_t( [=]( socket_t /*unused*/ ){}, opt );
        udp::server( server ); return server; 
    }

    /*─······································································─*/

    udp_t client( const udp_t& client ){ client.onOpen.once([=]( socket_t cli ){
        cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); });
        ptr_t<_file_::read> _read = new _file_::read;

        process::poll::add([=](){
            if(!cli.is_available() )    { cli.close(); return -1; }
            if((*_read)(&cli)==1 )      { return 1; }
            if(  _read->state<=0 )      { return 1; }
            cli.onData.emit(_read->data); return 1;
        });

    }); return client; }

    /*─······································································─*/

    udp_t client( agent_t* opt=nullptr ){
        auto client = udp_t( [=]( socket_t /*unused*/ ){}, opt );
        udp::client( client ); return client; 
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif