/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_INITIALIZER
#define NODEPP_INITIALIZER

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > class initializer_t : public ptr_t<T> { 
    public: initializer_t() noexcept : ptr_t<T>(){}

    /*─······································································─*/
    
    template< class V, class = typename type::enable_if<!type::is_same<V,string_t>::value,T>::type >
    initializer_t( const V& arr ) noexcept {
        this->resize( arr.size() ); for( ulong i=0; i<arr.size(); i++ )
            { this->value_[i] = type::cast<T>( arr[i] ); }
    }
    
    initializer_t( const array_t<T>& arr ) noexcept {
        this->resize( arr.size() ); for( ulong i=0; i<arr.size(); i++ )
            { this->value_[i] = arr[i]; }
    }

    template< class... V >
    initializer_t( const T& head, const V&... tail ) noexcept {
        if( this->empty() ) this->resize( sizeof...(V) + 1 );
        ulong index = 0; iterator::map([&]( const T& item ){
            this->value_[index] = (T)item; index++;
        }, head, tail... );
    }

    template< class V, ulong N >
    initializer_t( const V (&arr) [N] ) noexcept {
        this->resize( N ); for( ulong i=0; i<N; i++ )
            { this->value_[i] = (T)arr[i]; }
    }
    
    initializer_t( const ptr_t<T>& arr ) noexcept : ptr_t<T>( arr ) {}

    /*─······································································─*/

    long index_of( function_t<bool,T> func ) const noexcept { long i=0;
        for( auto& x : *this ){ if( func(x) ) return i; i++; } return -1;
    }

    ulong count( function_t<bool,T> func ) const noexcept { ulong n=0; 
        for( auto& x : *this ){ if( func(x) ) n++; } return n;
    }
    
    /*─······································································─*/

    T reduce( function_t<T,T,T> func ) const noexcept { T act = (*this)[0];
        for( auto x=this->begin() + 1; x != this->end(); x++ )
           { act = func( act, *x ); } return act;
    }

    bool some( function_t<bool,T> func ) const noexcept { 
        for( auto& x : *this ){ if( func(x) ) return 1; } return 0;
    }

    bool none( function_t<bool,T> func ) const noexcept { 
        for( auto& x : *this ){ if( func(x) ) return 0; } return 1;
    }

    bool every( function_t<bool,T> func ) const noexcept { 
        for( auto& x : *this ){ if( func(x) ) return 0; } return 1;
    }

    void map( function_t<void,T&> func ) const noexcept { 
        for( auto& x : *this ){ func(x); }
    }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif