/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_QUEUE
#define NODEPP_QUEUE

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {
template< class V > class queue_t {
protected: 

    class NODE { public:
        NODE* next = nullptr;
        NODE* prev = nullptr; V data; 
        NODE( V value ){ data = value; } 
    };
    
    struct DONE {
        NODE *fst    = nullptr;
        NODE *act    = nullptr;
        NODE *lst    = nullptr;
        ulong length = 0;
    };  ptr_t<DONE> obj;

    ptr_t<ulong> get_slice_range( long x, long y ) const noexcept {
        
        if( empty() || x == y ){ return nullptr; } if( y>0 ){ y--; }

        if( x < 0 ){ x = size()-1+x; } if( (ulong)x > size()-1 ){ return nullptr; }
        if( y < 0 ){ y = size()-1+y; } if( (ulong)y > size()-1 ){ y = size() - 1; }
                                       if( y < x )              { return nullptr; }

        ulong a = clamp( (ulong)y, 0UL, size()-1 );
        ulong b = clamp( (ulong)x, 0UL, a ); 
        ulong c = a - b + 1; return {{ b, a, c }};

    }

    ptr_t<ulong> get_splice_range( long x, ulong y ) const noexcept {
        
        if( empty() || y == 0 ){ return nullptr; }

        if( x < 0 ){ x = size()-1+x; } if( (ulong)x > size()-1 ){ return nullptr; }
            y += x - 1;
        if( y > size()-1 ){ y= size()-1; } if( y < (ulong)x )   { return nullptr; }

        ulong a = clamp( (ulong)y, 0UL, size()-1 );
        ulong b = clamp( (ulong)x, 0UL, a ); 
        ulong c = a - b + 1; return {{ b, a, c }};

    }
    
public:

    template < class T, ulong N >
    queue_t( const T (&value)[N] ) noexcept : obj( new DONE ) { 
        auto i=N; while( i-->0 ){ unshift(value[i]); }
    }

    template < class T >
    queue_t( const T* value, ulong N ) noexcept : obj( new DONE ) { 
        if( value == nullptr || N == 0 ){ return; }
        auto i=N; while( i-->0 ){ unshift(value[i]); }
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

            V item; memcpy( &item, &x->data, sizeof(V) );
            n_buffer.insert( n, item ); x = x->next;
        }

        return n_buffer;
    }
    
    /*─······································································─*/

    virtual ~queue_t() noexcept { if( obj.count() > 1 ){ return; } clear(); }
    
    /*─······································································─*/

    ulong size() const noexcept { return first() == nullptr ? 0 : obj->length; }

    bool empty() const noexcept { return first() == nullptr ? 1 : size()==0; }
    
    /*─······································································─*/

    NODE* operator[]( ulong idx ) const noexcept { return this->get( idx ); }
    
    /*─······································································─*/

    ptr_t<V> data() const noexcept { 
        if( empty() ){ return nullptr; } ptr_t<V> res ( size() );
        ulong y=0; auto n = first(); while( n!=nullptr ){ 
            res[y] = type::cast<V>( n->data ); 
            n = n->next; y++;
        }   return res;
    }
    
    /*─······································································─*/

    long index_of( function_t<bool,V> func ) const noexcept {
        long i=0; if( empty() ){ return -1; } 
        auto n = first(); while( n != nullptr ) { 
            if( func(n->data) == 1 ){ return i; }
            if( n->next == nullptr ){ break; }
            i++; n = n->next;
        }   return -1;
    }

    ulong count( function_t<bool,V> func ) const noexcept { 
        ulong i=0; if( empty() ){ return 0; } 
        auto n = first(); while( n != nullptr ) { 
            if( func(n->data) == 1 ){ i++; }
                n = n->next;
        }   return i;
    }
    
    /*─······································································─*/

    bool some( function_t<bool,V> func ) const noexcept {
        auto n = first(); while( n != nullptr ) { 
            if( func(n->data) == 1 ){ return 1; }
                n = n->next;
        }   return false;
    }

    bool none( function_t<bool,V> func ) const noexcept {
        if( empty() ){ return false; }
        auto n = first(); while( n != nullptr ) { 
            if( func(n->data) == 1 ){ return 0; }
                n = n->next;
        }   return true;
    }

    bool every( function_t<bool,V> func ) const noexcept {
        if( empty() ){ return false; }
        auto n = first(); while( n != nullptr ) { 
            if( func(n->data) == 0 ){ return 0; }
                n = n->next;
        }   return true;
    }

    void map( function_t<void,V&> func ) const noexcept {
        if( empty() ){ return; } auto n = first(); 
        while( n!=nullptr ){ func( n->data ); n = n->next; }
    }

    /*─······································································─*/
    
    bool is_item( NODE* item ) const noexcept {
        if( item == nullptr ){ return false; }
        auto n = first(); while( n!=nullptr ){
         if( n == item ){ return true; } 
             n = n->next;
        }    return false;
    }

    /*─······································································─*/

    void unshift( const V& value ) const noexcept { insert( first(), value ); }
    void    push( const V& value ) const noexcept { insert( nullptr, value ); }
    void                   shift() const noexcept { erase( first() ); }
    void                     pop() const noexcept { erase( nullptr ); }
    
    /*─······································································─*/

    void clear() const noexcept { while( !empty() ){ shift(); } }
    void erase() const noexcept { while( !empty() ){ shift(); } }
    void  free() const noexcept { while( !empty() ){ shift(); } }
    
    /*─······································································─*/

    void insert( ulong index, const V& value ) const noexcept { 
	    index = clamp( index, 0UL, size() - 1 );
	    insert( get(index), value ); 
    }

    void insert( ulong index, V* value, ulong N ) const noexcept {
	    index = clamp( index, 0UL, size() - 1 );
    	ulong i=index; for( ulong x=0; x<N; x++ )
            { insert( x, value[x] ); }
    }

    template< ulong N >
    void insert( ulong index, const V(&value)[N] ) const noexcept {
	    index = clamp( index, 0UL, size() - 1 );
    	ulong i=index; for( ulong x=0; x<N; x++ ) 
            { insert( x, value[x] ); }
    }

    void insert( NODE* n, const V& value ) const noexcept {
        if( empty() ){
            obj->fst = new NODE( value ); obj->lst = first();
        } elif ( is_item(n) ) {
            auto  m = new NODE( value ); m->prev = n->prev; 
            if ( n->prev!= nullptr ){ n->prev->next = m; }
            if ( n->next== nullptr ){ obj->lst = n; }
            if ( m->prev== nullptr ){ obj->fst = m; }
                 m->next = n; n->prev = m;
        } else { 
            auto n = last(); auto m = new NODE( value ); 
            if ( n->prev== nullptr ){ obj->fst = n; }
            if ( m->next== nullptr ){ obj->lst = m; }
                 m->prev = n; n->next = m;
        }   obj->length += 1;
    }
    
    /*─······································································─*/

    void erase( ulong begin, ulong end ) const noexcept {
        auto r = get_slice_range( begin, end );
           if( r == nullptr ){ return; }
        while( r[2]-->0 ) { erase( r[0] ); }
    }

    void erase( ulong begin ) const noexcept { 
        auto r = get_slice_range( begin, size() );
           if( r == nullptr ){ return; }
        erase( get( r[0] ) ); 
    }

    void erase( NODE* n ) const noexcept {
        if( empty() )                { return; }
        if(!is_item(n) )             { n = last(); }
        if( n->next == nullptr )     { obj->lst= n->prev; }
        if( n->prev == nullptr )     { obj->fst= n->next; }
        if( n == obj->act ){ next(); } do {
            if ( n->prev != nullptr ){ n->prev->next = n->next; }
            if ( n->next != nullptr ){ n->next->prev = n->prev; } 
        } while(0); delete n; obj->length -= 1;
    }

    /*─······································································─*/

    NODE* get() const noexcept { return obj->act==nullptr ? first() : obj->act; }

    void set( NODE* x ) const noexcept { if( is_item(x) ) obj->act = x; }

    NODE* get( ulong x ) const noexcept { 
        if( empty() ){ return nullptr; } auto n = first();
        while( n->next != nullptr && x-->0 ){ n = n->next; } return n;
    }
    
    /*─······································································─*/

    NODE* first() const noexcept { return obj->fst == nullptr ? nullptr : obj->fst; }
    NODE* last()  const noexcept { return obj->lst == nullptr ? first() : obj->lst; }
    
    /*─······································································─*/
    
    NODE* next() const noexcept { 
        obj->act = obj->act      ==nullptr ? obj->fst :
                   obj->act->next==nullptr ? obj->fst : obj->act->next; 
    return obj->act; }
    
    NODE* prev() const noexcept { 
        obj->act = obj->act      ==nullptr ? obj->lst :
                   obj->act->prev==nullptr ? obj->lst : obj->act->prev; 
    return obj->act; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
