/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_SOCKET
#define NODEPP_WINDOWS_SOCKET

/*────────────────────────────────────────────────────────────────────────────*/

#include <ws2tcpip.h>
#include <winsock2.h>
#include <winsock.h>
#include <mswsock.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace _socket_ {

    inline void start_device(){ 
    static bool sockets=false;
        if( sockets == false ){ WSADATA wsaData;
            process::onSIGEXIT.once([=](){ WSACleanup(); });
        if( WSAStartup(MAKEWORD(2,2),&wsaData)!= 0 )
          { throw except_t("Failed to initialize Winsock"); }
        }   sockets = true;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

struct agent_t {
    ulong buffer_size   = CHUNK_SIZE;
    uint  conn_timeout  = 0;//10000 ;
    uint  recv_timeout  = 0;//120000;
    uint  send_timeout  = 0;//120000;
    bool  reuse_address = 1;
    bool  no_delay_mode = 0;
    bool  reuse_port    = 1;
    bool  keep_alive    = 0;
    bool  broadcast     = 0;
};

class socket_t {
protected:

    void kill() const noexcept {
        CancelIoEx((HANDLE)obj->fd, &obj->ovr);
        CancelIoEx((HANDLE)obj->fd, &obj->ovw);
        obj->state |= STATE::FS_STATE_KILL;
        ::shutdown( obj->fd, SD_BOTH ); 
        ::closesocket( obj->fd );
    }

    bool is_state( uchar value ) const noexcept {
        if( obj->state & value ){ return true; }
    return false; }

    void set_state( uchar value ) const noexcept {
    if( obj->state & STATE::FS_STATE_KILL ){ return; }
        obj->state = value;
    }

    enum STATE {
         FS_STATE_UNKNOWN = 0b00000000,
         FS_STATE_OPEN    = 0b00000001,
         FS_STATE_CLOSE   = 0b00000010,
         FS_STATE_READING = 0b00010000,
         FS_STATE_WRITING = 0b00100000,
         FS_STATE_KILL    = 0b00000100,
         FS_STATE_REUSE   = 0b00001000,
         FS_STATE_DISABLE = 0b00001110
    };

protected:

    using TIMEVAL     = struct timeval;
    using SOCKADDR    = struct sockaddr;
    using SOCKADDR_IN = struct sockaddr_in;

    struct NODE {

        ulong recv_timeout=0; 
        ulong send_timeout=0;
        ulong conn_timeout=0;
        ulong range[2] = { 0, 0 };

        WSAOVERLAPPED ovr, ovw ;
        SOCKET fd   =INVALID_SOCKET;
        SOCKET tmp  =INVALID_SOCKET;
        SOCKADDR server_addr, client_addr;

        LPFN_ACCEPTEX  lpfnAcceptEx  = nullptr;
        LPFN_CONNECTEX lpfnConnectEx = nullptr;
        int feof = 1, addrlen, len; bool srv=0;
        
        uchar state    = STATE::FS_STATE_OPEN;
        char  addr_buf [(sizeof(SOCKADDR_IN) + 16) * 2];

        ptr_t<char> buffer; string_t borrow;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    bool is_blocked( bool mode, DWORD& c ) const noexcept {
    auto state = mode ? STATE::FS_STATE_WRITING : STATE::FS_STATE_READING;
    auto    ov = mode ? &obj->ovw : &obj->ovr;

        if( obj->state & state ){
        if( GetOverlappedResult((HANDLE)obj->fd, ov, &c, FALSE) )
          { goto DONE; }} else { goto DONE; }

        do{ DWORD err = WSAGetLastError();
        if( err == ERROR_IO_INCOMPLETE || err == WSA_IO_PENDING )
          { return 1; }} while(0);

    DONE:; obj->state &= ~state;
    return 0; }

    bool is_blocked( int& c ) const noexcept {
    if( c >= 0 ){ return 0; } auto error =  WSAGetLastError();
    if( error == WSAEISCONN ){ c=0; return 0; } return (
        error == WSAEWOULDBLOCK || error == WSAEINPROGRESS ||
        error == WSAEALREADY    || error == WSAECONNRESET
    );}

    /*─······································································─*/

    int set_nonbloking_mode() const noexcept { return 0; }

public:

    event_t<>          onUnpipe;
    event_t<>          onResume;
    event_t<except_t>  onError;
    event_t<>          onDrain;
    event_t<>          onClose;
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
        obj->conn_timeout = process::millis() + time; 
        return time;
    }

    ulong set_recv_timeout( ulong time ) const noexcept {
        if( time == 0 ){ obj->recv_timeout = 0; return 0; }
        TIMEVAL en; memset( &en, 0, sizeof(en) ); en.tv_sec = time / 1000; en.tv_usec = 0;
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&en, sizeof(en) ); 
        obj->recv_timeout = process::millis() + time; return c==0 ? time : 0;
    }

    ulong set_send_timeout( ulong time ) const noexcept {
        if( time == 0 ){ obj->send_timeout = 0; return 0; }
        TIMEVAL en; memset( &en, 0, sizeof(en) ); en.tv_sec = time / 1000; en.tv_usec = 0;
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&en, sizeof(en) ); 
        obj->send_timeout = process::millis() + time; return c==0 ? time : 0;
    }

    /*─······································································─*/

    int set_no_delay_mode( uint en ) const noexcept { if( IPPROTO != IPPROTO_TCP ){ return -1; }
    int c= setsockopt( obj->fd, IPPROTO, TCP_NODELAY, (char*)&en, sizeof(en) ); 
        return c;
    }

    int set_recv_buff( uint en ) const noexcept {
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_RCVBUF, (char*)&en, sizeof(en) ); 
        return c;
    }

    int set_send_buff( uint en ) const noexcept {
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_SNDBUF, (char*)&en, sizeof(en) );
        return c;
    }

    int set_accept_connection( uint en ) const noexcept {
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_ACCEPTCONN, (char*)&en, sizeof(en) ); 
        return c;
    }

    int set_dont_route( uint en ) const noexcept {
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_DONTROUTE, (char*)&en, sizeof(en) );
        return c;
    }

    int set_keep_alive( uint en ) const noexcept {
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&en, sizeof(en) ); 
        return c;
    }

    int set_broadcast( uint en ) const noexcept {
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_BROADCAST, (char*)&en, sizeof(en) ); 
        return c;
    }

    int set_reuse_address( uint en ) const noexcept {
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_REUSEADDR, (char*)&en, sizeof(en) ); 
        return c;
    }

    int set_ipv6_only_mode( uint en ) const noexcept {
    int c= setsockopt( obj->fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&en, sizeof(en) ); 
        return c;
    }

#ifdef SO_REUSEPORT
    int set_reuse_port( uint en ) const noexcept {
    int c= setsockopt( obj->fd, SOL_SOCKET, SO_REUSEPORT, (char*)&en, sizeof(en) );
        return c;
    }
#endif

    int get_no_delay_mode() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt( obj->fd, IPPROTO, TCP_NODELAY, (char*)&en, &size ); 
        return c==0 ? en : c;
    }

    int get_error() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, SOL_SOCKET, SO_ERROR, (char*)&en, &size); 
        return c==0 ? en : c;
    }

    int get_recv_buff() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, SOL_SOCKET, SO_RCVBUF, (char*)&en, &size); 
        return c==0 ? en : c;
    }

    int get_send_buff() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, SOL_SOCKET, SO_SNDBUF, (char*)&en, &size); 
        return c==0 ? en : c;
    }

    int get_accept_connection() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, SOL_SOCKET, SO_ACCEPTCONN, (char*)&en, &size); 
        return c==0 ? en : c;
    }

    int get_dont_route() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, SOL_SOCKET, SO_DONTROUTE, (char*)&en, &size); 
        return c==0 ? en : c;
    }

    int get_reuse_address() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, SOL_SOCKET, SO_REUSEADDR, (char*)&en, &size);
        return c==0 ? en : c;
    }

#ifdef SO_REUSEPORT
    int get_reuse_port() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, SOL_SOCKET, SO_REUSEPORT, (char*)&en, &size);
        return c==0 ? en : c;
    }
#endif

    int get_ipv6_only_mode() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&en, &size);
        return c==0 ? en : c;
    }

    int get_keep_alive() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&en, &size);
        return c==0 ? en : c;
    }

    int get_broadcast() const noexcept { int en; socklen_t size = sizeof(en);
    int c= getsockopt(obj->fd, SOL_SOCKET, SO_BROADCAST, (char*)&en, &size);
        return c==0 ? en : c;
    }

    /*─······································································─*/

    SOCKADDR* get_addr() const noexcept { return obj->srv==1 ? &obj->client_addr : &obj->server_addr; }

    string_t get_sockname() const noexcept { SOCKADDR* cli = get_addr();
    int c= getsockname( obj->fd, cli, &obj->len ); string_t buff { INET_ADDRSTRLEN };
        inet_ntop( AF, &(((SOCKADDR_IN*)cli)->sin_addr), (char*)buff, buff.size() );
        return c < 0 ? "127.0.0.1" : buff;
    }

    string_t get_peername() const noexcept { SOCKADDR* cli = get_addr();
    int c= getpeername( obj->fd, cli, &obj->len ); string_t buff { INET_ADDRSTRLEN };
        inet_ntop( AF, &(((SOCKADDR_IN*)cli)->sin_addr), (char*)buff, buff.size() );
        return c < 0 ? "127.0.0.1" : buff;
    }

    int get_sockport() const noexcept { SOCKADDR* cli = get_addr();
        return ntohs( ((SOCKADDR_IN*)cli)->sin_port );
    }

    /*─······································································─*/

    ulong set_buffer_size( ulong _size ) const noexcept {
        set_send_buff( _size ); set_recv_buff( _size );
        obj->buffer = ptr_t<char>(_size); return _size;
    }

    /*─······································································─*/

    ulong set_timeout( ulong time ) const noexcept {
        set_recv_timeout( time );
        set_send_timeout( time ); return time;
    }

    /*─······································································─*/

    void  resume() const noexcept { if(is_state(STATE::FS_STATE_OPEN )){ return; } set_state(STATE::FS_STATE_OPEN ); onResume.emit(); }
    void    stop() const noexcept { if(is_state(STATE::FS_STATE_REUSE)){ return; } set_state(STATE::FS_STATE_REUSE); onDrain .emit(); }
    void   reset() const noexcept { if(is_state(STATE::FS_STATE_KILL )){ return; } resume(); pos(0); }
    void   flush() const noexcept { obj->buffer.fill(0); }

    /*─······································································─*/

    bool    is_closed() const noexcept { return is_state(STATE::FS_STATE_DISABLE) || is_feof() || obj->fd==INVALID_SOCKET; }
    bool      is_feof() const noexcept { return obj->feof <= 0 && obj->feof != -2; }
    bool   is_waiting() const noexcept { return obj->feof == -2; }
    bool is_available() const noexcept { return !is_closed(); }
    bool    is_server() const noexcept { return obj->srv;  }

    /*─······································································─*/

    void close() const noexcept {
        if( is_state ( STATE::FS_STATE_DISABLE ) ){ return; }
            set_state( STATE::FS_STATE_CLOSE   );
    onDrain.emit(); free(); }

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

    socket_t() noexcept : obj( new NODE() ) { _socket_::start_device(); }

   ~socket_t() noexcept { if( obj.count()>1 && !is_closed() ){ return; } free(); }

    /*─······································································─*/

    void free() const noexcept {

        if( is_state( STATE::FS_STATE_REUSE ) && obj.count() > 1 ){ return; }
        if( is_state( STATE::FS_STATE_KILL  ) ) /*-------------*/ { return; }
        if(!is_state( STATE::FS_STATE_CLOSE | STATE::FS_STATE_REUSE ) )
          { kill(); onDrain.emit(); } else { kill(); }

        onUnpipe.clear(); onResume.clear();
        onError .clear(); onData  .clear(); 
        onOpen  .clear(); onPipe  .clear(); onClose.emit();

    }

    /*─······································································─*/

    virtual int socket( const string_t& host, int port ) const noexcept {
        if( host.empty() ){ onError.emit("dns coudn't found ip"); return -1; }
            obj->addrlen = sizeof( obj->server_addr ); DWORD c = 0;

        obj->fd = WSASocketW( AF, SOCK, IPPROTO, NULL, 0, WSA_FLAG_OVERLAPPED );

        if( obj->fd == INVALID_SOCKET )
          { onError.emit("can't initializate socket fd"); return -1; }
        
        GUID GuidAcceptEx = WSAID_ACCEPTEX;
        WSAIoctl(obj->fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),
                &obj->lpfnAcceptEx, sizeof(obj->lpfnAcceptEx), &c, NULL, NULL);

        GUID GuidConnectEx = WSAID_CONNECTEX;
        WSAIoctl(obj->fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx),
                &obj->lpfnConnectEx, sizeof(obj->lpfnConnectEx), &c, NULL, NULL);

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

    inline int _connect() const noexcept { 
        if( process::millis() > get_conn_timeout() || obj->srv==1 || obj->lpfnConnectEx==nullptr )
          { return -1; } DWORD c=0;

        if( obj->state & STATE::FS_STATE_READING ){
        if( is_blocked( false, c ) ){ return -2; }
            obj->feof  = ::setsockopt( obj->fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 )==0 ? 1 : -1; 
        return 1; }

        SOCKADDR_IN any     = {0}; 
        any.sin_addr.s_addr = INADDR_ANY;
        any.sin_family      = AF; any.sin_port = 0;
        ::bind( obj->fd, (SOCKADDR*)&any, sizeof(any) );

        memset( &obj->ovr, 0, sizeof(WSAOVERLAPPED) );
        obj->state|= STATE::FS_STATE_READING;
        
        obj->feof = obj->lpfnConnectEx( obj->fd, &obj->server_addr, obj->addrlen, NULL, 0, &c, &obj->ovr );
        obj->feof = is_blocked( false, c ) ? -2 : 1; if( obj->feof == -2 ){ return -2; }
        obj->feof = ::setsockopt( obj->fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 )==0 ? 1 : -1; 
    
    return 1; }

    inline int _accept() const noexcept { 
        if( obj->srv==0 || obj->lpfnAcceptEx==nullptr )
          { return -1; } DWORD c=0; 

        if( obj->state & STATE::FS_STATE_WRITING ){
        if( is_blocked( true, c ) ){ return -2; }   SOCKET nfd = obj->tmp; obj->tmp = INVALID_SOCKET;
            obj->feof  = ::setsockopt( nfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&obj->fd, sizeof(SOCKET) )==0 ? 1 : -1;
        return (int)nfd; }

        if( obj->tmp == INVALID_SOCKET )
          { obj->tmp = WSASocketW( AF, SOCK, IPPROTO, NULL, 0, WSA_FLAG_OVERLAPPED ); }

        memset( &obj->ovw, 0, sizeof(WSAOVERLAPPED) );
        obj->state|= STATE::FS_STATE_WRITING;
        
        obj->feof = obj->lpfnAcceptEx( obj->fd, obj->tmp, obj->addr_buf, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &c, &obj->ovw );
        obj->feof = is_blocked( true, c ) ? -2 : 1; if( obj->feof == -2 ){ return -2; } SOCKET nfd= obj->tmp; obj->tmp = INVALID_SOCKET;
        obj->feof = ::setsockopt( nfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&obj->fd, sizeof(SOCKET) )==0 ? 1 : -1; 
        
    return (int)nfd; }

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
        while((c=_connect()) == -2 ){ process::next(); } return c;
    }

    int listen() const noexcept { int c=0;
        while((c=_listen()) == -2 ){ process::next(); } return c;
    }

    int accept() const noexcept { int c=0;
        while((c=_accept()) == -2 ){ process::next(); } return c;
    }

    int bind() const noexcept { int c=0;
        while((c=_bind()) == -2 ){ process::next(); } return c;
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
        if( process::millis() > get_recv_timeout() || is_closed() )
          { return -1; } if ( sx==0 ) { return 0; } DWORD c=0, f=0; 

        if( obj->state & STATE::FS_STATE_READING ){
        if( is_blocked( false, c ) ){ return -2; }
            obj->feof = (int)c; return obj->feof; 
        }

        memset( &obj->ovr, 0, sizeof(WSAOVERLAPPED) );
        obj->state|= STATE::FS_STATE_READING;
        WSABUF wbuf ={ sx, bf }; 

        if( SOCK != SOCK_DGRAM ){
            obj->feof = WSARecv( obj->fd, &wbuf, 1, &c, &f, &obj->ovr, NULL );
            obj->feof = is_blocked( false, c )? -2 : (int) c;
        } else {
            obj->feof = WSARecvFrom( obj->fd, &wbuf, 1, &c, &f, get_addr(), &obj->len, &obj->ovr, NULL );
            obj->feof = is_blocked( false, c )? -2 : (int) c;
        }

    if( obj->feof <= 0 && obj->feof != -2 ){ return -1; }
    return obj->feof; }

    virtual int __write( char* bf, const ulong& sx ) const noexcept {
        if( process::millis() > get_send_timeout() || is_closed() )
          { return -1; } if ( sx==0 ) { return 0; } DWORD c=0, f=0; 

        if( obj->state & STATE::FS_STATE_WRITING ){
        if( is_blocked( true, c ) ){ return -2; }
            obj->feof = (int)c; return obj->feof; 
        }

        memset( &obj->ovw, 0, sizeof(WSAOVERLAPPED) );
        obj->state|= STATE::FS_STATE_WRITING;
        WSABUF wbuf ={ sx, bf };

        if( SOCK != SOCK_DGRAM ){
            obj->feof = WSASend( obj->fd, &wbuf, 1, &c, 0, &obj->ovw, NULL );
            obj->feof = is_blocked( true, c )? -2 : (int) c;
        } else {
            obj->feof = WSASendTo( obj->fd, &wbuf, 1, &c, 0, get_addr(), obj->len, &obj->ovw, NULL );
            obj->feof = is_blocked( true, c )? -2 : (int) c;
        }  
    
    if( obj->feof <= 0 && obj->feof != -2 ){ return -1; }
    return obj->feof; }

    /*─······································································─*/

    bool _write_( char* bf, const ulong& sx, ulong* sy ) const noexcept {
        if( sx==0 || is_closed() ){ return 1; } while( *sy<sx ) {
            int c = __write( bf + *sy, sx - *sy );
            if( c <= 0 && c != -2 ) /*----*/ { return 0; }
            if( c >  0 ){ *sy+= c; continue; } return 1;
        }   return 0;
    }

    bool _read_( char* bf, const ulong& sx, ulong* sy ) const noexcept {
        if( sx==0 || is_closed() ){ return 1; } while( *sy<sx ) {
            int c = __read( bf + *sy, sx - *sy );
            if( c <= 0 && c != -2 ) /*----*/ { return 0; }
            if( c >  0 ){ *sy+= c; continue; } return 1;
        }   return 0;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/