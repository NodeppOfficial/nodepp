/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once
#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace dns { 

    string_t ipv6 = "([0-9a-fA-F]+\\:)+[0-9a-fA-F]+";
    
    string_t ipv4 = "([0-9]+\\.)+[0-9]+";
    
    /*─······································································─*/

    bool is_ipv4( string_t URL ){ return regex::test( URL, ipv4 ) ? 1 : 0; }

    bool is_ipv6( string_t URL ){ return regex::test( URL, ipv6 ) ? 1 : 0; }
    
    /*─······································································─*/

    string_t lookup_ipv6( string_t host ) { _socket_::start_device();

          if( host == "broadcast" || host == "::2" ){ return "::2"; } 
        elif( host == "localhost" || host == "::1" ){ return "::1"; } 
        elif( host == "global"    || host == "::0" ){ return "::0"; }
        elif( host == "loopback"  || host == "::3" ){ return "::3"; }

        if( url::is_valid(host) ){ host = url::hostname(host); }

        addrinfo hints, *res; memset(&hints,0,sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family   = AF_UNSPEC;
        hints.ai_flags    = AI_PASSIVE;

        if( getaddrinfo( host.get(), nullptr, &hints, &res ) != 0 )
          { return nullptr; }

        char ipstr[INET6_ADDRSTRLEN]; string_t ipAddress;
        while ( res != nullptr ) { void *addr = nullptr; 
            if( res->ai_family == AF_INET6 ) {
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
                addr = &(ipv6->sin6_addr);
            } if ( addr != nullptr ){
                 inet_ntop( res->ai_family, addr, ipstr, sizeof(ipstr) );
            }    ipAddress = ipstr; res = res->ai_next;
        }

        freeaddrinfo(res); return ipAddress;
    }
    
    /*─······································································─*/

    string_t lookup_ipv4( string_t host ) { _socket_::start_device();

          if( host == "255.255.255.255" || host == "broadcast" ){ return "255.255.255.255"; } 
        elif( host == "127.0.0.1"       || host == "localhost" ){ return "127.0.0.1"; } 
        elif( host == "0.0.0.0"         || host == "global"    ){ return "0.0.0.0"; }
        elif( host == "1.1.1.1"         || host == "loopback"  ){ return "1.1.1.1"; }

        if( url::is_valid(host) ){ host = url::hostname(host); }

        addrinfo hints, *res; memset(&hints,0,sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family   = AF_UNSPEC;
        hints.ai_flags    = AI_PASSIVE;

        if( getaddrinfo( host.get(), nullptr, &hints, &res ) != 0 )
          { return nullptr; }

        char ipstr[INET_ADDRSTRLEN]; string_t ipAddress;
        while ( res != nullptr ) { void *addr = nullptr; 
            if( res->ai_family == AF_INET ) {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
                addr = &(ipv4->sin_addr);
            } if ( addr != nullptr ){
                 inet_ntop( res->ai_family, addr, ipstr, sizeof(ipstr) );
            }    ipAddress = ipstr; res = res->ai_next;
        }

        freeaddrinfo(res); return ipAddress;
    }
    
    /*─······································································─*/

    string_t lookup( string_t host ) { return lookup_ipv4( host ); }
    
    /*─······································································─*/

    string_t get_hostname(){
        auto socket = socket_t();
            
        socket.SOCK    = SOCK_DGRAM;
        socket.IPPROTO = IPPROTO_UDP;
        socket.socket ( "loopback", 0 );
        socket.connect();

        return socket.get_sockname();
    }
    
    /*─······································································─*/

    bool is_ip( string_t URL ){ 
        if( URL.empty() )     { return 0; }
        if( is_ipv4(URL) > 0 ){ return 1; }
        if( is_ipv6(URL) > 0 ){ return 1; } return 0;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/