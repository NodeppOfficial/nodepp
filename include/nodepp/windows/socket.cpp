/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once

#include <ws2tcpip.h>
#include <winsock2.h>
#include <winsock.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace _socket_ {

    void start_device(){ static bool sockets=false;
        if( sockets == false ){ WSADATA wsaData;
            process::onSIGEXIT([=](){ WSACleanup(); });
            if( WSAStartup(MAKEWORD(2,2),&wsaData) != 0 )
              { throw except_t("Failed to initialize Winsock"); }
        }   sockets = true;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

struct agent_t {
    bool  reuse_address = 1;
    bool  no_delay_mode = 0;
    uint  conn_timeout  = 0;//10000 ;
    uint  recv_timeout  = 0;//120000;
    uint  send_timeout  = 0;//120000;
    ulong buffer_size   = CHUNK_SIZE;
    bool  reuse_port    = 1;
    bool  keep_alive    = 0;
    bool  broadcast     = 0;
};

class socket_t {
protected:

    void kill() const noexcept {
        obj->state |= FILE_STATE::KILL;
        ::shutdown( obj->fd, SD_BOTH ); 
        ::closesocket( obj->fd );
    }

    bool is_state( uchar value ) const noexcept {
        if( obj->state & value ){ return true; }
    return false; }

    void set_state( uchar value ) const noexcept {
    if( obj->state & KILL ){ return; }
        obj->state = value;
    }

    enum FILE_STATE {
        UNKNOWN = 0b00000000,
        OPEN    = 0b00000001,
        CLOSE   = 0b00000010,
        KILL    = 0b00000100,
        REUSE   = 0b00001000,
        DISABLE = 0b00001110
    };

protected:

    using TIMEVAL     = struct timeval;
    using SOCKADDR    = struct sockaddr;
    using SOCKADDR_IN = struct sockaddr_in;

    struct NODE {

        ulong recv_timeout=0; ulong send_timeout=0;
        ulong conn_timeout=0;

        SOCKADDR server_addr, client_addr;
        int addrlen; bool srv=0; int len;
        int feof = 1;

        uchar        state    = FILE_STATE::OPEN;
        SOCKET       fd       = INVALID_SOCKET;
        ulong        range[2] = { 0, 0 };
        bool         keep     = false;
        ptr_t<char>  buffer;
        string_t     borrow;

        limit::probe_t limit_probe;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    virtual bool is_blocked( int& c ) const noexcept {
        if ( c >= 0 ){ return 0; } auto error =  WSAGetLastError();
        if ( error == WSAEISCONN ){ c=0; return 0; } return (
             error == WSAEWOULDBLOCK || error == WSAEINPROGRESS ||
             error == WSAEALREADY    || error == WSAECONNRESET
        );
    }

    /*─······································································─*/

    virtual int set_nonbloking_mode() const noexcept { ulong mode = 1;
        return ioctlsocket( obj->fd, FIONBIO, &mode ) != 0 ? -1 : 1;
    }

public:

    event_t<>          onUnpipe;
    event_t<>          onResume;
    event_t<except_t>  onError;
    event_t<>          onDrain;
    event_t<>          onClose;
    event_t<>          onStop;
    event_t<>          onOpen;
    event_t<>          onPipe;
    event_t<string_t>  onData;

    /*─······································································─*/

    int SOCK    = SOCK_STREAM;
    int AF      = AF_INET;
    int IPPROTO = 0;

    /*─······································································─*/

    ulong get_recv_timeout() const noexcept {
        return obj->recv_timeout==0 ? process::millis() : obj->recv_timeout;
    }

    ulong get_send_timeout() const noexcept {
        return obj->send_timeout==0 ? process::millis() : obj->send_timeout;
    }

    ulong get_conn_timeout() const noexcept {
        return obj->conn_timeout==0 ? process::millis() : obj->conn_timeout;
    }

    /*─······································································─*/

    ulong set_conn_timeout( ulong time ) const noexcept {
        if( time == 0 ){ obj->conn_timeout = 0; return 0; }
        obj->conn_timeout = process::millis() + time; return time;
    }

    ulong set_recv_timeout( ulong time ) const noexcept {
        if( time == 0 ){ obj->recv_timeout = 0; return 0; }
        TIMEVAL en; memset( &en, 0, sizeof(en) ); en.tv_sec = time / 1000; en.tv_usec = 0; int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&en, sizeof(en) ) ) )
             { process::next(); } obj->recv_timeout = process::millis() + time; return time;
    }

    ulong set_send_timeout( ulong time ) const noexcept {
        if( time == 0 ){ obj->send_timeout = 0; return 0; }
        TIMEVAL en; memset( &en, 0, sizeof(en) ); en.tv_sec = time / 1000; en.tv_usec = 0; int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&en, sizeof(en) ) ) )
             { process::next(); } obj->send_timeout = process::millis() + time; return time;
    }

    /*─······································································─*/

    int set_no_delay_mode( uint en ) const noexcept { int c; if( IPPROTO != IPPROTO_TCP ){ return -1; }
        while( is_blocked( c=setsockopt( obj->fd, IPPROTO, TCP_NODELAY, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }

    int set_recv_buff( uint en ) const noexcept { int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_RCVBUF, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }

    int set_send_buff( uint en ) const noexcept { int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_SNDBUF, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }

    int set_accept_connection( uint en ) const noexcept { int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_ACCEPTCONN, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }

    int set_dont_route( uint en ) const noexcept { int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_DONTROUTE, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }

    int set_keep_alive( uint en ) const noexcept { int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }

    int set_broadcast( uint en ) const noexcept { int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_BROADCAST, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }

    int set_reuse_address( uint en ) const noexcept { int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_REUSEADDR, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }

    int set_ipv6_only_mode( uint en ) const noexcept { int c;
        while( is_blocked( c=setsockopt( obj->fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }

#ifdef SO_REUSEPORT
    int set_reuse_port( uint en ) const noexcept { int c;
        while( is_blocked( c=setsockopt( obj->fd, SOL_SOCKET, SO_REUSEPORT, (char*)&en, sizeof(en) ) ) )
             { process::next(); } return c;
    }
#endif

    int get_no_delay_mode() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt( obj->fd, IPPROTO, TCP_NODELAY, (char*)&en, &size ) ) )
             { process::next(); } return c==0 ? en : c;
    }

    int get_error() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, SOL_SOCKET, SO_ERROR, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }

    int get_recv_buff() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, SOL_SOCKET, SO_RCVBUF, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }

    int get_send_buff() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, SOL_SOCKET, SO_SNDBUF, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }

    int get_accept_connection() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, SOL_SOCKET, SO_ACCEPTCONN, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }

    int get_dont_route() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, SOL_SOCKET, SO_DONTROUTE, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }

    int get_reuse_address() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, SOL_SOCKET, SO_REUSEADDR, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }

#ifdef SO_REUSEPORT
    int get_reuse_port() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, SOL_SOCKET, SO_REUSEPORT, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }
#endif

    int get_ipv6_only_mode() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }

    int get_keep_alive() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }

    int get_broadcast() const noexcept { int c, en; socklen_t size = sizeof(en);
        while( is_blocked( c=getsockopt(obj->fd, SOL_SOCKET, SO_BROADCAST, (char*)&en, &size) ) )
             { process::next(); } return c==0 ? en : c;
    }

    /*─······································································─*/

    string_t get_sockname() const noexcept { int c; string_t buff { INET_ADDRSTRLEN };
        SOCKADDR cli; if( obj->srv==1 ) cli = obj->client_addr; else cli = obj->server_addr;
        while( is_blocked( c=getsockname( obj->fd, &cli, &obj->len )) ){ process::next(); }
        inet_ntop( AF, &(((SOCKADDR_IN*)&cli)->sin_addr), (char*)buff, buff.size() );
        return c < 0 ? "127.0.0.1" : buff;
    }

    string_t get_peername() const noexcept { int c; string_t buff { INET_ADDRSTRLEN };
        SOCKADDR cli; if( obj->srv==1 ) cli = obj->client_addr; else cli = obj->server_addr;
        while( is_blocked( c=getpeername( obj->fd, &cli, &obj->len )) ){ process::next(); }
        inet_ntop( AF, &(((SOCKADDR_IN*)&cli)->sin_addr), (char*)buff, buff.size() );
        return c < 0 ? "127.0.0.1" : buff;
    }

    int get_sockport() const noexcept {
        SOCKADDR cli; if( obj->srv==1 ) cli = obj->client_addr; else cli = obj->server_addr;
        return ntohs( ((SOCKADDR_IN*)&cli)->sin_port );
    }

    /*─······································································─*/

    virtual ulong set_buffer_size( ulong _size ) const noexcept {
        set_send_buff( _size ); set_recv_buff( _size );
        obj->buffer = ptr_t<char>(_size); return _size;
    }

    /*─······································································─*/

    ulong set_timeout( ulong time ) const noexcept {
        set_recv_timeout( time );
        set_send_timeout( time ); return time;
    }

    /*─······································································─*/

    bool     is_closed() const noexcept { return is_state(FILE_STATE::DISABLE) || is_feof() || obj->fd==INVALID_HANDLE_VALUE; }
    bool       is_feof() const noexcept { return obj->feof <= 0 && obj->feof != -2; }
    bool  is_available() const noexcept { return !is_closed(); }
    bool is_persistent() const noexcept { return obj->keep; }
    bool     is_server() const noexcept { return obj->srv;  }

    /*─······································································─*/

    void  resume() const noexcept { if(is_state(FILE_STATE::OPEN )){ return; } set_state(FILE_STATE::OPEN ); onResume.emit(); }
    void    stop() const noexcept { if(is_state(FILE_STATE::REUSE)){ return; } set_state(FILE_STATE::REUSE); onStop  .emit(); }
    void   reset() const noexcept { if(is_state(FILE_STATE::KILL )){ return; } resume(); pos(0); }
    void   flush() const noexcept { obj->buffer.fill(0); }

    /*─······································································─*/

    void close() const noexcept { 
        if( is_closed()  ){ return; } 
        if( obj->keep==1 ){ stop(); goto DONE; }
         set_state( FILE_STATE::CLOSE ); DONE:; 
    onDrain.emit(); }

    /*─······································································─*/

    SOCKET    get_fd() const noexcept { return obj == nullptr ? INVALID_SOCKET : obj->fd;    }
    ulong* get_range() const noexcept { return obj == nullptr ?        nullptr : obj->range; }

    /*─······································································─*/

    void   set_borrow( const string_t& brr ) const noexcept { obj->borrow = brr; }
    ulong  get_borrow_size() const noexcept { return obj->borrow.size(); }
    char*  get_borrow_data() const noexcept { return obj->borrow.data(); }
    void        del_borrow() const noexcept { obj->borrow.clear(); }
    string_t&   get_borrow() const noexcept { return obj->borrow; }

    /*─······································································─*/

    ulong   get_buffer_size() const noexcept { return obj->buffer.size(); }
    char*   get_buffer_data() const noexcept { return obj->buffer.data(); }
    ptr_t<char>& get_buffer() const noexcept { return obj->buffer; }

    /*─······································································─*/

    ulong pos( ulong /*unused*/ ) const noexcept { return 0; }

    ulong size() const noexcept { return 0; }

    ulong  pos() const noexcept { return 0; }

    /*─······································································─*/

    void set_sockopt( agent_t opt ) const noexcept {
        set_no_delay_mode( opt.no_delay_mode );
        set_reuse_address( opt.reuse_address );
        set_conn_timeout ( opt.conn_timeout  );
        set_recv_timeout ( opt.recv_timeout  );
        set_send_timeout ( opt.send_timeout  );
        set_buffer_size  ( opt.buffer_size   );
    #ifdef SO_REUSEPORT
        set_reuse_port   ( opt.reuse_port    );
    #endif
        set_keep_alive   ( opt.keep_alive    );
        set_broadcast    ( opt.broadcast     );
    }

    agent_t get_sockopt() const noexcept {
    agent_t opt;
        opt.reuse_address = get_reuse_address();
        opt.recv_timeout  = get_recv_timeout();
        opt.send_timeout  = get_send_timeout();
        opt.conn_timeout  = get_conn_timeout();
        opt.buffer_size   = get_buffer_size();
    #ifdef SO_REUSEPORT
        opt.reuse_port    = get_reuse_port();
    #endif
        opt.keep_alive    = get_keep_alive();
        opt.broadcast     = get_broadcast();
    return opt;
    }

    /*─······································································─*/

    socket_t( SOCKET fd, ulong _size=CHUNK_SIZE ) : obj( new NODE() ) { _socket_::start_device();
        if( fd == INVALID_SOCKET ){ throw except_t("Such Socket has an Invalid fd"); }
        obj->fd = fd; set_nonbloking_mode(); set_buffer_size(_size);
    }

    socket_t() noexcept : obj( new NODE() ) {}

    virtual ~socket_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    /*─······································································─*/

    virtual void free() const noexcept {

        if( is_state(FILE_STATE::REUSE) && obj.count() > 1 ){ resume(); return; }
        if( is_state(FILE_STATE::KILL ) ){ return; } close(); kill();

        onUnpipe.clear(); onResume.clear();
        onStop  .clear(); onError .clear();
        onOpen  .clear(); onPipe  .clear();
        onData  .clear(); /*-------------*/ onClose.emit();

    }

    /*─······································································─*/

    virtual int socket( const string_t& host, int port ) const noexcept {
        if( host.empty() ){ onError.emit("dns coudn't found ip"); return -1; }
            obj->addrlen = sizeof( obj->server_addr ); _socket_::start_device();

        if((obj->fd=::socket( AF, SOCK, IPPROTO )) == INVALID_SOCKET )
          { onError.emit("can't initializate socket fd"); return -1; }

        set_buffer_size( CHUNK_SIZE );
        set_nonbloking_mode();
        set_ipv6_only_mode(0);
        set_reuse_address(1);

    #ifdef SO_REUSEPORT
        set_reuse_port(1);
    #endif

        SOCKADDR_IN server, client;
        memset(&server, 0, sizeof(SOCKADDR_IN));
        memset(&client, 0, sizeof(SOCKADDR_IN));
        server.sin_family = AF; if( port>0 ) server.sin_port = htons(port);

          if( host == "0.0.0.0"         || host == "global"    ){ server.sin_addr.s_addr = INADDR_ANY; }
        elif( host == "1.1.1.1"         || host == "loopback"  ){ server.sin_addr.s_addr = INADDR_LOOPBACK; }
        elif( host == "255.255.255.255" || host == "broadcast" ){ server.sin_addr.s_addr = INADDR_BROADCAST; }
        elif( host == "127.0.0.1"       || host == "localhost" ){ inet_pton(AF, "127.0.0.1", &server.sin_addr); }
        else                                                    { inet_pton(AF, host.c_str(),&server.sin_addr); }

        obj->server_addr = *((SOCKADDR*) &server); obj->client_addr = *((SOCKADDR*) &client); obj->len = sizeof( server ); return 1;
    }

    /*─······································································─*/

    inline int _connect() const noexcept { int c=0;
        if( process::millis() > get_conn_timeout() || obj->srv == 1 ){ return -1; }
        return is_blocked( c=::connect( obj->fd, &obj->server_addr, obj->addrlen ) ) ? -2 : c>=0 ? 1: -1;
    }

    inline int _accept() const noexcept { int c=0; if( obj->srv == 0 ){ return -1; }
        return is_blocked( c=::accept( obj->fd, &obj->server_addr, &obj->addrlen ) ) ? -2 : c;
    }

    inline int _bind() const noexcept {
        if( process::millis() > get_conn_timeout() ){ return -1; } int c=0; obj->srv = 1;
        return is_blocked( c=::bind( obj->fd, &obj->server_addr, obj->addrlen ) ) ? -2 : c;
    }

    inline int _listen() const noexcept { int c = 0;
        if( process::millis() > get_conn_timeout() || obj->srv == 0 ){ return -1; }
        return is_blocked( c=::listen( obj->fd, limit::get_soft_fileno() ) ) ? -2 : c;
    }

    /*─······································································─*/

    int connect() const noexcept { int c=0;
        while( (c=_connect())==-2 ){ process::next(); } return c;
    }

    int listen() const noexcept { int c=0;
        while( (c=_listen())==-2 ){ process::next(); } return c;
    }

    int accept() const noexcept { int c=0;
        while( (c=_accept())==-2 ){ process::next(); } return c;
    }

    int bind() const noexcept { int c=0;
        while( (c=_bind())==-2 ){ process::next(); } return c;
    }

    /*─······································································─*/

    char read_char() const noexcept { return read(1)[0]; }

    string_t read_until( string_t ch ) const noexcept {
        auto gen = generator::file::until();
        while( gen( this, ch ) == 1 )
             { process::next(); }
        return gen.data;
    }

    string_t read_until( char ch ) const noexcept {
        auto gen = generator::file::until();
        while( gen( this, ch ) == 1 )
             { process::next(); }
        return gen.data;
    }

    string_t read_line() const noexcept {
        auto gen = generator::file::line();
        while( gen( this ) == 1 )
             { process::next(); }
        return gen.data;
    }

    /*─······································································─*/

    string_t read( ulong size=CHUNK_SIZE ) const noexcept {
        auto gen = generator::file::read();
        while( gen( this, size ) == 1 )
             { process::next(); }
        return gen.data;
    }

    ulong write( const string_t& msg ) const noexcept {
        auto gen = generator::file::write();
        while( gen( this, msg ) == 1 )
             { process::next(); }
        return gen.data;
    }

    /*─······································································─*/

    virtual int _read ( char* bf, const ulong& sx ) const noexcept { return __read ( bf, sx ); }
    virtual int _write( char* bf, const ulong& sx ) const noexcept { return __write( bf, sx ); }

    /*─······································································─*/

    virtual int __read( char* bf, const ulong& sx ) const noexcept {
        if ( process::millis() > get_recv_timeout() || is_closed() )
           { return -1; } if ( sx==0 ) { return 0; }
        if ( SOCK != SOCK_DGRAM ){
            obj->feof = ::recv( obj->fd, bf, sx, 0 );
            obj->feof = is_blocked(obj->feof)? -2 : obj->feof;
            if( obj->feof <= 0 && obj->feof != -2 ){ return -1; }
            return obj->feof;
        } else { SOCKADDR* cli = obj->srv==1 ? &obj->client_addr : &obj->server_addr;
            obj->feof = ::recvfrom( obj->fd, bf, sx, 0, cli, &obj->len );
            obj->feof = is_blocked(obj->feof)? -2 : obj->feof;
            if( obj->feof <= 0 && obj->feof != -2 ){ return -1; }
            return obj->feof;
        }   return -1;
    }

    virtual int __write( char* bf, const ulong& sx ) const noexcept {
        if ( process::millis() > get_send_timeout() || is_closed() )
           { return -1; } if ( sx==0 ) { return 0; }
        if ( SOCK != SOCK_DGRAM ){
            obj->feof = ::send( obj->fd, bf, sx, 0 );
            obj->feof = is_blocked(obj->feof)? -2 : obj->feof;
            if( obj->feof <= 0 && obj->feof != -2 ){ return -1; }
            return obj->feof;
        } else { SOCKADDR* cli = obj->srv==1 ? &obj->client_addr : &obj->server_addr;
            obj->feof = ::sendto( obj->fd, bf, sx, 0, cli, obj->len );
            obj->feof = is_blocked(obj->feof)? -2 : obj->feof;
            if( obj->feof <= 0 && obj->feof != -2 ){ return -1; }
            return obj->feof;
        }   return -1;
    }

    /*─······································································─*/

    bool _write_( char* bf, const ulong& sx, ulong& sy ) const noexcept {
        if( sx==0 || is_closed() ){ return 1; } while( sy < sx ) {
            int c = __write( bf+sy, sx-sy );
            if( c <= 0 && c != -2 ) /*----*/ { return 0; }
            if( c >  0 ){ sy += c; continue; } return 1;
        }   return 0;
    }

    bool _read_( char* bf, const ulong& sx, ulong& sy ) const noexcept {
        if( sx==0 || is_closed() ){ return 1; } while( sy < sx ) {
            int c = __read( bf+sy, sx-sy );
            if( c <= 0 && c != -2 ) /*----*/ { return 0; }
            if( c >  0 ){ sy += c; continue; } return 1;
        }   return 0;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/
