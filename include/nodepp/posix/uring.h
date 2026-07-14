/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_IOURING
#define NODEPP_POSIX_IOURING

/*────────────────────────────────────────────────────────────────────────────*/

#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <linux/io_uring.h>

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_URING_ATOMIC_LOAD( a )    __atomic_load_n   ( a,    __ATOMIC_ACQUIRE )
#define NODEPP_URING_ATOMIC_SAVE( a, b ) __atomic_store_n  ( a, b, __ATOMIC_RELEASE )
#define NODEPP_URING_ATOMIC_ADD_( a, b ) __atomic_fetch_add( a, b, __ATOMIC_SEQ_CST )

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class uring_t {
protected:

    using IOtmr = struct __kernel_timespec;
    using IOpll = struct pollfd      ;
    using IOsqe = struct io_uring_sqe;
    using IOcqe = struct io_uring_cqe;
    using IOprm = struct io_uring_params;
    
    /*─······································································─*/

    using IOvec = struct iovec ;
    using IOmsg = struct msghdr;
    
    /*─······································································─*/

    enum FLAG : int {
         URING_FLAG_NULL = 0b00000000,
         URING_FLAG_FREE = 0b00000001,
         URING_FLAG_USED = 0b00000010,
         URING_FLAG_DONE = 0b00000100,
         URING_FLAG_WRTE = 0b00000001,
         URING_FLAG_READ = 0b00000000
    };
    
    /*─······································································─*/

    struct DONE { void* sign; int mode; IOsqe sqe; IOcqe cqe; string_t mem; };
    struct DENO { void* addr; DONE data[2]; };
    struct NODE {
 
        IOprm prm; IOpll pqes; uint entries; int count=0;

        volatile uint *sq_tail , *sq_head, *sq_mask; 
        volatile uint *sq_array;
        volatile uint *cq_tail , *cq_head, *cq_mask;
        
        IOsqe *sqes  ; IOcqe *cqes; 
        void *sq_ptr , *cq_ptr    ; 
        
        queue_t<void*> borrow;
        queue_t<DENO > que   ; int ed = -1;
        queue_t<DONE > sque  ; int fd = -1;
        
        size_t sq_ring_size, cq_ring_size;

       ~NODE(){
        if( ed == -1 && fd == -1 ) { return; }
            munmap( sqes  , entries * sizeof( IOsqe ) ); 
            munmap( sq_ptr, sq_ring_size ); ::close(ed);
            munmap( cq_ptr, cq_ring_size ); ::close(fd);
        }

    }; ptr_t<NODE> obj;
    
    /*─······································································─*/

    template< class T >
    int io_consumer( T* raw, int flag ) const noexcept {
    if( raw==nullptr ) { errno=EINVAL; return -1; }

        auto &mem = raw->data.data[ flag ];
    
    if( mem.mode==FLAG::URING_FLAG_DONE ){
        mem.mode =FLAG::URING_FLAG_FREE ;
        
        IOsqe sqe = mem.sqe;
        IOcqe cqe = mem.cqe;

        if  ( cqe.res  < 0  )
            { errno=-cqe.res; return -1; } 
        else{ return cqe.res; /*------*/ }
    
    }

        io_submit (); 
        io_consume();

    errno=EWOULDBLOCK; return -1; }
    
    /*─······································································─*/

    ptr_t<void*> io_queue() const noexcept { 
        return obj->borrow.splice(0,NODEPP_MAX_BATCH_SIZE).data(); 
    }
    
    /*─······································································─*/

    template< class T >
    void* io_submitter( T* fd, int /*unused*/ ) const noexcept {

        void* raw = (void*) fd->get_pd();
        auto  mem = obj->que.as( raw );

    if( mem!=nullptr ) { return mem; } else {

        obj->que.push({ DENO({0}) });

        auto &mem     = /*-----*/ obj->que.last()->data; 
        fd ->get_pd() = (uchar_64)obj->que.last();

        mem.addr         = nullptr;
        mem.data[0].mode = FLAG::URING_FLAG_FREE;
        mem.data[1].mode = FLAG::URING_FLAG_FREE;

    } return io_submitter( fd, 0x00 ); }

protected:

    int  io_submit() const noexcept {
    uint tail = NODEPP_URING_ATOMIC_LOAD( obj->sq_tail ); 
    uint mask = NODEPP_URING_ATOMIC_LOAD( obj->sq_mask ); 
    uint head = NODEPP_URING_ATOMIC_LOAD( obj->sq_head );

        if( obj->sque.empty() ) /*---*/ { return -1; }

        uint available = obj->entries - (tail - head);
        int  z=0; auto x=obj->sque.first();

        while( x != nullptr && z < (int)available ) {
            uint index = tail & mask;
            
            obj->sqes[index] = x->data.sqe;
            tail++ ; z++ ; x = x->next;

        NODEPP_URING_ATOMIC_SAVE( obj->sq_array + index, index );
        NODEPP_URING_ATOMIC_SAVE( obj->sq_tail  , tail ); }

    if( z> 0 ){ 

        int x=syscall( __NR_io_uring_enter, obj->fd, z, 0, 0, NULL ); 

    if( x> 0 ){ obj->sque.splice( 0, x ); return x; }} return 0; }
    
    /*─······································································─*/

    int  io_consume() const noexcept {
    uint tail = NODEPP_URING_ATOMIC_LOAD( obj->cq_tail ); int z=0; 
    uint head = NODEPP_URING_ATOMIC_LOAD( obj->cq_head );
    uint mask = NODEPP_URING_ATOMIC_LOAD( obj->cq_mask );
    
        if( head==tail ) { return -1; } while( head!=tail ){
        
            uint  index = head & mask;
            IOcqe  *cqe = &obj->cqes[index];  

            uchar_64 pt = cqe->user_data &  0x0000FFFFFFFFFFFF;
            uchar_64 md = cqe->user_data >> 48;

            auto y = obj->que.as( (void*) pt );
            if ( y != nullptr ) {

                y->data.data[md].mode = FLAG::URING_FLAG_DONE;
                y->data.data[md].cqe  = *cqe;

            if( y->data.addr != nullptr ){
                obj->borrow.push( y->data.addr );
            } z++; }
        
        head++; }

    NODEPP_URING_ATOMIC_SAVE( obj->cq_head, head ); return z; }

public: 

    uring_t() : obj( 0UL ) {}

    /*─······································································─*/

    int start_device() const noexcept { if( obj->ed == -1 && obj->fd == -1 ) {

        obj->entries = NODEPP_MAX_BATCH_SIZE ; IOprm &p = obj->prm;
        obj->fd = syscall(__NR_io_uring_setup, obj->entries, &obj->prm);

        obj->sq_ring_size = p.sq_off.array + p.sq_entries * sizeof(uint);
        obj->sq_ptr = mmap(NULL, obj->sq_ring_size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, obj->fd, IORING_OFF_SQ_RING);
        
        obj->cq_ring_size = p.cq_off.cqes + p.cq_entries * sizeof(IOcqe);
        obj->cq_ptr = mmap(NULL, obj->cq_ring_size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, obj->fd, IORING_OFF_CQ_RING);

        obj->sqes = (IOsqe*)mmap(NULL, p.sq_entries * sizeof(IOsqe), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, obj->fd, IORING_OFF_SQES);
        obj->cqes = (IOcqe*)((char*)obj->cq_ptr + p.cq_off.cqes );

        obj->sq_mask  = (uint*)((char*)obj->sq_ptr + p.sq_off.ring_mask);
        obj->sq_array = (uint*)((char*)obj->sq_ptr + p.sq_off.array);
        obj->sq_tail  = (uint*)((char*)obj->sq_ptr + p.sq_off.tail );
        obj->sq_head  = (uint*)((char*)obj->sq_ptr + p.sq_off.head );

        obj->cq_mask  = (uint*)((char*)obj->cq_ptr + p.cq_off.ring_mask);
        obj->cq_tail  = (uint*)((char*)obj->cq_ptr + p.cq_off.tail );
        obj->cq_head  = (uint*)((char*)obj->cq_ptr + p.cq_off.head );

        obj->ed = eventfd( 0, EFD_CLOEXEC|EFD_NONBLOCK );
        obj->pqes.fd = obj->ed; obj->pqes.events= POLLIN;
        syscall(__NR_io_uring_register, obj->fd, IORING_REGISTER_EVENTFD, &obj->ed, 1);

    return 1; } return -1; }
    
    /*─······································································─*/

    int get_fd() const noexcept { return obj->fd; }

    int   wake() const noexcept { uchar_64 value =1; 
    return ::write(obj->ed,&value,sizeof(uchar_64)); }
    
    /*─······································································─*/

    template< class T >
    void* append( T* fd ) const noexcept { return io_submitter( fd, 0x00 ); }
    
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

    int free( uchar_64 pd ) const noexcept {
        auto mem = obj->que.as( (void*) pd );
    if( mem == nullptr ){ return -1; }
        obj->que.erase( mem );
    return 1; }

    int remove( uchar_64 pd ) const noexcept {
        auto mem = obj->que.as( (void*) pd );
    if( mem == nullptr ){ return -1; }
        mem->data.addr = nullptr;
    return 1; }

    /*─······································································─*/

    ptr_t<void*> next( int ms ) const noexcept {

        io_submit ();
        io_consume();

        if( ms!=0 && obj->borrow.empty() ){ uchar_64 value=0;
            ::poll( &obj->pqes, 1, ms ); io_consume();
        if( ::read( obj->ed,&value,sizeof(uchar_64))>0 ){} }

        return io_queue(); 
        
    }

    /*─······································································─*/
    
    template< class T >
    int write( T* fd, void* buf, size_t len ) const noexcept {
        
        auto  io  = io_submitter( fd, FLAG::URING_FLAG_WRTE );
        auto  raw = obj->que.as ( io );
        auto &mem = raw->data.data  [ FLAG::URING_FLAG_WRTE ];
        
    if( mem.mode== /**/ FLAG::URING_FLAG_FREE ){
        mem.mode      = FLAG::URING_FLAG_USED;
        IOsqe &sqe    = mem.sqe; 

        memset( &sqe, 0, sizeof(IOsqe) );

        sqe.addr      = (uchar_64) buf ;
        sqe.len       = len ;
        sqe.off       = -1;
        sqe.opcode    = IORING_OP_WRITE;

        sqe.fd        = fd->get_fd();
        sqe.user_data = fd->get_pd()| ((uchar_64)FLAG::URING_FLAG_WRTE) << 48; 
        
        obj->sque.push( mem ); errno=EWOULDBLOCK; return -1;
        
    } return io_consumer( raw, FLAG::URING_FLAG_WRTE ); }
    
    template< class T >
    int read( T* fd, void* buf, size_t len ) const noexcept {
        
        auto  io  = io_submitter( fd, FLAG::URING_FLAG_READ );
        auto  raw = obj->que.as ( io );
        auto &mem = raw->data.data  [ FLAG::URING_FLAG_READ ];
        
    if( mem.mode== /**/ FLAG::URING_FLAG_FREE ){
        mem.mode      = FLAG::URING_FLAG_USED;
        IOsqe &sqe    = mem.sqe;
        
        memset( &sqe, 0, sizeof(IOsqe) );

        sqe.addr      = (uchar_64) buf;
        sqe.len       = len ;
        sqe.off       = -1;
        sqe.opcode    = IORING_OP_READ;

        sqe.fd        = fd->get_fd();
        sqe.user_data = fd->get_pd()| ((uchar_64)FLAG::URING_FLAG_READ) << 48; 

        obj->sque.push( mem ); errno=EWOULDBLOCK; return -1;

    } return io_consumer( raw, FLAG::URING_FLAG_READ ); }
    
    /*─······································································─*/
    
    template< class T, class V >
    int connect( T* fd, V* buf, socklen_t len ) const noexcept {
        
        auto  io  = io_submitter( fd, FLAG::URING_FLAG_WRTE );
        auto  raw = obj->que.as ( io );
        auto &mem = raw->data.data  [ FLAG::URING_FLAG_WRTE ];
        
    if( mem.mode== /**/ FLAG::URING_FLAG_FREE ){
        mem.mode      = FLAG::URING_FLAG_USED;
        IOsqe &sqe    = mem.sqe;
        
        memset( &sqe, 0, sizeof(IOsqe) );

        sqe.opcode    = IORING_OP_CONNECT;
        sqe.addr      = (uchar_64) buf ;
        sqe.addr2     = (uchar_64) len ;

        sqe.fd        = fd->get_fd(); 
        sqe.user_data = fd->get_pd()| ((uchar_64)FLAG::URING_FLAG_WRTE) << 48; 

        obj->sque.push( mem ); errno=EWOULDBLOCK; return -1;

    } return io_consumer( raw, FLAG::URING_FLAG_WRTE ); }

    template< class T, class V >
    int accept( T* fd, V* buf, socklen_t* len ) const noexcept {
        
        auto  io  = io_submitter( fd, FLAG::URING_FLAG_READ );
        auto  raw = obj->que.as ( io );
        auto &mem = raw->data.data  [ FLAG::URING_FLAG_READ ];

    if( mem.mode== /**/ FLAG::URING_FLAG_FREE ){
        mem.mode      = FLAG::URING_FLAG_USED;
        IOsqe &sqe    = mem.sqe;
        
        memset( &sqe, 0, sizeof(IOsqe) );

        sqe.opcode    = IORING_OP_ACCEPT;
        sqe.addr      = (uchar_64) buf ;
        sqe.addr2     = (uchar_64) len ;

        sqe.fd        = fd->get_fd();
        sqe.user_data = fd->get_pd()| ((uchar_64)FLAG::URING_FLAG_READ) << 48; 

        obj->sque.push( mem ); errno=EWOULDBLOCK; return -1;
    
    } return io_consumer( raw, FLAG::URING_FLAG_READ ); }
    
    /*─······································································─*/
    
    template< class T >
    int send( T* fd, void* buf, size_t len, int flags ) const noexcept {
        
        auto  io  = io_submitter( fd, FLAG::URING_FLAG_WRTE );
        auto  raw = obj->que.as ( io );
        auto &mem = raw->data.data  [ FLAG::URING_FLAG_WRTE ];
        
    if( mem.mode== /**/ FLAG::URING_FLAG_FREE ){
        mem.mode      = FLAG::URING_FLAG_USED;
        IOsqe &sqe    = mem.sqe;
        
        memset( &sqe, 0, sizeof(IOsqe) );

        sqe.opcode    = IORING_OP_SEND;
        sqe.msg_flags = flags;
        sqe.len       = len  ;
        sqe.addr      = (uchar_64) buf;

        sqe.fd        = fd->get_fd();
        sqe.user_data = fd->get_pd()| ((uchar_64)FLAG::URING_FLAG_WRTE) << 48; 

        obj->sque.push( mem ); errno=EWOULDBLOCK; return -1;

    } return io_consumer( raw, FLAG::URING_FLAG_WRTE ); }
    
    template< class T, class V >
    int sendto( T* fd, void* buf, size_t len, int flags, const V* addr, socklen_t addrlen ) const noexcept {
        
        auto  io  = io_submitter( fd, FLAG::URING_FLAG_WRTE );
        auto  raw = obj->que.as ( io );
        auto &mem = raw->data.data  [ FLAG::URING_FLAG_WRTE ];
        
    if( mem.mode== /**/ FLAG::URING_FLAG_FREE ){
        mem.mode      = FLAG::URING_FLAG_USED;
        
        IOsqe    &sqe = mem.sqe;
        string_t &raw = mem.mem; 
        raw.resize( sizeof(IOmsg) + sizeof(IOvec), '\0' );
        
        memset( &sqe, 0, sizeof(IOsqe) );
        
        IOmsg* msg = (IOmsg*) raw.get() ;
        IOvec* iov = (IOvec*)(raw.get() + sizeof(IOmsg));

        iov->iov_base = buf;
        iov->iov_len  = len;

        msg->msg_name    = (void*)addr;
        msg->msg_namelen = addrlen;
        msg->msg_flags   = flags;
        msg->msg_iov     = iov;
        msg->msg_iovlen  = 1;

        sqe.opcode       = IORING_OP_SENDMSG;
        sqe.fd           = fd->get_fd() ;
        sqe.addr         = (uchar_64)msg;
        sqe.len          = 1;
        sqe.msg_flags    = 0;
        sqe.user_data    = fd->get_pd() | ((uchar_64)FLAG::URING_FLAG_WRTE) << 48; 

        obj->sque.push( mem ); errno=EWOULDBLOCK; return -1;

    } return io_consumer( raw, FLAG::URING_FLAG_WRTE ); }
    
    /*─······································································─*/
    
    template< class T >
    int recv( T* fd, void* buf, size_t len, int flags ) const noexcept {
        
        auto  io  = io_submitter( fd, FLAG::URING_FLAG_READ );
        auto  raw = obj->que.as ( io );
        auto &mem = raw->data.data  [ FLAG::URING_FLAG_READ ];
    
    if( mem.mode== /**/ FLAG::URING_FLAG_FREE ){
        mem.mode      = FLAG::URING_FLAG_USED;
        IOsqe &sqe    = mem.sqe;
        
        memset( &sqe, 0, sizeof(IOsqe) );

        sqe.opcode    = IORING_OP_RECV;
        sqe.msg_flags = flags;
        sqe.len       = len  ;
        sqe.addr      = (uchar_64) buf;

        sqe.fd        = fd->get_fd();
        sqe.user_data = fd->get_pd()| ((uchar_64)FLAG::URING_FLAG_READ) << 48; 

        obj->sque.push( mem ); errno=EWOULDBLOCK; return -1;
        
    } return io_consumer( raw, FLAG::URING_FLAG_READ ); }
    
    template< class T, class V >
    int recvfrom( T* fd, void* buf, size_t len, int flags, const V* addr, socklen_t* addrlen ) const noexcept {
        
        auto  io  = io_submitter( fd, FLAG::URING_FLAG_READ );
        auto  raw = obj->que.as ( io );
        auto &mem = raw->data.data  [ FLAG::URING_FLAG_READ ];

    if( mem.mode== /**/ FLAG::URING_FLAG_FREE ){
        mem.mode      = FLAG::URING_FLAG_USED;

        IOsqe    &sqe = mem.sqe;
        string_t &raw = mem.mem; 
        raw.resize( sizeof(IOmsg) + sizeof(IOvec), '\0' );
        
        memset( &sqe, 0, sizeof(IOsqe) );
        IOmsg* msg = (IOmsg*) raw.get() ;
        IOvec* iov = (IOvec*)(raw.get() + sizeof(IOmsg));

        iov->iov_base    = buf;
        iov->iov_len     = len;

        msg->msg_name    = (void*)addr;
        msg->msg_namelen = *addrlen;
        msg->msg_flags   = flags;
        msg->msg_iov     = iov  ;
        msg->msg_iovlen  = 1    ;

        sqe.opcode       = IORING_OP_RECVMSG;
        sqe.fd           = fd->get_fd() ;
        sqe.addr         = (uchar_64)msg;
        sqe.len          = 1;
        sqe.msg_flags    = 0;
        sqe.user_data    = fd->get_pd() | ((uchar_64)FLAG::URING_FLAG_READ) << 48; 
        
        obj->sque.push( mem ); errno=EWOULDBLOCK; return -1;

    } return io_consumer( raw, FLAG::URING_FLAG_READ ); }

}; }

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { 
inline /*--------*/ uring_t& NODEPP_URING () {
thread_local static uring_t out; return out; }}

/*────────────────────────────────────────────────────────────────────────────*/

#undef NODEPP_URING_ATOMIC_LOAD
#undef NODEPP_URING_ATOMIC_SAVE
#undef NODEPP_URING_ATOMIC_ADD_
#endif

/*────────────────────────────────────────────────────────────────────────────*/