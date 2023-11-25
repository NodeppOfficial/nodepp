#ifndef NODEPP_HTTP
#define NODEPP_HTTP

/*────────────────────────────────────────────────────────────────────────────*/

#include "url.h"
#include "tcp.h"
#include "map.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

using http_header_t = map_t< string_t, string_t >;

/*────────────────────────────────────────────────────────────────────────────*/

namespace HTTP_NODEPP {

    string_t _get_http_status( uint status ){
        switch( status ){
            case 100:  return "Continue";                                   break;
            case 101:  return "Switching Protocols";                        break;
            case 102:  return "Processing";                                 break;
            case 103:  return "Early Hints";                                break;
            case 200:  return "OK";                                         break;
            case 201:  return "Created";                                    break;
            case 202:  return "Accepted";                                   break;
            case 203:  return "Non-Authoritative Information";              break;
            case 204:  return "No Content";                                 break;
            case 205:  return "Reset Content";                              break;
            case 206:  return "Partial Content";                            break;
            case 207:  return "Multi-Status";                               break;
            case 208:  return "Already Reported";                           break;
            case 226:  return "IM Used";                                    break;
            case 300:  return "Multiple Choices";                           break;
            case 301:  return "Moved Permanently";                          break;
            case 302:  return "Found";                                      break;
            case 303:  return "See Other";                                  break;
            case 304:  return "Not Modified";                               break;
            case 305:  return "Use Proxy";                                  break;
            case 307:  return "Temporary Redirect";                         break;
            case 308:  return "Permanent Redirect";                         break;
            case 400:  return "Bad Request";                                break;
            case 401:  return "Unauthorized";                               break;
            case 402:  return "Payment Required";                           break;
            case 403:  return "Forbidden";                                  break;
            case 404:  return "Not Found";                                  break;
            case 405:  return "Method Not Allowed";                         break;
            case 406:  return "Not Acceptable";                             break;
            case 407:  return "Proxy Authentication Required";              break;
            case 408:  return "Request Timeout";                            break;
            case 409:  return "Conflict";                                   break;
            case 410:  return "Gone";                                       break;
            case 411:  return "Length Required";                            break;
            case 412:  return "Precondition Failed";                        break;
            case 413:  return "Payload Too Large";                          break;
            case 414:  return "URI Too Long";                               break;
            case 415:  return "Unsupported Media Type";                     break;
            case 416:  return "Range Not Satisfiable";                      break;
            case 417:  return "Expectation Failed";                         break;
            case 418:  return "I'm a Teapot";                               break;
            case 421:  return "Misdirected Request";                        break;
            case 422:  return "Unprocessable Entity";                       break;
            case 423:  return "Locked";                                     break;
            case 424:  return "Failed Dependency";                          break;
            case 425:  return "Too Early";                                  break;
            case 426:  return "Upgrade Required";                           break;
            case 428:  return "Precondition Required";                      break;
            case 429:  return "Too Many Requests";                          break;
            case 431:  return "Request Header Fields Too Large";            break;
            case 451:  return "Unavailable For Legal Reasons";              break;
            case 500:  return "Internal Server Error";                      break;
            case 501:  return "Not Implemented";                            break;
            case 502:  return "Bad Gateway";                                break;
            case 503:  return "Service Unavailable";                        break;
            case 504:  return "Gateway Timeout";                            break;
            case 505:  return "HTTP Version Not Supported";                 break;
            case 506:  return "Variant Also Negotiates";                    break;
            case 507:  return "Insufficient Storage";                       break;
            case 508:  return "Loop Detected";                              break;
            case 509:  return "Bandwidth Limit Exceeded";                   break;
            case 510:  return "Not Extended";                               break;
            case 511:  return "Network Authentication Required";            break;
            default: _Error(regex::format("Status ${0} Not Found",status)); break;
        }
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

class http_t : public socket_t {
protected:

    bool          has_header=0;
    string_t      version;
    
public:

    http_header_t headers;
    int           status;
    query_t       query;

    string_t      protocol;
    string_t      search;
    string_t      method;
    string_t      path;
    string_t      url;
    
    /*─······································································─*/

    http_t() noexcept : socket_t() {}

    http_t( const socket_t& oth ) noexcept : socket_t(oth) {}
    
    /*─······································································─*/

    string_t get_version() const noexcept { return version; }
    
    /*─······································································─*/

    int read_header() noexcept {
        
        if( !is_available() ){ return -1; }
        static array_t<string_t> init; 
        string_t base, line, a, b;
        int idx;

    _Start

        base = read_line(); protocol = "HTTP";
          if( !regex::test( base,"HTTP/\\d\\.\\d" ) ) _End; 
        init = regex::match_all( base, "[^\\s\t\r\n ]+" ); _Next;
        
        if( !regex::test( init[1], "^\\d+" ) ) {
            auto idx = init[1].index_of([]( char x ){ return x=='?'; });
              
            if( idx > 0 ){
                path   = init[1].slice( 0,idx-1 );
                search = init[1].slice( idx + 1 );
                query  = search_params::parse(search);
            } else {
                path   = init[1];
            }

            version = init[2]; method = init[0];
            url     = string::format( "https://%s%s%s", (char*)headers["Host"], (char*)path, (char*)search );
        } else {
            version = init[0]; status = string::to_int(init[1]);
        }   _Next;

        do{ line = read_line(); idx = line.index_of([]( char x ){ return x==':'; });
            if( idx < 0 ) break; a = line.slice( 0,idx-1 ).to_capital_case();
                                 b = line.slice( idx + 1 ); headers[a] = b;
        } while ( true ); _Return(0); _Goto(0);

    _Stop
    }
    
    /*─······································································─*/

    void write_headers( uint status, http_header_t headers ) noexcept {
        if( has_header == 1 ){ return; } has_header = 1;
        string_t res; res += string::format("%s %u %s\r\n",(char*)version,status,(char*)HTTP_NODEPP::_get_http_status(status));
        for( auto x:headers ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
                               res += "\r\n"; write( res ); if( method == "HEAD" ){ close(); }
    }
    
    /*─······································································─*/

    void write_headers( string_t method, string_t path, string_t version, http_header_t headers ) noexcept { 
        if( has_header == 1 ){ return; } has_header = 1;
        string_t res; res += string::format("%s %s %s\r\n",(char*)method,(char*)path,(char*)version);
        for( auto x:headers ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
                               res += "\r\n"; write( res );
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace http {

    template< class T > tcp_t server( const T& cb, agent_t* opt=nullptr ){
        return tcp_t([=]( http_t cli ){ int c=0;
            while(( c=cli.read_header() ) > 0 )
                 { process::next(); }
            if( c==0 ){ cb( cli ); }
            else{ cli.close(); }
        }, opt ); 
    }
    
    /*─······································································─*/

    template< class T > tcp_t client( const T& cb, agent_t* opt=nullptr ){ 
        return tcp_t([=]( http_t cli ){ cb( cli ); }, opt ); 
    }
    
}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif