/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_QUEUE
#define NODEPP_QUEUE

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {
template< class V > class queue_t {
protected:

    class NODE { public:
          NODE *next = nullptr;
          NODE *prev = nullptr; V data;
          NODE( V value ){ data = value; }
    };

    struct DONE {
           NODE *fst    = nullptr;
           NODE *act    = nullptr;
           NODE *lst    = nullptr;
           ulong length = 0;
    };     ptr_t<DONE> obj;

    type::optional<ulong[3]> get_slice_range( long x, long y ) const noexcept {

        if( empty() || x == y ){ return nullptr; } if( y>0 ){ --y; }

        if( x < 0 ){ x = size()+0+x; } if( (ulong)x > size()-1 ){ return nullptr; }
        if( y < 0 ){ y = size()-1+y; } if( (ulong)y > size()-1 ){ y = size() - 1; }
        if( y < x ){ return nullptr; }

        ulong a = clamp( (ulong)y, 0UL, size()-1 );
        ulong b = clamp( (ulong)x, 0UL, a        );
        ulong c = a - b + 1;

        ulong arr[3]; /*-----------------------*/
              arr[0] = b; arr[1] = a; arr[2] = c;

        return arr;
    }

    type::optional<ulong[3]> get_splice_range( long x, ulong y ) const noexcept {

        if( empty() || y == 0 ){ return nullptr; }

        if( x < 0 ){ x = size()-1+x; } if( (ulong)x > size()-1 ){ return nullptr; }
            y += x - 1;
        if( y > size()-1 ){ y= size()-1; } if( y < (ulong)x )   { return nullptr; }

        ulong a = clamp( (ulong)y, 0UL, size()-1 );
        ulong b = clamp( (ulong)x, 0UL, a        );
        ulong c = a - b + 1;

        ulong arr[3]; /*-----------------------*/
              arr[0] = b; arr[1] = a; arr[2] = c;

        return arr;
    }

public:

    virtual ~queue_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    /*─······································································─*/

    queue_t( const V* value, const ulong& n=0 ) noexcept : obj( new DONE ) {
        if ( value == nullptr || n == 0 ){ return; } auto i=n;
      while( i-->0 ){ unshift(value[i]); }
    }

    queue_t( const ulong& n, const V& c ) noexcept: obj( new DONE )  {
        if ( n == 0 ){ return; }
        auto i=n; while( i-->0 ){ unshift(c); }
    }

    template < class T, ulong N >
    queue_t( const T (&value)[N] ) noexcept : obj( new DONE ) {
        auto i=N; while( i-->0 ){ unshift(value[i]); }
    }

    queue_t( const ptr_t<V>& args ) noexcept: obj( new DONE ) {
        for( auto &x: args ){ push( x ); }
    }

    queue_t() noexcept : obj( new DONE ) {}

    /*─······································································─*/

    queue_t<V> sort( function_t<bool,V,V> func ) const noexcept {
        queue_t<V> n_buffer;

        auto x = this->first(); while( x != nullptr ){
        auto n = n_buffer.first();

            while( n!=nullptr ){ if( !func( x->data, n->data ) )
                 { n = n->next; continue; } break;
            }

            n_buffer.insert( n, x->data ); x = x->next;
        }

        return n_buffer;
    }

    /*─······································································─*/

    V& operator[]( ulong idx ) const noexcept { return this->get( idx )->data; }

    /*─······································································─*/

    NODE* first() const noexcept { return obj->fst==nullptr?nullptr:obj->fst; }
    NODE* last()  const noexcept { return obj->lst==nullptr?first():obj->lst; }
    bool empty()  const noexcept { return obj.null() ? 0 : obj->length==0;    }
    ulong size()  const noexcept { return    empty() ? 0 : obj->length;       }

    /*─······································································─*/

    ptr_t<V> data() const noexcept {
        if( empty() ){ return nullptr; } /*---------*/
        ptr_t<V> out( size() ); V* addr = out.begin();
        auto n = first(); while( n!=nullptr ){
            *addr = n->data; ++addr;
        n = n->next; } return out;
    }

    /*─······································································─*/

    int index_of( function_t<bool,V> func ) const noexcept {
        int  i=0; auto n= first(); if( empty() ){ return -1; }
        while( n != nullptr ) { /*--------------------------*/
        if( func(n->data) == 1 ){ return i; }
        if( n->next == nullptr ){ break; }
        ++i; n = n->next; } return -1;
    }

    int count( function_t<bool,V> func ) const noexcept {
        int  i=0; auto n = first(); if( empty() ){ return 0; }
        while( n != nullptr ) { /*--------------------------*/
        if( func(n->data) == 1 ){ ++i; }
        n = n->next; } return i;
    }

    queue_t reverse() const noexcept { 
        auto n_buffer=queue_t<V>(); auto n=first();
        while( n!=nullptr ){ /*------------------*/
            n_buffer.unshift( n->data );
        n=n->next; } return n_buffer;
    }

    /*─······································································─*/

    bool some( function_t<bool,V> func ) const noexcept {
        if( empty() ){ return false; } /*------*/
        auto n = first(); while( n != nullptr ) {
        if( func(n->data) == 1 ){ return 1; }
        n = n->next; } return false;
    }

    bool none( function_t<bool,V> func ) const noexcept {
        if( empty() ){ return false; } /*------*/
        auto n = first(); while( n != nullptr ) {
        if( func(n->data) == 1 ){ return 0; }
        n = n->next; } return true;
    }

    bool every( function_t<bool,V> func ) const noexcept {
        if( empty() ){ return false; } /*------*/
        auto n = first(); while( n != nullptr ) {
        if( func(n->data) == 0 ){ return 0; }
        n = n->next; } return true;
    }

    void map( function_t<void,V&> func ) const noexcept {
        if( empty() ){ return; } auto n = first(); /*-----*/
        while( n!=nullptr ){ func( n->data ); n = n->next; }
    }

    /*─······································································─*/

    inline bool is_item( void* item ) const noexcept {
        if( empty() || item==nullptr ){ return false; }
        /*---------------------------*/ return true; /*
        auto n = first(); while( n!=nullptr ){
         if( n == item ){ return true; }
        n=n->next; } return false;
        */
    }

    queue_t copy() const noexcept { queue_t n_buffer;
        auto n = first(); while( n!=nullptr ){
             n_buffer.push( n->data );
        n=n->next(); } return n_buffer;
    }

    /*─······································································─*/

    queue_t slice( long start ) const noexcept {

        auto n_buffer = queue_t<V>(); uint idx=0;
	    auto r = get_slice_range( start,size() );
        if( !r.has_value() ){ return nullptr; }

        auto z = *r.get(); auto n = get( z[0] ); 
        
        while( n!=nullptr && idx<z[2] )
             { n_buffer.push( n->data ); n=n->next; ++idx; }
        return n_buffer;

    }

    queue_t slice( long start, long end ) const noexcept {

        auto n_buffer = queue_t<V>(); uint idx=0;
	    auto r = get_slice_range( start, end );
        if( !r.has_value() ){ return nullptr; }

        auto z = *r.get(); auto n = get( z[0] ); 
        
        while( n!=nullptr && idx<z[2] )
             { n_buffer.push( n->data ); n=n->next; ++idx; }
        return n_buffer;

    }

    /*─······································································─*/

    queue_t splice( long start, ulong end ) const noexcept {

        auto n_buffer = queue_t<V>(); uint idx=1;
	    auto r = get_splice_range( start, end );
        if( !r.has_value() ){ return nullptr; }

        auto z = *r.get(); auto n = get( z[0] ); 
        
        while( n!=nullptr && idx<=z[2] )
             { n_buffer.push( n->data ); n=n->next; ++idx; }

        erase( z[0], z[0]+end ); return n_buffer;

    }

    template< class T, ulong N >
    queue_t splice( long start, ulong end, const T (&value)[N] ) const noexcept {

        auto n_buffer = queue_t<V>(); uint idx=1;
	    auto r = get_splice_range( start, end );
        if( !r.has_value() ){ return nullptr; }

        auto z = *r.get(); auto n = get( z[0] ); 
        
        while( n!=nullptr && idx<=z[2] )
             { n_buffer.push( n->data ); n=n->next; ++idx; }

        erase( z[0], z[0]+end ); insert( z[0], value ); return n_buffer;

    }

    /*─······································································─*/

    void unshift( const V& value ) const noexcept { insert( first(), value ); }
    void    push( const V& value ) const noexcept { insert( nullptr, value ); }
    void                   shift() const noexcept { erase( first() ); }
    void                     pop() const noexcept { erase( nullptr ); }

    /*─······································································─*/

    inline NODE* next() const noexcept { if( empty() ){ return nullptr; }
           obj->act = obj->act      ==nullptr ? first() :
                      obj->act->next==nullptr ? first() : obj->act->next;
    return obj->act; }

    inline NODE* prev() const noexcept { if( empty() ){ return nullptr; }
           obj->act = obj->act      ==nullptr ? last() :
                      obj->act->prev==nullptr ? last() : obj->act->prev;
    return obj->act; }

    /*─······································································─*/

    void clear() const noexcept { while( !empty() ){ shift(); } }
    void erase() const noexcept { while( !empty() ){ shift(); } }
    void  free() const noexcept { while( !empty() ){ shift(); } }

    /*─······································································─*/

    template< class T, ulong N >
    inline void insert( ulong index, const T(&value)[N] ) const noexcept {
	    index = clamp( index, 0UL, size() );
    	ulong i=index; for( ulong x=0; x<N; ++x )
        { insert( i+x, value[x] ); }
    }

    inline void insert( ulong index, ulong N, V* value ) const noexcept {
	    index = clamp( index, 0UL, size() );
    	ulong i=index; for( ulong x=0; x<N; ++x )
        { insert( i+x, value[x] ); }
    }

    inline void insert( ulong index, const V& value ) const noexcept {
	    index = clamp( index, 0UL, size() );
        if( index>=size() ){ insert( nullptr   , value ); }
        else /*---------*/ { insert( get(index), value ); }
    }

    inline void insert( NODE* n, const V& value ) const noexcept {
        if( empty() ){
            obj->fst = new NODE( value ); obj->lst=first();
        } elif ( is_item(n) ) {
            auto m = new NODE( value ); m->prev = n->prev;
            if ( n->prev!= nullptr ){ n->prev->next = m; }
            if ( n->next== nullptr ){ obj->lst = n; }
            if ( m->prev== nullptr ){ obj->fst = m; }
                 m->next = n; n->prev = m;
        } else {
            auto n = last(); auto m = new NODE( value );
            if ( n->prev== nullptr ){ obj->fst = n; }
            if ( m->next== nullptr ){ obj->lst = m; }
                 m->prev = n; n->next = m;
        } ++obj->length;
    }

    /*─······································································─*/

    inline void erase( ulong begin, ulong end ) const noexcept {
        auto r = get_slice_range( begin, end ); /*---------*/
           if( !r.has_value() ){ return; } auto z = *r.get();
        /*------------------------------*/ auto x = z[2];
        while( x-->0 ) { erase( z[0] ); }
    }

    inline void erase( ulong begin ) const noexcept {
        auto r = get_slice_range( begin, size() ); /*------*/
           if( !r.has_value() ){ return; } auto z = *r.get();
        erase( get( z[0] ) );
    }

    inline void erase( NODE* n ) const noexcept {
        if( empty()    )/*---------*/{ return;     }
        if(!is_item(n) )/*---------*/{ n = last(); }
        if( n->next == nullptr )/*-*/{ obj->lst= n->prev; }
        if( n->prev == nullptr )/*-*/{ obj->fst= n->next; }
        if( n == obj->act ){ next(); } do {
        if( n->prev != nullptr )/*-*/{ n->prev->next = n->next; }
        if( n->next != nullptr )/*-*/{ n->next->prev = n->prev; }
          } while(0); obj->length--; delete n;
    }

    /*─······································································─*/

    inline void set( NODE* x ) const noexcept { if( is_item(x) ){ obj->act = x; } }

    inline NODE* get() const noexcept {
        if( empty() )/*------*/{ return nullptr;   }
        if( obj->act==nullptr ){ obj->act=first(); } return obj->act;
    }

    inline NODE* get( ulong x ) const noexcept {
        if( empty() ){ return nullptr; } 
        auto y=x%size(); auto n=first();
        while( n != nullptr && y-->0 ){ n=n->next; } return n;
    }

    inline NODE* get( void* x ) const noexcept {
        if( empty() )/*----*/{ return nullptr; }
        auto n = first(); while( n != nullptr ){
        if ( n == x ) { break; } n  = n->next; } return n;
    }

    inline NODE* as( void* x ) const noexcept {
        if( x == nullptr ){ return nullptr; }
        return type::cast<NODE>(x);
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif