/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_ALLOCATOR
#define NODEPP_ALLOCATOR

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< ulong HEAP_SIZE=NODEPP_HEAP_SIZE > class allocator_t {
private:

    using u_8  = unsigned char;
    using u_16 = uint16_t     ;
    using u_32 = uint32_t     ;
    using u_64 = uint64_t     ;

protected:

    enum FLAG : u_8 {
         ALLOC_FLAG_NULL = 0x00,
         ALLOC_FLAG_FREE = 0x01,
         ALLOC_FLAG_USED = 0x02,
         ALLOC_FLAG_SOLO = 0x03
    };

    u_64 _align_( u_64 size ) const noexcept {
         u_64 ALIGN = NODEPP_ARCH_SIZE;
         u_64 MASK  = ALIGN - 1;
    return (size + MASK) &~ MASK; }

protected:

    struct HEAD { u_64 size; u_8* offs; };
    struct BODY { u_64 size; u_8  mode; };
    struct NODE {
        NODE* next= nullptr; NODE* prev= nullptr;
        HEAD  head; u_8 bffr[HEAP_SIZE];
    };
    
    /*─······································································─*/

    NODE* obj = nullptr;
    NODE* act = nullptr;

    /*─······································································─*/

    int _mov_page_( NODE* addr ) noexcept {
    if( addr && addr!=obj ){

        if( addr->prev ){ addr->prev->next=addr->next; }
        if( addr->next ){ addr->next->prev=addr->prev; }

        obj->prev=addr; addr->next=obj ; obj=addr;
    
    return 1; } return addr ? 0 : -1; }

    int _del_page_( NODE* addr ) noexcept {
        if( _mov_page_(addr)!=-1 ){
            obj = addr->next; ::free( addr ); 
        return 1; } return -1; 
    }

    int _clr_page_() noexcept { do {
        if   ( NODEPP_SHTDWN() ){ break; } 
        while( obj ){ auto x=obj->next; ::free(obj); obj=x; }
    return 1; } while(0) ; return -1; }

    int _new_page_() noexcept {

        NODE* addr = (NODE*) ::malloc( _align_( sizeof(NODE) ) );
        if( obj ){ obj->prev = addr; }

        addr->prev= nullptr;
        addr->next= obj ; obj = addr ;
        addr->head.size = HEAP_SIZE  ;
        addr->head.offs = addr->bffr ;

        BODY* body = (BODY*)addr->head.offs;
        body->size = HEAP_SIZE-sizeof(BODY);
        body->mode = FLAG::ALLOC_FLAG_FREE ;

    return 1; }

    /*─······································································─*/

    void* _new_mem_( u_64 size ) noexcept {
    if ( size == 0 ){ return nullptr; }

        u_64 sz = _align_    ( size );
        u_64 sy = sz + sizeof( BODY );
        u_64 sx = sy + sizeof( BODY );

    do { if ( sy >= HEAP_SIZE ) {

        u_8 * mem  = (u_8 *) ::malloc(sy);
        BODY* body = (BODY*) mem;
        body->size = sz         ;
        body->mode = FLAG::ALLOC_FLAG_SOLO;

        return (void*)( mem + sizeof(BODY) );

    } else {

        NODE* x=obj; if( x == nullptr ){ break; }
    //  if( x->head.size < sx ){ _nxt_mem_ (x); }
        if( x->head.size < sx ){
        if( x->head.size !=sy ){ break; }}

        u_8 * mem  = x->head.offs;

        x->head.offs = mem + sy  ;
        x->head.size = x->head.size - sy;

        BODY* body = (BODY*) mem ;
        body->size = sz          ;
        body->mode = FLAG::ALLOC_FLAG_USED;

    if( x->head.size > 0UL ){

        BODY* body = (BODY*) x->head.offs ;
        body->mode = FLAG::ALLOC_FLAG_FREE;
        body->size = x->head.size - sizeof(BODY);

    } return (void*)( mem + sizeof(BODY) ); }} while(0);

        /*--*/ _new_page_( /**/ );
        return _new_mem_ ( size );
    
    }

    /*─······································································─*/

    int _del_mem_( void*& addr ) noexcept { do {
    if( addr == nullptr ){ break; }
        
        u_8 * mem = (u_8 *) addr - sizeof(BODY);
        BODY* body= (BODY*) mem  ;

        if( body->mode == FLAG::ALLOC_FLAG_USED ||
            body->mode == FLAG::ALLOC_FLAG_FREE ){ 
            body->mode =  FLAG::ALLOC_FLAG_FREE ; 
        return 1; } 

        if( body->mode == FLAG::ALLOC_FLAG_SOLO ){
            body->mode =  FLAG::ALLOC_FLAG_FREE ; 
            ::free( mem ); return 1;
        }   ::free( mem ); return 0; 

    } while(0); return -1; }

    /*─······································································─*/

    int _nxt_mem_( NODE* addr ) noexcept { if( addr!=nullptr ){

        u_8 * end = addr->bffr + HEAP_SIZE;
        u_8 * prv = nullptr    ;
        u_8 * mem = addr->bffr ; while( mem<end ){

        BODY* bd1 = (BODY*)mem ;
        u_8 * nxt = mem + bd1->size + sizeof(BODY);

        if( bd1->mode == FLAG::ALLOC_FLAG_NULL ){ return 0; }
        if( bd1->mode == FLAG::ALLOC_FLAG_FREE ){
        if( prv != nullptr ){
            
            BODY* bd2 = (BODY*) prv;

        if( bd2->mode == FLAG::ALLOC_FLAG_FREE ){
            bd2->size += bd1->size + sizeof(BODY);
        } else { prv=mem; }
        } else { prv=mem; }

            BODY* bd2 = (BODY*) prv;

            if( addr->head.size < bd2->size + sizeof(BODY) ){
                addr->head.size = bd2->size + sizeof(BODY) ;
                addr->head.offs = prv       ;
            }

        } else { prv=nullptr; } mem=nxt; }

        if( addr->head.size>=     HEAP_SIZE ){ return 1; }
        if( addr->head.size>=obj->head.size ){ return 2; }

    return 0; } return -1; }

protected:

    int _next_() noexcept { if( obj!=nullptr ){
    int b = NODEPP_MAX_BATCH_SIZE; act = act ? act : obj;

        while ( b-->0 ){ auto x=act; act = act->next; 
        switch( _nxt_mem_(x) ){ 
            case 1: _del_page_(x); break; 
            case 2: _mov_page_(x); break; 
            case 0: /*--unused--*/ break;
        } if( !act ){ break; }}
         
    return 1; } return -1; }

public:

    allocator_t() { /*---------*/ }
   ~allocator_t() { _clr_page_(); }

    /*─······································································─*/

    void* calloc( u_64 num, u_64 size ) noexcept {
    void* mem = malloc( num * size );
          memset( mem, 0x00, num * size );
          return  mem;
    }

    void* realloc( void* ptr, u_64 size ) noexcept {
        if( size== 0 ) /**/ { free(ptr); return nullptr; }
        if( ptr == nullptr ){ return malloc(size); }
    void* mem = malloc(size); memmove (mem,ptr,size);
        free(ptr); return mem;
    }

    /*─······································································─*/

    void* malloc( u_64 size ) noexcept { return _new_mem_ ( size ); }
    int   free  ( void* ptr ) noexcept { return _del_mem_ ( ptr  ); }
    int   next  ( /*-----*/ ) noexcept { return _next_    ( /**/ ); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_ALLOW_ALLOCATOR==0

struct allocator_fallback_t {

    void* calloc( uint64_t num, uint64_t size ) noexcept {
    void* mem = malloc( num * size );
          memset( mem, 0x00, num * size );
          return mem;
    }

    void* realloc( void* ptr, uint64_t size ) noexcept {
        if( size== 0 ) /**/ { free(ptr); return nullptr; }
        if( ptr == nullptr ){ return malloc(size); }
        return ::realloc( ptr, size );
    }

    /*─······································································─*/

    void* malloc( uint64_t size ) noexcept { return (void*)  ::malloc(size); }
    int   free  ( void* ptr )     noexcept { ::free ( ptr ); return 1; }
    int   next  ( /*-----*/ )     noexcept { /*-----------*/ return 1; }

};

namespace nodepp {
allocator_fallback_t& NODEPP_ALLOC() /*---------*/ {
thread_local static allocator_fallback_t out; return out; }}

#else

namespace nodepp {
allocator_t<NODEPP_HEAP_SIZE>& NODEPP_ALLOC() /*---------*/ {
thread_local static allocator_t<NODEPP_HEAP_SIZE> out; return out; }}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_ALLOW_ALLOCATOR==1

void operator delete( void* ptr ) noexcept {
nodepp::NODEPP_ALLOC().free( ptr ); }

void* operator new  ( size_t size ) {
return nodepp::NODEPP_ALLOC().malloc( size ); }

/*─······································································─*/

void operator delete[]( void* ptr ) noexcept {
nodepp::NODEPP_ALLOC().free( ptr ); }

void* operator new[]  ( size_t size ) {
return nodepp::NODEPP_ALLOC().malloc( size ); }

/*─······································································─*/

#if __cplusplus >= 201402L

void operator delete  ( void* ptr, size_t ) noexcept {
nodepp::NODEPP_ALLOC().free( ptr ); }

void operator delete[]( void* ptr, size_t ) noexcept {
nodepp::NODEPP_ALLOC().free( ptr ); }

#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/