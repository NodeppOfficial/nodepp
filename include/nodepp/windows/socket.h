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
            process::NODEPP_SIGNAL().onSIGEXIT.once([=](){ WSACleanup(); });
        if( WSAStartup(MAKEWORD(2,2),&wsaData)!= 0 )
          { NODEPP_THROW_ERROR("Failed to initialize Winsock"); }
        }   sockets = true;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

struct ip_t    { string_t address; uint port; };
struct agent_t {
    ulong buffer_size   = NODEPP_CHUNK_SIZE;
    ulong conn_timeout  = 60000;
    ulong recv_timeout  = 0;
    ulong send_timeout  = 0;
    bool  reuse_address = 1;
    bool  no_delay_mode = 0;
    bool  reuse_port    = 1;
    bool  keep_alive    = 0;
    bool  broadcast     = 0;
    int   socket_family = AF_UNSPEC;
};

class socket_t {
protected:

    using TIMEVAL     = struct timeval;
    using SOCKADDR    = struct sockaddr;
    using SOCKADDR_IN = struct sockaddr_in;
    using SOCKADDR_IN6= struct sockaddr_in6;
    using SOCKADDR_ST = struct sockaddr_storage;

protected:

    void kill() const noexcept {
        obj->state |= STATE::FS_STATE_KILL;
    }

    SOCKADDR_ST& get_addr() const noexcept { 
        return is_server() ? obj->client_addr 
        /*--------------*/ : obj->server_addr; 
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
         FS_STATE_DISABLE = 0b00001110,
         FS_STATE_SERVER  = 0b10000000
    };

protected:

    struct NODE {

        ulong recv_timeout=0; 
        ulong send_timeout=0;
        ulong conn_timeout=0;
        ulong range[2] = { 0, 0 };

        WSAOVERLAPPED ovr, ovw ;
        SOCKET fd  = INVALID_SOCKET;
        SOCKET tmp = INVALID_SOCKET;
        SOCKADDR_ST server_addr, client_addr;

        int feof = 1, addrlen;
        LPFN_ACCEPTEX  lpfnAcceptEx  = nullptr;
        LPFN_CONNECTEX lpfnConnectEx = nullptr;
        
        uchar state    = STATE::FS_STATE_OPEN;
        char  addr_buf [(sizeof(SOCKADDR_IN) + 16) * 2];

        ptr_t<char> buffer; string_t borrow;
        generator::file::until _until;
        generator::file::line  _line ;
        generator::file::read  _read ;
        generator::file::write _write;

       ~NODE(){ if( fd == INVALID_SOCKET ){ return; }
            CancelIoEx   ((HANDLE) fd, &ovr);
            CancelIoEx   ((HANDLE) fd, &ovw);
            ::shutdown   ( fd , SD_RECEIVE ); 
            ::closesocket( fd /*--------*/ );
        }

    };  ptr_t<NODE> obj;

    /*─······································································─*/

    bool is_blocked( DWORD /*unused*/ ) const noexcept {
        DWORD err = GetLastError();
        return( err == ERROR_IO_INCOMPLETE || err == ERROR_IO_PENDING );
    }

    bool is_blocked( bool mode, DWORD& c ) const noexcept {
    auto ov = mode ? &obj->ovw : &obj->ovr;
    
        if( !HasOverlappedIoCompleted(ov) ) { return 1; }

        if( obj->state & ( STATE::FS_STATE_READING | STATE::FS_STATE_WRITING ) ){
        if( GetOverlappedResult((HANDLE)obj->fd, ov, &c, FALSE) )
          { goto DONE; }} else { goto DONE; }
        if( is_blocked( c ) )  { return 1 ; }
    
    DONE:; return 0; }

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
        if( time == 0 ){ obj->recv_timeout = 0; return 0; } TIMEVAL en; 
        en.tv_sec  =  time / 1000; 
        en.tv_usec = (time % 1000) * 1000;
        int c = setsockopt( obj->fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&en, sizeof(en) ); 
        obj->recv_timeout = process::millis() + time; 
        return c == 0 ? time : 0;
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

    expected_t<ip_t,except_t> get_sockname() const noexcept {
        SOCKADDR_ST addr; socklen_t len = sizeof(addr);

        if( is_closed() )
          { return except_t( "invalid socket" ); }

        if( getsockname( obj->fd, (SOCKADDR*)&addr, &len ) < 0 )
          { return except_t( "address not found" ); }

        char host[INET6_ADDRSTRLEN] = {0}; uint port;

        if( addr.ss_family == AF_INET ) {
            SOCKADDR_IN* s = (SOCKADDR_IN*)&addr;
            port = ntohs( ((SOCKADDR_IN*) &addr)->sin_port );
            inet_ntop( AF_INET, &s->sin_addr, host, sizeof(host) );
        } else {
            SOCKADDR_IN6* s = (SOCKADDR_IN6*)&addr;
            port = ntohs( ((SOCKADDR_IN6*) &addr)->sin6_port );
            inet_ntop( AF_INET6, &s->sin6_addr, host, sizeof(host) );
        }

        return ip_t({ host, port });
    }

    expected_t<ip_t,except_t> get_peername() const noexcept { 
        SOCKADDR_ST& addr = get_addr(); socklen_t len = sizeof(addr);

        if( is_closed() )
          { return except_t( "invalid socket" ); }

        if( getpeername( obj->fd, (SOCKADDR*) &addr, &len ) < 0 )
          { return except_t( "address not found" ); }

        char host[INET6_ADDRSTRLEN] = {0}; uint port;

        if( addr.ss_family == AF_INET ) {
            SOCKADDR_IN* s = (SOCKADDR_IN*)&addr;
            port = ntohs( ((SOCKADDR_IN*) &addr)->sin_port );
            inet_ntop( AF_INET, &s->sin_addr, host, sizeof(host) );
        } else {
            SOCKADDR_IN6* s = (SOCKADDR_IN6*)&addr;
            port = ntohs( ((SOCKADDR_IN6*) &addr)->sin6_port );
            inet_ntop( AF_INET6, &s->sin6_addr, host, sizeof(host) );
        }

        return ip_t({ host, port });
    }

    /*─······································································─*/

    ulong set_buffer_size( ulong _size ) const noexcept {
        set_send_buff( _size ); set_recv_buff( _size );
        obj->buffer = ptr_t<char>(_size); return _size;
    }

    /*─······································································─*/

    void set_client_address( SOCKADDR_ST address ) const noexcept {
         if( is_server() ){ obj->client_addr = address; }
         else /*-------*/ { obj->server_addr = address; }
    }

    SOCKADDR_ST get_client_address() const noexcept { 
        return is_server() ? obj->client_addr 
        /*--------------*/ : obj->server_addr; 
    }

    /*─······································································─*/

    ulong set_timeout( ulong time ) const noexcept {
        set_conn_timeout( time );
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
    bool    is_server() const noexcept { return obj->state & STATE::FS_STATE_SERVER; }
    bool      is_feof() const noexcept { return obj->feof <= 0 && obj->feof != -2; }
    bool   is_waiting() const noexcept { return obj->feof == -2; }
    bool is_available() const noexcept { return !is_closed(); }

    /*─······································································─*/

    void close() const noexcept {
        if( is_state ( STATE::FS_STATE_DISABLE ) ){ return; }
            onDrain.emit(); set_state( STATE::FS_STATE_CLOSE   );
    free(); }

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
        opt.socket_family = AF;
    return opt;
    }

    /*─······································································─*/

    socket_t( SOCKET fd, ulong _size=NODEPP_CHUNK_SIZE ) : obj( new NODE() ) { _socket_::start_device();
        if( fd == INVALID_SOCKET ){ NODEPP_THROW_ERROR("Such Socket has an Invalid fd"); }
        obj->fd = fd; set_nonbloking_mode(); set_buffer_size(_size);
    }

    virtual ~socket_t() noexcept { if( obj.count()>1 && !is_closed() ){ return; } free(); }

    socket_t() noexcept : obj( new NODE() ) { _socket_::start_device(); }

    /*─······································································─*/

    void free() const noexcept {

        if( is_state( STATE::FS_STATE_REUSE ) && !is_feof() && obj.count() >1 ){ return; }
        if( is_state( STATE::FS_STATE_KILL  ) ) /*-------*/ { return; }
        if(!is_state( STATE::FS_STATE_CLOSE | STATE::FS_STATE_REUSE ) ){ onDrain.emit(); }

        onUnpipe.clear(); onResume.clear();
        onError .clear(); onData  .clear(); kill();
        onOpen  .clear(); onPipe  .clear(); onClose.emit();

    }

    /*─······································································─*/

    virtual int socket( const string_t& host, int port ) const noexcept {
        if( host.empty() ){ onError.emit("invalid IP address"); return -1; } DWORD c = 0;

        if((obj->fd=WSASocketW( AF, SOCK, IPPROTO, NULL, 0, WSA_FLAG_OVERLAPPED )) == INVALID_SOCKET )
          { onError.emit("can't initializate socket fd"); return -1; }
        
        GUID GuidAcceptEx = WSAID_ACCEPTEX;
        WSAIoctl(obj->fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),
                &obj->lpfnAcceptEx, sizeof(obj->lpfnAcceptEx), &c, NULL, NULL);

        GUID GuidConnectEx = WSAID_CONNECTEX;
        WSAIoctl(obj->fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx),
                &obj->lpfnConnectEx, sizeof(obj->lpfnConnectEx), &c, NULL, NULL);

        set_buffer_size( NODEPP_CHUNK_SIZE );
        set_nonbloking_mode();
        set_reuse_address (1);

    #ifdef SO_REUSEPORT
        set_reuse_port(1);
    #endif

        SOCKADDR_ST server_st; memset(&server_st, 0, sizeof(SOCKADDR_ST));
        SOCKADDR_ST client_st; memset(&client_st, 0, sizeof(SOCKADDR_ST));

        if( AF == AF_INET6 ) { set_ipv6_only_mode(0);

            obj->addrlen    = sizeof( SOCKADDR_IN6 );
            SOCKADDR_IN6* s = (SOCKADDR_IN6*)&server_st;
            memset( s,0,sizeof(SOCKADDR_IN6));

            s->sin6_family  = AF_INET6; if( port>0 ){ s->sin6_port = htons(port); }

            if  ( host == "::0" || host == "global"    ){ s->sin6_addr = in6addr_any;      }
            elif( host == "::2" || host == "loopback"  ){ s->sin6_addr = in6addr_loopback; }
            elif( host == "::1" || host == "localhost" ){ inet_pton(AF, "::1", /*-*/ &s->sin6_addr); }
            else                                        { inet_pton(AF, host.c_str(),&s->sin6_addr); }

        } else {

            obj->addrlen   = sizeof(SOCKADDR_IN);
            SOCKADDR_IN* s = (SOCKADDR_IN*)&server_st;
            memset(s,0,sizeof(SOCKADDR_IN));

            s->sin_family  = AF_INET; if( port>0 ){ s->sin_port = htons(port); }

            if  ( host == "0.0.0.0"   || host == "global"    ){ s->sin_addr.s_addr = INADDR_ANY;       }
            elif( host == "1.1.1.1"   || host == "loopback"  ){ s->sin_addr.s_addr = INADDR_LOOPBACK;  }
            elif( host == "127.0.0.1" || host == "localhost" ){ inet_pton(AF, "127.0.0.1", &s->sin_addr); }
            else                                              { inet_pton(AF, host.c_str(),&s->sin_addr); }

        }

        obj->server_addr = server_st; 
        obj->client_addr = client_st;

    return 1; }

    /*─······································································─*/

    int _connect() const noexcept { 

        if( process::millis() > get_conn_timeout() )/**/{ return -1; }
        if( is_server() || obj->lpfnConnectEx==nullptr ){ return -1; } DWORD c=0;

        if( obj->state & STATE::FS_STATE_READING ){
        if( is_blocked( false, c ) ){ return -2; }
            int c = ::setsockopt( obj->fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 ); 
            obj->state &=~ STATE::FS_STATE_READING; return c==0 ? 1 : -1;
        }

        memset( &obj->ovr, 0, sizeof(WSAOVERLAPPED) ); obj->state|= STATE::FS_STATE_READING;
        SOCKADDR_IN any     = {0}; 
        any.sin_addr.s_addr = INADDR_ANY;
        any.sin_family      = AF; any.sin_port = 0;
        ::bind( obj->fd, (SOCKADDR*)&any, sizeof(any) );
        
        if( obj->lpfnConnectEx( obj->fd, (SOCKADDR*) &obj->server_addr, obj->addrlen, NULL, 0, &c, &obj->ovr ) ){
            int c = ::setsockopt( obj->fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 ); 
            obj->state &=~ STATE::FS_STATE_READING; return c==0 ? 1 : -1;
        } elif( is_blocked( c ) ) { return -2; } 
    
    return -1; }

    int _accept() const noexcept { 

        if( !is_server() || obj->lpfnAcceptEx==nullptr ){ return -1; } DWORD c=0;

        if( obj->state & STATE::FS_STATE_WRITING ){
        if( is_blocked( true, c ) ){ return -2; }
            int c = ::setsockopt( obj->tmp, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&obj->fd, sizeof(SOCKET) );
            c = c==0 ? (int) obj->tmp : INVALID_SOCKET; obj->tmp = INVALID_SOCKET; 
            obj->state &=~ STATE::FS_STATE_WRITING; return c; 
        }

        memset( &obj->ovw, 0, sizeof(WSAOVERLAPPED) ); obj->state|= STATE::FS_STATE_WRITING;
        if( obj->tmp == INVALID_SOCKET )
          { obj->tmp = WSASocketW( AF, SOCK, IPPROTO, NULL, 0, WSA_FLAG_OVERLAPPED ); }

        if( obj->lpfnAcceptEx( obj->fd, obj->tmp, obj->addr_buf, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &c, &obj->ovw ) ){
            int c = ::setsockopt( obj->tmp, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&obj->fd, sizeof(SOCKET) );
            c = c==0 ? (int) obj->tmp : INVALID_SOCKET; obj->tmp = INVALID_SOCKET; 
            obj->state &=~ STATE::FS_STATE_WRITING; return c; 
        } elif( is_blocked( c ) ) { return -2; } 
        
    return -1; }

    /*─······································································─*/

    int listen() const noexcept { if( !is_server() ){ return -1; }
        return ::listen( obj->fd, NODEPP_MAX_SOCKET ) ?-1: 1;
    }

    int accept() const noexcept { int c=0;
        while((c=_accept()) == -2 ){ process::next(); } return c;
    }

    int connect() const noexcept { int c=0;
        while((c=_connect()) == -2 ){ process::next(); } return c;
    }

    int bind() const noexcept { obj->state |= STATE::FS_STATE_SERVER;
        return ::bind( obj->fd, (SOCKADDR*) &obj->server_addr, obj->addrlen ) ?-1: 1;
    }

    /*─······································································─*/

    string_t read( ulong size=NODEPP_CHUNK_SIZE ) const noexcept {
        while( obj->_read( this, size ) == 1 )
             { process::next(); }
        return obj->_read.data;
    }

    char read_char() const noexcept { return read(1)[0]; }

    ulong write( const string_t& msg ) const noexcept {
        while( obj->_write( this, msg ) == 1 )
             { process::next(); }
        return obj->_write.data;
    }

    /*─······································································─*/

    string_t read_until( string_t ch ) const noexcept {
        while( obj->_until( this, ch ) == 1 )
             { process::next(); }
        return obj->_until.data;
    }

    string_t read_until( char ch ) const noexcept {
        while( obj->_until( this, ch ) == 1 )
             { process::next(); }
        return obj->_until.data;
    }

    string_t read_line() const noexcept {
        while( obj->_line( this ) == 1 )
             { process::next(); }
        return obj->_line.data;
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
            obj->state&=~ STATE::FS_STATE_READING;
            obj->feof  = c==0 ? -1 : (int) c; 
        return obj->feof; }

        SOCKADDR_ST& addr = get_addr(); socklen_t len = sizeof(addr);
        memset( &obj->ovr, 0, sizeof(WSAOVERLAPPED) );
        obj->state|= STATE::FS_STATE_READING;
        WSABUF wbuf={ sx, bf }; 

        int res = SOCK != SOCK_DGRAM
                ? WSARecv    ( obj->fd, &wbuf, 1, &c, &f, &obj->ovr , NULL )
                : WSARecvFrom( obj->fd, &wbuf, 1, &c, &f, (SOCKADDR*) &addr, &len, &obj->ovr, NULL );

        if( res==0 ) {
            obj->state&=~ STATE::FS_STATE_READING;
            obj->feof  = c==0 ? -1: (int) c;
        } elif( is_blocked( c ) ) { obj->feof = -2; return -2; } 

    return ( obj->feof <= 0 && obj->feof != -2 ) ? -1 : obj->feof; }

    virtual int __write( char* bf, const ulong& sx ) const noexcept {
        if( process::millis() > get_send_timeout() || is_closed() )
          { return -1; } if ( sx==0 ) { return 0; } DWORD c=0, f=0; 

        if( obj->state & STATE::FS_STATE_WRITING ){
        if( is_blocked( true, c ) ){ return -2; }
            obj->state&=~ STATE::FS_STATE_WRITING;
            obj->feof  = c==0 ? -1 : (int) c; 
        return obj->feof; }

        SOCKADDR_ST& addr = get_addr(); socklen_t len = sizeof(addr);
        memset( &obj->ovw, 0, sizeof(WSAOVERLAPPED) );
        obj->state |= STATE::FS_STATE_WRITING;
        WSABUF wbuf ={ sx, bf };

        int res = SOCK != SOCK_DGRAM
                ? WSASend  ( obj->fd, &wbuf, 1, &c, 0, &obj->ovw , NULL )
                : WSASendTo( obj->fd, &wbuf, 1, &c, 0, (SOCKADDR*) &addr, len, &obj->ovw, NULL );

        if( res==0 ) {
            obj->state&=~ STATE::FS_STATE_WRITING;
            obj->feof  = c==0 ? -1: (int) c;
        } elif( is_blocked( c ) ) { obj->feof = -2; return -2; } 

    return ( obj->feof <= 0 && obj->feof != -2 ) ? -1 : obj->feof; }

    /*─······································································─*/

    int _write_( char* bf, const ulong& sx, ulong* sy ) const noexcept {
        if( sx==0 || is_closed() ){ return -1; } while( *sy<sx ) {
            int c = __write( bf + *sy, sx - *sy );
            if( c <= 0 && c != -2 ) /*----*/ { return -2; }
            if( c >  0 ){ *sy+= c; continue; } break/**/;
        }   return sx;
    }

    int _read_( char* bf, const ulong& sx, ulong* sy ) const noexcept {
        if( sx==0 || is_closed() ){ return -1; } while( *sy<sx ) {
            int c = __read( bf + *sy, sx - *sy );
            if( c <= 0 && c != -2 ) /*----*/ { return -2; }
            if( c >  0 ){ *sy+= c; continue; } break/**/;
        }   return sx;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/