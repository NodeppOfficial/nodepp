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

    using u_8  = uchar;
    using u_16 = unsigned /*-----*/ int;
    using u_32 = unsigned long /**/ int;
    using u_64 = size_t; // unsigned long long int;

protected:

    enum FLAG : u_8 {
         ALLOC_FLAG_NULL = 0b00000000,
         ALLOC_FLAG_FREE = 0b00000100,
         ALLOC_FLAG_USED = 0b00000010,
         ALLOC_FLAG_SOLO = 0b00000001
    };

    u_64 _align_( u_64 size ) const noexcept {
         u_64 ALIGN = NODEPP_ARCH_SIZE;
         u_64 MASK  = ALIGN - 1;
    return (size + MASK) &~ MASK; }

protected:

    struct alignas(NODEPP_ARCH_SIZE) HEAD { u_64 size; u_8* offs; };
    struct alignas(NODEPP_ARCH_SIZE) BODY { u_64 size; u_8  mode; };
    struct alignas(NODEPP_ARCH_SIZE) NODE {
        NODE* next= nullptr  ; NODE* prev= nullptr;
        u_8 * addr= nullptr  ; HEAD  head;
        u_8   bffr[HEAP_SIZE];
    };
    
    /*─······································································─*/

    NODE* obj = nullptr;
    NODE* act = nullptr;

    /*─······································································─*/

    int _del_page_( NODE* addr ) noexcept { do {
    if( addr == nullptr ){ break; } 

        NODE* prv = addr->prev;
        NODE* nxt = addr->next;

        if( addr==obj ){ obj = obj->next; }
        if( addr==act ){ act = act->next; }

        if( prv )/*--*/{ prv->next = nxt; }
        if( nxt )/*--*/{ nxt->prev = prv; }

    ::free( addr ); return 1; } while(0); return -1; }

    /*─······································································─*/

    NODE* _new_page_() noexcept { do {

        NODE* addr = (NODE*) ::malloc( _align_( sizeof(NODE) ) );
        if( obj ){ obj->prev = addr ; }

        addr->head.offs = addr->bffr;
        addr->head.size = HEAP_SIZE ;
        addr->addr      = addr->bffr;

        addr->prev = nullptr ;
        addr->next = obj     ;
        obj        = addr    ;

        BODY* body = (BODY*) addr->bffr   ;
        body->mode = FLAG::ALLOC_FLAG_FREE;
        body->size = HEAP_SIZE - sizeof(BODY);

    return addr; } while(0); return nullptr; }

    /*─······································································─*/

    int _clr_page_() noexcept { do {
    if( obj==nullptr || NODEPP_SHTDWN() ){ break; } 
        NODE* tmp = obj; while( tmp!=nullptr ){
        NODE* nxt = tmp->next; _del_page_(tmp);
        tmp = nxt ; }
    return 1; } while(0); return -1; }

    /*─······································································─*/

    void* _new_mem_( u_64 size ) noexcept {
    if ( size == 0 ){ return nullptr; }

        u_64 sx = _align_( size + sizeof(BODY) + sizeof(BODY) );
        u_64 sy = _align_( size + sizeof(BODY) );

    do { if ( sy >= HEAP_SIZE ) {

        u_8 * mem  = (u_8 *) ::malloc(sy);
        BODY* body = (BODY*) mem;

        body->size = sy - sizeof(BODY);
        body->mode = FLAG::ALLOC_FLAG_SOLO;

        return (void*)( mem + sizeof(BODY) );

    } else { if( obj==nullptr ){ break; }

        NODE* x=obj; if( x == nullptr ){ break; }
        if( x->head.size <  sx ){ _nxt_mem_(x); }
        if( x->head.size <  sx ){
        if( x->head.size != sy ){ break; }}

        u_8 * end  = x->bffr + HEAP_SIZE;
        u_8 * amem = x->head.offs;

        BODY* body = (BODY*) amem;
        body->mode = FLAG::ALLOC_FLAG_USED;
        body->size = sx - sizeof(BODY) - sizeof(BODY);

    if( x->head.size == sy ){

        x->head.offs = nullptr;
        x->head.size = 0ULL   ;
    
    } else {

        u_8 * bmem = amem + body->size + sizeof(BODY);

        BODY* brrw = (BODY*) bmem;
        brrw->size = x->head.size - sx;
        brrw->mode = FLAG::ALLOC_FLAG_FREE;

        x->head.offs = bmem;
        x->head.size = brrw->size + sizeof(BODY);

    }

        return (void*)( amem + sizeof(BODY) );

    }} while(0);

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
            ::free(mem ); return 1;
        }   ::free(addr); return 0; 

    } while(0); return -1; }

    /*─······································································─*/

    int _nxt_mem_( NODE* addr ) noexcept { do {
    if( addr == nullptr ){ break; }

        u_8 * end = addr->bffr + HEAP_SIZE;
        u_8 * prv = nullptr    ;
        u_8 * mem = addr->bffr ; while( mem<end ){
        
        BODY* bd1 = (BODY*)mem ;
        u_8 * nxt = mem + bd1->size + sizeof(BODY);

        if( bd1->mode == FLAG::ALLOC_FLAG_FREE ){
        if( prv != nullptr ){
            
            BODY* bd2 = (BODY*)prv;

        if( bd2->mode == FLAG::ALLOC_FLAG_FREE ){
            bd2->size += bd1->size + sizeof(BODY);
        } else { prv=nullptr; }
        } else { prv=mem    ; }

            BODY* bd2 = (BODY*)prv;
    
            if( addr->head.size < bd2->size ){
                addr->head.size = bd2->size + sizeof(BODY);
                addr->head.offs = prv; 
            }

        } else { prv=nullptr; } mem=nxt; } 

        if( addr->head.size>=     HEAP_SIZE ){ return 1; }
        if( addr->head.size> obj->head.size ){ return 2; }

    return 0; } while(0); return -1; }

protected:

    /*─······································································─*/

    int _next_() noexcept { do { if( obj == nullptr ){ break; }
    int b = NODEPP_MAX_BATCH_SIZE; act = act ? act : obj;

        NODE* x=act    ; while ( x != nullptr && b-->0 ){
        NODE* y=x->next; switch( _nxt_mem_(x) ){
        case 1: _del_page_(x); break; 
        case 2: /*------------*/ do {

            if( x== obj ){ /*---*/ break ; }
            if( x->prev ){ x->prev->next = x->next; }
            if( x->next ){ x->next->prev = x->prev; }

            obj->prev = x; x->next = obj; obj = x;
    
        } while(0); break; } x=y; } act=x;

    return 1; } while(0); return -1; }

public:

    allocator_t() { /*---------*/ }
   ~allocator_t() { _clr_page_(); }

    /*─······································································─*/

    void* calloc( u_64 num, u_64 size ) noexcept {
    void* mem = malloc( num * size );
          memset( mem, 0x00, num * size );
          return mem;
    }

    void* realloc( void* ptr, u_64 size ) noexcept {
        if( size== 0 ) /**/ { free(ptr); return nullptr; }
        if( ptr == nullptr ){ return malloc(size); }
    void* mem = malloc(size); memmove(mem,ptr,size);
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

    void* calloc( size_t num, size_t size ) noexcept {
    void* mem = malloc( num * size );
          memset( mem, 0x00, num * size );
          return mem;
    }

    void* realloc( void* ptr, size_t size ) noexcept {
        if( size== 0 ) /**/ { free(ptr); return nullptr; }
        if( ptr == nullptr ){ return malloc(size); }
        return ::realloc( ptr, size );
    }

    /*─······································································─*/

    void* malloc( size_t size ) noexcept { return (void*)  ::malloc(size); }
    int   free  ( void*  ptr  ) noexcept { ::free ( ptr ); return 1; }
    int   next  ( /*-------*/ ) noexcept { /*-----------*/ return 1; }

};

namespace nodepp {
inline allocator_fallback_t& NODEPP_ALLOC() /*---------*/ {
thread_local static allocator_fallback_t out; return out; }}

#else

namespace nodepp {
inline allocator_t<NODEPP_HEAP_SIZE>& NODEPP_ALLOC() /*---------*/ {
thread_local static allocator_t<NODEPP_HEAP_SIZE> out; return out; }}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_ALLOW_ALLOCATOR==1

inline void operator delete( void* ptr ) noexcept {
nodepp::NODEPP_ALLOC().free( ptr ); }

inline void* operator new( size_t size ) {
return nodepp::NODEPP_ALLOC().malloc( size ); }

/*─······································································─*/

inline void operator delete[]( void* ptr ) noexcept {
nodepp::NODEPP_ALLOC().free( ptr ); }

inline void* operator new[]( size_t size ) {
return nodepp::NODEPP_ALLOC().malloc( size ); }

/*─······································································─*/

#if __cplusplus >= 201402L

inline void operator delete( void* ptr, size_t ) noexcept {
nodepp::NODEPP_ALLOC().free( ptr ); }

inline void operator delete[]( void* ptr, size_t ) noexcept {
nodepp::NODEPP_ALLOC().free( ptr ); }

#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/