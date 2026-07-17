/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_DNS
#define NODEPP_POSIX_DNS

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {  struct dns_t { string_t address, hostname; int family; }; }
namespace nodepp { namespace dns {

    inline bool is_ipv6( const string_t& URL ){ 
    thread_local static regex_t reg ( "([0-9a-f]+\\:)+", true );
        reg.clear_memory(); return reg.test( URL ) ? 1 : 0; 
    }
    
    inline bool is_ipv4( const string_t& URL ){ 
    thread_local static regex_t reg ( "([0-9]+\\.)+[0-9]+" );
        reg.clear_memory(); return reg.test( URL ) ? 1 : 0; 
    }
    
    /*─······································································─*/

    inline bool is_ip( const string_t& URL ){ 
        if( URL.empty( ) ){ return 0; }
        if( is_ipv4(URL) ){ return 1; }
        if( is_ipv6(URL) ){ return 1; } return 0;
    }
    
    /*─······································································─*/

    inline expected_t<ip_t,except_t> get_host_data( int family = AF_INET ){
    _socket_::start_device(); auto socket = socket_t();
            
        socket.SOCK    = SOCK_DGRAM ;
        socket.IPPROTO = IPPROTO_UDP;
        socket.AF      = family     ;
        socket.socket ( "loopback", 0 );
        socket.connect();

    return socket.get_sockname(); }
    
    /*─······································································─*/

    inline ptr_t<dns_t> lookup( string_t host, int family = AF_UNSPEC ) { 
        _socket_::start_device();

        if( regex::test( host, "\\[[^\\]]+\\]" ) ){
            host   = regex::match( host, "[^\\[\\]]+" );
            family = family==AF_UNSPEC ? AF_INET6 : family;
        }

        if( family == AF_INET || family == AF_UNSPEC ) {

            if( host == "localhost" || host == "127.0.0.1" ){ 
                dns_t tmp ({ "127.0.0.1", host, AF_INET }); 
                return ptr_t<dns_t>( 1UL, tmp );
            }
            elif( host == "global"    || host == "0.0.0.0" ){ 
                dns_t tmp ({ "0.0.0.0", host, AF_INET }); 
                return ptr_t<dns_t>( 1UL, tmp );
            }
            elif( host == "loopback"  || host == "1.1.1.1" ){
                dns_t tmp ({ "1.1.1.1", host, AF_INET }); 
                return ptr_t<dns_t>( 1UL, tmp );
            }

        }
        
        if( family == AF_INET6 || family == AF_UNSPEC )  {

            if( host == "localhost" || host == "::1" ){ 
                dns_t tmp ({ "::1", host, AF_INET6 }); 
                return ptr_t<dns_t>( 1UL, tmp );
            }
            elif( host == "global"  || host == "::0" ){ 
                dns_t tmp ({ "::0", host, AF_INET6 }); 
                return ptr_t<dns_t>( 1UL, tmp );
            }
            elif( host == "loopback"|| host == "::2" ){ 
                dns_t tmp ({ "::2", host, AF_INET6 }); 
                return ptr_t<dns_t>( 1UL, tmp );
            }

        }
        
        addrinfo hints, *res, *ptr; memset( &hints, 0, sizeof(hints) );
        if( url::is_valid(host) ){ host = url::hostname(host); }
        
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family   = family     ;
        hints.ai_flags    = AI_PASSIVE ;

        if( getaddrinfo( host.get(), nullptr, &hints, &res ) != 0 )
          { return nullptr; }

        string_t ipAddress ; char ipstr[INET6_ADDRSTRLEN]; 
        queue_t<dns_t> list; void *addr = nullptr;

        for( ptr = res; ptr != nullptr; ptr = ptr->ai_next ) {
            
            if( ptr->ai_family == AF_INET ) {
            if( family == AF_UNSPEC || family == AF_INET ){
                addr = &((struct sockaddr_in*) ptr->ai_addr)->sin_addr;
                inet_ntop( ptr->ai_family, addr, ipstr, sizeof(ipstr) );
                list.unshift( dns_t({ ipstr, host, ptr->ai_family }) );
            }}
            
            elif( ptr->ai_family == AF_INET6 ) {
            if  ( family == AF_UNSPEC || family == AF_INET6 ){
                addr = &((struct sockaddr_in6*)ptr->ai_addr)->sin6_addr;
                inet_ntop( ptr->ai_family, addr, ipstr, sizeof(ipstr) );
                list.push( dns_t({ ipstr, host, ptr->ai_family }) );
            }}

        }

        freeaddrinfo(res); return list.data();
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/