/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_IOCP
#define NODEPP_WINDOWS_IOCP

/*────────────────────────────────────────────────────────────────────────────*/

#include <windows.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <winsock.h>
#include <mswsock.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class iocp_t {
protected:

    using IOFD = OVERLAPPED_ENTRY;
    
    /*─······································································─*/

    enum FLAG : uchar {
         IOCP_FLAG_NULL = 0b00000000,
         IOCP_FLAG_FREE = 0b00000001,
         IOCP_FLAG_USED = 0b00000010,
         IOCP_FLAG_DONE = 0b00000100,
         IOCP_FLAG_WRTE = 0b00000001,
         IOCP_FLAG_READ = 0b00000000
    };
    
    /*─······································································─*/

    struct DONE { int mode; WSAOVERLAPPED ov; WSABUF buf; DWORD c,f; HANDLE fd; string_t mem; };
    struct DENO { DONE data[2]; void* addr; };
    struct NODE { HANDLE pd;

        queue_t<void*> borrow;
        queue_t<DENO > que   ;
        ptr_t  <IOFD > ev    ;

       ~NODE(){ CloseHandle( pd ); }
 
    }; ptr_t<NODE> obj;
    
    /*─······································································─*/

    bool is_blocked( DWORD /*unused*/ ) const noexcept {
        DWORD err = GetLastError();
        return( err == ERROR_IO_INCOMPLETE || 
                err == ERROR_IO_PENDING    || 
                err == WAIT_TIMEOUT        ||
                err == WSAEWOULDBLOCK      ||
                err == WSAEINPROGRESS
        );
    }

    bool is_blocked( HANDLE fd, OVERLAPPED& ov, DWORD& c ) const noexcept { do {
    
        if( !HasOverlappedIoCompleted(&ov) ) /*----*/ { return 1; }
        if( GetOverlappedResult( fd, &ov, &c, FALSE) ){ break   ; }
        if( is_blocked( c ) ) /*-------------------*/ { return 1; }

    } while(0); return 0; }
    
    /*─······································································─*/

    template< class T >
    int io_consumer( T* raw, int flag ) const noexcept {
    if( raw==nullptr ){ return -1; }

        auto &mem = raw->data.data[ flag ];
        auto &ov  = mem.ov;
        auto &c   = mem.c ;
        auto &fd  = mem.fd;
    
    if( mem.mode & FLAG::IOCP_FLAG_FREE ){ return -1; }
    if( !is_blocked ( fd, ov, c ) ){
        mem.mode = FLAG::IOCP_FLAG_FREE; return (int) c;
    }   
    
    return -2; }

    /*─······································································─*/

    template< class T >
    void* io_submitter( T* fd, int /*unused*/ ) const noexcept {

        void* raw = (void*) fd->get_pd();
        auto  mem = obj->que.as( raw );

    if( mem!=nullptr ) { return mem; } else {

        obj->que.push({ DENO({0}) });

        auto &mem     = /*-----*/ obj->que.last()->data;
        fd ->get_pd() = (uchar_64)obj->que.last();

        mem.data[0].mem.resize( NODEPP_CHUNK_SIZE );
        mem.data[1].mem.resize( NODEPP_CHUNK_SIZE );
        
        mem.addr         = nullptr;
        mem.data[0].mode = FLAG::IOCP_FLAG_FREE;
        mem.data[1].mode = FLAG::IOCP_FLAG_FREE;

    } return io_submitter( fd, 0x00 ); }

public:

    iocp_t() : obj( 0UL ) {}

    /*─······································································─*/

    int start_device() const { do {
    if( obj->pd == NULL ){

        obj->ev . resize( NODEPP_MAX_BATCH_SIZE );
        obj->pd = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
        if( obj->pd == NULL ){ NODEPP_THROW_ERROR("Can't Initialize iocp_t"); }

    return 1; }} while(0); return -1; }
    
    /*─······································································─*/

    HANDLE get_fd() const noexcept { return obj->pd; }

    int wake() const noexcept {
        return PostQueuedCompletionStatus( obj->pd, 0, 0, NULL ) ? 1 : -1;
    }
    
    /*─······································································─*/

    template< class T >
    void* append( T* fd ) const noexcept { 
        if( CreateIoCompletionPort( (HANDLE) fd->get_fd(), obj->pd, fd->get_pd(), 0 ) ) 
          { return io_submitter( fd, 0x00 ); } return nullptr;
    }
    
    int append( uchar_64 pd, void* address ) const noexcept {
        auto mem = obj->que.as( (void*) pd );
    if( mem == nullptr ){ return -1; }
        mem->data.addr = address;
    return 1; }

    void* address( uchar_64 pd ) const noexcept {
        auto mem = obj->que.as( (void*) pd );
    if( mem == nullptr ){ return nullptr; }
        return mem->data.addr; 
    }

    int free( HANDLE fd, uchar_64 pd ) const noexcept {
        auto mem = obj->que.as( (void*) pd );
    if( mem == nullptr ){ return -1; }

        CancelIoEx( fd,&mem->data.data[0].ov );
        CancelIoEx( fd,&mem->data.data[1].ov ); 
        obj->que.erase( mem );

    return 1; }

    int remove( uchar_64 pd ) const noexcept {
        auto mem = obj->que.as( (void*) pd );
    if( mem == nullptr ){ return -1; }
        mem->data.addr = nullptr;
    return 1; }

    /*─······································································─*/

    ptr_t<IOFD> next( int ms ) const noexcept {

        ULONG idx; int res = GetQueuedCompletionStatusEx( 
            obj->pd , obj->ev.data(), obj->ev.size(), 
            &idx, ms, FALSE
        );

        if( !res ){ return nullptr; }
        return ptr_t<IOFD>( obj->ev, 0, idx );
        
    }
    
    /*─······································································─*/

    template< class T >
    int write( T* fd, void* buf, DWORD len, DWORD* offset ) const noexcept {
        
        auto flag = FLAG::IOCP_FLAG_WRTE;
        auto io   = io_submitter( fd, flag );
        auto raw  = obj->que.as ( io );
        auto &mem = raw->data.data  [ flag ];
        auto size = min( len, (DWORD) NODEPP_CHUNK_SIZE );
        
    if( mem.mode == FLAG::IOCP_FLAG_FREE ){
        mem.mode  = FLAG::IOCP_FLAG_USED ;

        auto &ov  = mem.ov ; ov = {0};
        auto &c   = mem.c  ; c  = 0UL;
        auto &hd  = mem.fd ; hd = fd->get_fd();
        auto &raw = mem.mem; ov.Offset=offset[0];

        memcpy( raw.get(), buf, size );
        WriteFile( fd->get_fd(), raw.get(), size, &c, &ov );

        if( is_blocked(c) ) { return -2; }
        mem.mode = FLAG::IOCP_FLAG_FREE; offset[0] += c; return c;

    } int out = io_consumer( raw, flag ); switch( out<=0 ){

        case true : return out; break; case false: do {
             offset[0] +=  out; return out;
        } while(0); break;

    }}

    template< class T >
    int read( T* fd, void* buf, DWORD len, DWORD* offset ) const noexcept {
        
        auto flag = FLAG::IOCP_FLAG_READ;
        auto io   = io_submitter( fd, flag );
        auto raw  = obj->que.as ( io );
        auto &mem = raw->data.data  [ flag ];
        auto size = min( len, (DWORD) NODEPP_CHUNK_SIZE );
        
    if( mem.mode == FLAG::IOCP_FLAG_FREE ){
        mem.mode  = FLAG::IOCP_FLAG_USED ;

        auto &ov  = mem.ov ; ov = {0};
        auto &c   = mem.c  ; c  = 0UL;
        auto &hd  = mem.fd ; hd = fd->get_fd();
        auto &raw = mem.mem; ov.Offset=offset[0];

        ReadFile( fd->get_fd(), raw.get(), size, &c, &ov );

        if( is_blocked(c) ) { return -2; }
        mem.mode = FLAG::IOCP_FLAG_FREE;
        memmove( buf, raw.get(), c ); offset[0] += c; return c;

    } int out = io_consumer( raw, flag ); switch( out<=0 ){

        case true : return out; break; case false: do {

            string_t &raw = mem.mem; offset[0] += out; 
            memmove( buf, raw.get(), out );

        return out; } while(0); break;

    }}
    
    /*─······································································─*/

    template< class T, class V >
    int connect( T* fd, V* addr, DWORD len, int addr_family ) const noexcept {
        
        auto flag = FLAG::IOCP_FLAG_WRTE;
        auto io   = io_submitter( fd, flag );
        auto raw  = obj->que.as ( io );
        auto &mem = raw->data.data  [ flag ];
        
    if( mem.mode == FLAG::IOCP_FLAG_FREE ){
        mem.mode  = FLAG::IOCP_FLAG_USED ;

        auto &hd  = mem.fd ; hd = (HANDLE) fd->get_fd();
        auto &ov  = mem.ov ; ov = {0};
        auto &c   = mem.c  ; c  = 0UL; 
        auto &raw = mem.mem; LPFN_CONNECTEX connect = nullptr;

        GUID GuidConnectEx = WSAID_CONNECTEX; WSAIoctl( fd->get_fd(), 
             SIO_GET_EXTENSION_FUNCTION_POINTER, 
            &GuidConnectEx, sizeof(GuidConnectEx),
            &connect, sizeof(connect), &c, NULL, NULL
        );

        SOCKADDR_IN any     = {0}; 
        any.sin_addr.s_addr = INADDR_ANY ;
        any.sin_family      = addr_family; any.sin_port = 0;
        ::bind( fd->get_fd(), (SOCKADDR*)&any, sizeof(any) );

        if( connect( fd->get_fd(), addr, len, NULL, 0, &c, &ov ) ){
            int c=::setsockopt( fd->get_fd(), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 ); 
            mem.mode = FLAG::IOCP_FLAG_FREE; return c==0 ? 1: -1;
        } elif( is_blocked(c) ) { return -2; } 
        
            mem.mode = FLAG::IOCP_FLAG_FREE; return -1;

    } int out = io_consumer( raw, flag ); switch( out<0 ){

        case true : return out; break; case false: do {

            int c=::setsockopt( fd->get_fd(), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 ); 
            return c==0 ? 1 : -1; 
        
        } while(0); break;

    }}

    template< class T, class V >
    int accept( T* fd, V* addr, socklen_t* addr_len, int addr_family, int addr_sock, int addr_ipproto ) const noexcept {
        
        auto flag = FLAG::IOCP_FLAG_WRTE;
        auto io   = io_submitter( fd, flag );
        auto raw  = obj->que.as ( io );
        auto &mem = raw->data.data  [ flag ];
        
    if( mem.mode == FLAG::IOCP_FLAG_FREE ){
        mem.mode  = FLAG::IOCP_FLAG_USED ;

        auto &ov  = mem.ov ; LPFN_ACCEPTEX accept = nullptr;
        auto &hd  = mem.fd ; hd = (HANDLE) fd->get_fd();
        auto &c   = mem.c  ; c  = 0UL  ; SOCKET tmp;
        auto &raw = mem.mem; ov = {0}  ;
        auto sk   = (SOCKET*) raw.get();

        sk[0] = WSASocketW( addr_family, addr_sock, addr_ipproto, NULL, 0, WSA_FLAG_OVERLAPPED );
        GUID GuidAcceptEx = WSAID_ACCEPTEX; WSAIoctl( fd->get_fd(), 
             SIO_GET_EXTENSION_FUNCTION_POINTER, 
            &GuidAcceptEx, sizeof(GuidAcceptEx),
            &accept, sizeof(accept), &c, NULL, NULL
        );

        if( accept( fd->get_fd(), sk[0], raw.get()+sizeof(SOCKET), 0, addr_len[0]+16, addr_len[0]+16, &c, &ov ) ){

            auto en = fd->get_fd();
            int  c  =::setsockopt( sk[0], SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&en, sizeof(SOCKET) );
            mem.mode = FLAG::IOCP_FLAG_FREE; return c==0 ? sk[0] : INVALID_SOCKET;

        } elif( is_blocked( c ) ){ return -2; } 
        
            mem.mode = FLAG::IOCP_FLAG_FREE; return -1;

    } int out = io_consumer( raw, flag ); switch( out<0 ){

        case true : return out; break; case false: do { 

            string_t &raw = mem.mem; auto sk = (SOCKET*) raw.get(); auto en = fd->get_fd();
            int c=::setsockopt( sk[0], SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&en, sizeof(SOCKET) );
            return c==0 ? sk[0] : INVALID_SOCKET; 
            
        } while(0); break;

    }}

    /*─······································································─*/

    template< class T >
    int send( T* fd, void* buf, DWORD len, int flags ) const noexcept {
        
        auto flag = FLAG::IOCP_FLAG_WRTE;
        auto io   = io_submitter( fd, flag );
        auto raw  = obj->que.as ( io );
        auto &mem = raw->data.data  [ flag ];
        auto size = min( len, (DWORD) NODEPP_CHUNK_SIZE );
        
    if( mem.mode == FLAG::IOCP_FLAG_FREE ){
        mem.mode  = FLAG::IOCP_FLAG_USED ;

        auto &raw = mem.mem;
        auto &ov  = mem.ov ; ov = {0}  ;
        auto &c   = mem.c  ; c  = 0UL  ;
        auto &f   = mem.f  ; f  = flags;
        auto &bu  = mem.buf; bu = { size, raw.get() };
        auto &hd  = mem.fd ; hd = (HANDLE) fd->get_fd();

        memcpy ( raw.get(), buf, size );
        WSASend( fd->get_fd(), &bu, 1, &c, f, &ov, NULL );

        if( is_blocked(c) ) { return -2; }
        mem.mode = FLAG::IOCP_FLAG_FREE; return (int) c;

    }   return io_consumer( raw, flag ); }

    template< class T, class V >
    int sendto( T* fd, void* buf, DWORD len, int flags, V* addr, socklen_t addrlen ) const noexcept {
        
        auto flag = FLAG::IOCP_FLAG_WRTE;
        auto io   = io_submitter( fd, flag );
        auto raw  = obj->que.as ( io );
        auto &mem = raw->data.data  [ flag ];
        auto size = min( len, (DWORD) NODEPP_CHUNK_SIZE );
        
    if( mem.mode == FLAG::IOCP_FLAG_FREE ){
        mem.mode  = FLAG::IOCP_FLAG_USED ;

        auto &raw = mem.mem;
        auto &ov  = mem.ov ; ov = {0}  ;
        auto &c   = mem.c  ; c  = 0UL  ;
        auto &f   = mem.f  ; f  = flags;
        auto &bu  = mem.buf; bu = { size, raw.get() };
        auto &hd  = mem.fd ; hd = (HANDLE) fd->get_fd();

        memcpy   ( raw.get   (), buf, size );
        WSASendTo( fd->get_fd(), &bu, 1, &c, f, addr, addrlen, &ov, NULL );

        if( is_blocked(c) ) { return -2; } 
        mem.mode = FLAG::IOCP_FLAG_FREE; return (int) c;

    }   return io_consumer( raw, flag ); }

    /*─······································································─*/

    template< class T >
    int recv( T* fd, void* buf, DWORD len, int flags ) const noexcept {
        
        auto flag = FLAG::IOCP_FLAG_READ;
        auto io   = io_submitter( fd, flag );
        auto raw  = obj->que.as ( io );
        auto &mem = raw->data.data  [ flag ];
        auto size = min( len, (DWORD) NODEPP_CHUNK_SIZE );
        
    if( mem.mode == FLAG::IOCP_FLAG_FREE ){
        mem.mode  = FLAG::IOCP_FLAG_USED ;

        auto &raw = mem.mem;
        auto &ov  = mem.ov ; ov = {0}  ;
        auto &c   = mem.c  ; c  = 0UL  ;
        auto &f   = mem.f  ; f  = flags;
        auto &bu  = mem.buf; bu = { size, raw.get() };
        auto &hd  = mem.fd ; hd = (HANDLE) fd->get_fd();

        WSARecv( fd->get_fd(), &bu, 1, &c, &f, &ov, NULL );

        if( is_blocked(c) ) { return -2; } memmove( buf, raw.get(), c );
        mem.mode = FLAG::IOCP_FLAG_FREE; return (int) c;

    } int out = io_consumer( raw, flag ); switch( out<=0 ){

        case true : return out; break; case false: do {

            string_t &raw = mem.mem; 
            memmove( buf, raw.get(), out );

        return out; } while(0); break;

    }}

    template< class T, class V >
    int recvfrom( T* fd, void* buf, DWORD len, int flags, V* addr, socklen_t* addrlen ) const noexcept {
        
        auto flag = FLAG::IOCP_FLAG_READ;
        auto io   = io_submitter( fd, flag );
        auto raw  = obj->que.as ( io );
        auto &mem = raw->data.data  [ flag ];
        auto size = min( len, (DWORD) NODEPP_CHUNK_SIZE );
        
    if( mem.mode == FLAG::IOCP_FLAG_FREE ){
        mem.mode  = FLAG::IOCP_FLAG_USED ;

        auto &raw = mem.mem;
        auto &ov  = mem.ov ; ov = {0}  ;
        auto &c   = mem.c  ; c  = 0UL  ;
        auto &f   = mem.f  ; f  = flags;
        auto &bu  = mem.buf; bu = { size, raw.get() };
        auto &hd  = mem.fd ; hd = (HANDLE) fd->get_fd();

        WSARecvFrom( fd->get_fd(), &bu, 1, &c, &f, addr, addrlen, &ov, NULL );

        if( is_blocked(c) ) { return -2; } memmove( buf, raw.get(), c );
        mem.mode = FLAG::IOCP_FLAG_FREE; return (int) c;

    } int out = io_consumer( raw, flag ); switch( out<=0 ){

        case true : return out; break; case false: do {

            string_t &raw = mem.mem; 
            memmove( buf, raw.get(), out );

        return out; } while(0); break;

    }}

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { 
inline /*--------*/ iocp_t& NODEPP_IOCP () {
thread_local static iocp_t out; return out; }}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/