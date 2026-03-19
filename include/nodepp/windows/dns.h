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

namespace nodepp { namespace dns {

    inline bool is_ipv6( const string_t& URL ){ 
    thread_local static regex_t reg ( "([0-9a-fA-F]+\\:)+[0-9a-fA-F]+" );
        reg.clear_memory(); return reg.test( URL ) ? 1 : 0; 
    }
    
    inline bool is_ipv4( const string_t& URL ){ 
    thread_local static regex_t reg ( "([0-9]+\\.)+[0-9]+" );
        reg.clear_memory(); return reg.test( URL ) ? 1 : 0; 
    }
    
    /*─······································································─*/

    inline bool is_ip( const string_t& URL ){ 
        if( URL.empty() )/*-*/{ return 0; }
        if( is_ipv4(URL) > 0 ){ return 1; }
        if( is_ipv6(URL) > 0 ){ return 1; } return 0;
    }
    
    /*─······································································─*/

    inline string_t lookup( string_t host, int family = AF_UNSPEC ) { 
        _socket_::start_device();

        if( family == AF_INET6 ) {
            if   ( host == "broadcast" || host == "::2" ){ return "::2"; }
            elif ( host == "localhost" || host == "::1" ){ return "::1"; }
            elif ( host == "global"    || host == "::0" ){ return "::0"; }
            elif ( host == "loopback"  || host == "::3" ){ return "::3"; }
        }  else  {
            if   ( host == "broadcast" || host == "255.255.255.255" ){ return "255.255.255.255"; }
            elif ( host == "localhost" || host == "127.0.0.1"       ){ return "127.0.0.1"; }
            elif ( host == "global"    || host == "0.0.0.0"         ){ return "0.0.0.0"; }
            elif ( host == "loopback"  || host == "1.1.1.1"         ){ return "1.1.1.1"; }
        }   if   ( url::is_valid(host) ){ host = url::hostname(host); }

        addrinfo hints, *res, *ptr;  memset( &hints, 0, sizeof(hints) );
        
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family   = family     ;
        hints.ai_flags    = AI_PASSIVE ;

        if( getaddrinfo( host.get(), nullptr, &hints, &res ) != 0 )
          { return nullptr; }

        char ipstr[INET6_ADDRSTRLEN]; string_t ipAddress;

        for( ptr = res; ptr != nullptr; ptr = ptr->ai_next ) {
             void *addr = nullptr;

            if( ptr->ai_family == AF_INET ) {
                addr = &((struct sockaddr_in*) ptr->ai_addr)->sin_addr;
            } elif ( ptr->ai_family == AF_INET6 ) {
                addr = &((struct sockaddr_in6*)ptr->ai_addr)->sin6_addr;
            }

            if( addr ) {
                inet_ntop( ptr->ai_family, addr, ipstr, sizeof(ipstr) );
                ipAddress = ipstr; if ( family != AF_UNSPEC ){ break; }
            }

        }

        freeaddrinfo(res); return ipAddress;
    }
    
    /*─······································································─*/

    inline expected_t<ip_t,except_t> get_host_data(){
        auto socket = socket_t();
            
        socket.SOCK    = SOCK_DGRAM;
        socket.IPPROTO = IPPROTO_UDP;
        socket.socket ( "loopback", 0 );
        socket.connect();

    return socket.get_sockname(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/