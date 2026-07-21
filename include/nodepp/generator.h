/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_GENERATOR

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_FILE) && defined(NODEPP_FILE) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_FILE
namespace nodepp { namespace generator { namespace file {

    GENERATOR( read ){
    protected: len_t d; len_t* r;
    public:    string_t data; int state;

    template< class T > coEmit( T* fd, ulong size = NODEPP_CHUNK_SIZE ){
    coBegin; data.clear(); state=0; d=0;

        if  ( !fd->is_available() ) { coEnd; } r=fd->get_range();
        if  ( r[1] != 0  ){ auto pos=fd->pos(); d=min( r[1]-r[0], (len_t)size );
        if  ( pos < r[0] ){ fd->del_borrow(); fd->pos( r[0] ); }
        elif( pos >=r[1] ){ fd->close(); coEnd; }} else { 
              d = (len_t) min( fd->get_buffer_size(), size ); 
        }

        if( fd->get_borrow().empty() ){ 
            coWait((state=fd->_read( fd->get_buffer_data(), fd->get_buffer_size() ))==-2);
        if( state <= 0 )  { fd->close(); coEnd; }  else  { 
            fd->set_borrow( string_t( fd->get_buffer_data(), state ) );
        }}

        state = type::cast<ulong>(d);
        data  = fd->get_borrow().slice( 0, state );
        fd->get_borrow().ptr().slice( state, (ulong) -1 );

    coFinish }};

    /*─······································································─*/

    GENERATOR( write ){
    public: ulong data; int state;

    template< class T > coEmit( T* fd, string_t msg ){
    coBegin state=0; data=0;

        if(!fd->is_available() || msg.empty() ){ coEnd; }

        do{ coWait((state=fd->_write( msg.data()+data, msg.size()-data ))==-2 );
        if( state<=0 ){ fd->close() ; coEnd; } else { 
            data = min( data + state, msg.size() );
        } } while( data < msg.size() );

    coFinish }};

    /*─······································································─*/

    GENERATOR( until ){
    protected: ulong pos  ; file::read _read;
    public:    ulong state; string_t   data ;

    template< class T > coEmit( T* fd, string_t ch ){
    coBegin; state=0; pos=0; data.clear();

        coWait( _read(fd) ==1 );
            if( _read.state<=0 )
              { state = data.size(); coEnd; }
        fd->set_borrow( _read.data );

        do{ for( auto x: _read.data ){ ++state;
            if ( ch[pos]  ==x   ){ ++pos; } else { pos=0; }
            if ( ch.size()==pos ){ break; } }
        } while(0);

        if( memcmp( _read.data.get(), ch.get(), ch.size() )==0 ){
            auto &x = fd->get_borrow();
            data= x.slice( 0, ch.size() );
            /*-*/ x.ptr().slice( ch.size(), (ulong) -1 );
        } elif( state > pos ) {
            auto &x = fd->get_borrow();
            data= x.slice( 0, state - pos );
            /*-*/ x.ptr().slice( state - pos, (ulong) -1 );
        } else { 
            auto &x = fd->get_borrow();
            data= x.slice( 0, state ); 
            /*-*/ x.ptr().slice( state, (ulong) -1 );
        }

        state = data.size();

    coFinish }

    template< class T > coEmit( T* fd, char ch ){
    coBegin; data.clear(); coYield(1); state=0;

        coWait( _read(fd) ==1 );
            if( _read.state<=0 )
              { state = data.size(); coEnd; }
        fd->set_borrow( _read.data );

        do{ for( auto x: _read.data ){ ++state;
            if ( ch ==x ){ break; } continue; }
        } while(0);

        do{ auto &x = fd->get_borrow();
            data += x.slice( 0, state ); 
            /*---*/ x.ptr().slice( state, (ulong) -1 );
            state = data.size();
        } while(0);

        if( data[ data.size()-1 ] == ch ){ coEnd; }

    coGoto(1) ; coFinish }};

    /*─······································································─*/

    GENERATOR( line ){
    protected: file::read _read;
    public:    ulong      state; string_t data; 

    template< class T > coEmit( T* fd ){
    coBegin data.clear(); coYield(1); state=0;

        coWait( _read( fd )==1 );
        if    ( _read.state<=0 ){ state = data.size(); coEnd; }

        fd->set_borrow(_read.data);

        do{ for( auto x: _read.data ){ ++state;
            if ('\n'==x ){ break; } continue; }
        } while(0);

        data +=fd->get_borrow().splice( 0, state );
        state =data.size();
        
        if( data[data.size()-1] == '\n' ){ coEnd; }

    coGoto(1) ; coFinish }};

}}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_STREAM) && defined(NODEPP_STREAM) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_STREAM
namespace nodepp { namespace generator { namespace stream {

    GENERATOR( duplex ){
    protected:

        file::write _write1, _write2;
        file::read  _read1 , _read2;

    public:

        template< class T, class V > coEmit( const T& inp, const V& out ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();
        
        coYield(1);

            while ( inp.is_available() && out.is_available() ){
            while ( _read1 (&inp)==1 ){ coGoto(2); } if( _read1 .state<=0 ){ break; }
            coWait( _write1(&out,_read1.data)==1 );  if( _write1.state<=0 ){ break; }
                inp.onData.emit( _read1.data );
            }   inp.close(); out.close();

            coEnd; coYield(2);

            while ( inp.is_available() && out.is_available() ){
            while ( _read2 (&out)==1 ){ coGoto(1); } if( _read2 .state<=0 ){ break; }
            coWait( _write2(&inp,_read2.data)==1 );  if( _write2.state<=0 ){ break; }
                out.onData.emit( _read2.data );
            }   
            
            out.close(); inp.close();

        coFinish }

    };

    /*─······································································─*/

    GENERATOR( pipe ){
    protected:

        file::write _write;
        file::read  _read ;

    public:

        template< class T > coEmit( const T& inp ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();

            while ( inp.is_available() ){
            coWait( _read(&inp) ==1 ); if( _read.state<=0 ){ break; }
                inp.onData.emit(_read.data);
            }   
            
            inp.close();

        coFinish }

        template< class T, class V > coEmit( const T& inp, const V& out ){
        coBegin 

            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();

            while ( inp.is_available() && out.is_available() ){
            coWait( _read (&inp)==1 ); /*------*/ if( _read .state<=0 ){ break; }
            coWait( _write(&out,_read.data)==1 ); if( _write.state<=0 ){ break; }
                inp.onData.emit(_read.data);
            }   
            
            inp.close(); out.close();

        coFinish }

    };

    /*─······································································─*/

    GENERATOR( until ){
    protected:

        file::write _write;
        file::until _read ;

    public:

        template< class T, class U >
        coEmit( const T& inp, const U& val ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            
            while ( inp.is_available() ){
            coWait( _read(&inp,val)==1 ); if( _read.state<=0 ){ break; }
                inp.onData.emit(_read.data);
            }   
            
            inp.close();
        
        coFinish }

        template< class T, class V, class U >
        coEmit( const T& inp, const V& out, const U& val ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();
            
            while( inp.is_available() && out.is_available() ){
            coWait( _read (&inp,val)==1 ); /*--*/ if( _read .state<=0 ){ break; }
            coWait( _write(&out,_read.data)==1 ); if( _write.state<=0 ){ break; }
                inp.onData.emit(_read.data);
            }   
            
            inp.close(); out.close();
        
        coFinish }

    };

    /*─······································································─*/

    GENERATOR( line ){
    protected:

        file::write _write;
        file::line  _read ;

    public:

        template< class T > coEmit( const T& inp ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
        
            while( inp.is_available() ){
            coWait( _read(&inp)==1 ); if( _read.state<=0 ){ break; }
                inp.onData.emit(_read.data);
            }     
            
            inp.close();
        
        coFinish }

        template< class T, class V > coEmit( const T& inp, const V& out ){
        coBegin 
            
            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();
        
            while( inp.is_available() && out.is_available() ){
            coWait( _read (&inp)==1 ); /*------*/ if( _read .state<=0 ){ break; }
            coWait( _write(&out,_read.data)==1 ); if( _write.state<=0 ){ break; }
                inp.onData.emit(_read.data);
            }   
            
            inp.close(); out.close();
        
        coFinish }

    };

}}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_ZLIB) && defined(NODEPP_ZLIB) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_ZLIB
namespace nodepp { namespace generator { namespace zlib {

    GENERATOR( pipe_inflate ){
    protected:

        file::write _write;
        file::read  _read ; string_t borrow;

    public:

        template< class Z, class T, class V > coEmit( const Z& zlb, const T& inp, const V& out ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();

            while ( inp.is_available() && out.is_available() ){
            coWait( _read (&inp)==1 ); /*-----*/ if( _read .state<=0 ){ break; }
                borrow = zlb.update_inflate(_read.data);
            coWait( _write( &out, borrow )==1 ); if( _write.state<=0 ){ break; }
                inp.onData.emit( borrow );
            }   
            
            inp.close(); out.close();

        coFinish }

        template< class Z, class T > coEmit( const Z& zlb, const T& inp ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();

            while ( inp.is_available() ){
            coWait( _read(&inp)==1 ); if( _read.state<=0 ){ break; }
                borrow = zlb.update_inflate( _read.data );
                inp.onData.emit( borrow );
            }   
            
            inp.close();

        coFinish }

    };

    GENERATOR( pipe_deflate ){
    protected:

        file::write _write;
        file::read  _read ; string_t borrow;

    public:

        template< class Z, class T, class V > coEmit( const Z& zlb, const T& inp, const V& out ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();
        
            while ( inp.is_available() && out.is_available() ){
            coWait( _read (&inp)==1 ); /*-----*/ if( _read .state<=0 ){ break; }
                borrow = zlb.update_deflate(_read.data);
            coWait( _write( &out, borrow )==1 ); if( _write.state<=0 ){ break; }
                inp.onData.emit( borrow );
            }   
            
            inp.close(); out.close();
        
        coFinish }

        template< class Z, class T > coEmit( const Z& zlb, const T& inp ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            
            while ( inp.is_available() ){
            coWait( _read(&inp)==1 ); if( _read.state<=0 ){ break; }
                borrow = zlb.update_deflate(_read.data);
                inp.onData.emit( borrow );
            }   
            
            inp.close();

        coFinish }

    };

}}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_HTTP) && defined(NODEPP_HTTP) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_HTTP

namespace nodepp { namespace generator { namespace http {
    
    GENERATOR( read ){
    private:

        enum FLAG {
            HTTP_FLAG_UNKNOWN = 0b00000000,
            HTTP_FLAG_CHUNKED = 0b00000001,
            HTTP_FLAG_STREAM  = 0b00000010,
        };

    public: 
    
        string_t borrow; ulong data=0;

    public: 

        template< class T, class V >
        int chunk_http_chunked( T* fd, char* bf, ulong sx, V& mode ){

            auto &bff = borrow; do { if( !bff.empty() ) { if( mode.size==0 ){

                if( bff.starts_with("\r\n") ){ 
                    bff.ptr().slice( 2, (ulong) -1 ); 
                }

                auto x = bff.find("\r\n"); if( x.null() ){ break; }
                auto y = bff.slice_view( 0, x[0] ).find(";");

                if( y.null() ){
                    mode.size = encoder::hex::btoa<len_t>( bff.slice_view( 0, x[0] ) );
                } else {
                    mode.size = encoder::hex::btoa<len_t>( bff.slice_view( 0, y[0] ) );
                }
                
                if( mode.size==0 ){ data=0; return -1; }
                bff.ptr().slice( x[1], (ulong) -1 );

            }   auto sy  = min( mode.size, (len_t)sx );
                auto tmp = bff.slice_view( 0, sy );
                auto c   = tmp.size();

                mode.size -= min( (len_t)c, mode.size );
                memmove( bf, tmp.get(), c ); 

                bff.ptr().slice( c, (ulong) -1 ); 
                data = c; return -1;

            }} while(0); 
            
            int   c = fd->__read ( bf, sx );
            if  ( c > 0 ){ bff += string_t( bf, c ); }
            elif( c==-2 ){ data = 0; /**/ return  1; }
            else         { data = 0; /**/ return -1; }
                           data = 0; /**/ return  1;

        }

        template< class T, class V >
        int stream_http_stream( T* fd, char* bf, ulong sx, V& mode ){
            
            if( mode.size == 0 ){ data=0; return -1; }

            if( borrow.empty() ){

                int c = fd->__read( bf, min( mode.size, (len_t)sx ) );

                if( c==-2 ){ data=0; return  1; }
                if( c<= 0 ){ data=0; return -1; }

                mode.size -= min( mode.size, (len_t)c );
                data = c; return -1;
            
            } else {

                string_t tmp = borrow.slice( 0, mode.size ); 
                borrow.ptr().slice( mode.size, (ulong) -1 );
                auto c = tmp.size();

                memmove( bf, tmp.get(), tmp.size() );
                mode.size -= min( mode.size, (len_t)c );

                data = c; return -1;

            }

        }

        template< class T, class V >
        int default_http_stream( T* fd, char* bf, ulong sx, V& mode ){
            
            if( borrow.empty() ){

                int c = fd->__read( bf, sx );

                if( c==-2 ){ data=0; return  1; }
                if( c<= 0 ){ data=0; return -1; }
                             data=c; return -1;

            } else {

                string_t tmp = borrow.slice( 0, sx );
                memmove( bf, tmp.get(), tmp.size() );

                borrow.ptr().slice( sx, (ulong) -1 );
                data = tmp.size(); /*-*/ return -1;

            }

        }

        template< class T, class V >
        coEmit( T* fd, char* bf, ulong sx, V& mode ){
        switch( mode.state ){

            case FLAG::HTTP_FLAG_STREAM: 
            return stream_http_stream ( fd, bf, sx, mode ); break;

            case FLAG::HTTP_FLAG_CHUNKED:
            return chunk_http_chunked ( fd, bf, sx, mode ); break;

            default: 
            return default_http_stream( fd, bf, sx, mode ); break;

        }}

    };

    GENERATOR( write ){
    private:

        enum FLAG {
            HTTP_FLAG_UNKNOWN = 0b00000000,
            HTTP_FLAG_CHUNKED = 0b00000001,
            HTTP_FLAG_STREAM  = 0b00000010,
        };

        string_t borrow; ulong size;

    public: 
    
        ulong data=0;

    public: 

        template< class T, class V >
        int chunk_http_chunked( T* fd, char* bf, ulong sx, V& mode ){

            do { if( borrow.empty() ){

                borrow = encoder::hex::atob( sx ) + "\r\n" + string_t( bf, sx ) + "\r\n"; 
                size   = 0UL; 

            }   int c = fd->_write_( borrow.get(), borrow.size(), &size );

                if( c==-2 ){ break ; }
                
                if( borrow.size ()==size ){ 
                    borrow.clear();
                    data = sx; return -1;
                }
            
                        data = 0; return -1;
            } while(0); data = 0; return  1;

        }

        template< class T, class V >
        int stream_http_stream( T* fd, char* bf, ulong sx, V& mode ){
            
            if( mode.size > 0 ){

                int c = fd->__write( bf, min( mode.size, (len_t)sx ) );

                if( c==-2 ){ data=0; return  1; }
                if( c<= 0 ){ data=0; return -1; }

                mode.size -= min( mode.size, (len_t)c );
                data = c; return -1;
            }   data = 0; return -1;

        }

        template< class T, class V >
        int default_http_stream( T* fd, char* bf, ulong sx, V& mode ){

            int c = fd->__write( bf, sx );

            if( c==-2 ){ data=0; return  1; }
            if( c<= 0 ){ data=0; return -1; }

            data = c; return -1;

        }

        template< class T, class V >
        coEmit( T* fd, char* bf, ulong sx, V& mode ){
        switch( mode.state ){

            case FLAG::HTTP_FLAG_STREAM: 
            return stream_http_stream ( fd, bf, sx, mode ); break;

            case FLAG::HTTP_FLAG_CHUNKED:
            return chunk_http_chunked ( fd, bf, sx, mode ); break;

            default: 
            return default_http_stream( fd, bf, sx, mode ); break;

        }}

    };

}}}

#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_WS) && defined(NODEPP_GENERATOR) && ( defined(NODEPP_WS) || defined(NODEPP_WSS) )
#define GENERATOR_WS
    #include "encoder.h"
    #include "crypto.h"
namespace nodepp { namespace generator { namespace ws {

    struct ws_frame_t {
        bool     FIN; //1b
        uchar    RSV; //3b
        uchar    OPC; //4b
        bool     MSK; //1b
        uchar_32 KEY; //4B
        len_t    LEN; //8B
    };

    /*─······································································─*/

    template< class T > bool server( T& cli ) { do {
        auto data = cli.read(); int c=0; 
        cli.set_borrow( data );

        while((c=cli.read_header())==1 ){
        if   ( cli.is_waiting() ){ process::next(); }}
        
        if( c!=0 ) /*----------------*/ { break; }
        if( cli.headers.has("Sec-Websocket-Key") ){

            string_t sec = cli.headers["Sec-Websocket-Key"];
                auto sha = crypto::hash::SHA1(); sha.update( sec + NODEPP_WS_SECRET );
            string_t enc = encoder::base64::get( encoder::buffer::hex2buff(sha.get()) );

            cli.write_header( 101, header_t({
                { "Sec-Websocket-Accept", enc },
                { "Connection", "upgrade"     },
                { "Upgrade"   , "websocket"   }
            }) );

            cli.stop(); return true;
        }   cli.set_borrow( data );

    } while(0); return false; }

    /*─······································································─*/

    template< class T > bool client( T& cli, string_t url ) { do {
        string_t hsh = encoder::key::generate("abcdefghiABCDEFGHI0123456789",22);
        string_t key = string::format("%s==",hsh.data());

        header_t header ({
            { "Upgrade"   , "websocket" },
            { "Connection", "upgrade"   },
            { "Sec-Websocket-Key", key  },
            { "Sec-Websocket-Version", "13" }
        });

        cli.write_header( "GET", url::path(url), "HTTP/1.1", header );
        int c=0; 

        while((c=cli.read_header())==1 ){
        if   ( cli.is_waiting() ){ process::next(); }}

        if( c != 0 ){
            cli.onError.emit("Could not connect to server");
            cli.close(); break;
        }

        if( cli.status != 101 ){
            cli.onError.emit(string::format("Can't connect to WS Server -> status %d",cli.status));
            cli.close(); break;
        }

        if( cli.headers.has("Sec-Websocket-Accept") ){

            string_t dta = cli.headers["Sec-Websocket-Accept"];
                auto sha = crypto::hash::SHA1(); sha.update( key + NODEPP_WS_SECRET );
            string_t enc = encoder::base64::get( encoder::buffer::hex2buff(sha.get()) );

            if( dta != enc ){
                cli.onError.emit("secret key does not match"); 
                cli.close(); break;
            }   cli.stop (); return true;

        }

    } while(0); return false; }

    /*─······································································─*/

    GENERATOR( read ){
    protected:

        ulong size=0, key=0;
        ws_frame_t frame; string_t borrow;

    public:

        ulong data=0;

    protected:

        bool read_ws_frame( string_t borrow, ulong& size ) { 

            frame={0}; size=0;

            if ( borrow.size() < 2 ){ return false; } do { 

                auto y = array_t<bool>(encoder::bin::get( borrow[size] )); size++;

                frame.FIN   = y.slice_view(0,1)[0] == 1;
                for( auto x : y.slice_view(1,4) ){ frame.RSV = frame.RSV<<1 | x; }
                for( auto x : y.slice_view(4,8) ){ frame.OPC = frame.OPC<<1 | x; }

            } while(0); do {

                auto y = array_t<bool>(encoder::bin::get( borrow[size] )); size++;

                frame.MSK   = y.slice_view(0,1)[0] == 1;
                for( auto x : y.slice_view(1,8) ){ frame.LEN = frame.LEN<<1 | x; }

            } while(0);

            if( frame.LEN  > 125 ){ do {
            if( frame.LEN == 126 ){ auto mem = (char*) &frame.LEN; auto len = sizeof(uchar_16);
            if( borrow.size() < size + sizeof(uchar_16) ){ frame = {0}; return false; }
                type::copy_reverse( borrow.get() + size, borrow.get() + size + len, mem );
                size += len; break;
            } 
            if( frame.LEN == 127 ){ auto mem = (char*) &frame.LEN; auto len = sizeof(uchar_64);
            if( borrow.size() < size + sizeof(uchar_64) ){ frame = {0}; return false; }
                type::copy_reverse( borrow.get() + size, borrow.get() + size + len, mem );
                size += len; break;
            }} while(0); }

            if( frame.MSK == 1 ){
            if( borrow.size() < size + sizeof(uchar_32) ){ frame = {0}; return false; }
                uchar_32* mem = (uchar_32*)( borrow.get()+ size );
                frame.KEY = type::cast<uchar_32>( *mem ) ; size += sizeof(uchar_32); 
            }

        return true; }
        
        string_t pong_frame() const noexcept { return ptr_t<char>({ 0x8A, 0x00 }); }
        string_t ping_frame() const noexcept { return ptr_t<char>({ 0x89, 0x00 }); }
        string_t end_frame () const noexcept { return ptr_t<char>({ 0x88, 0x00 }); }

    public:

    template< class T > 
    coEmit( T* fd, char* bf, const ulong& sx ) {

        auto &bff = borrow;

        do{ if( !bff.empty() ){ if( frame.LEN ==0 ){
            if( !read_ws_frame( bff, size ) ){ break; }

            bff.ptr().slice( size, (ulong) -1 ); key = 0;

            if( frame.OPC ==  8 ){ data=0; fd->write( end_frame () ); return -1; }
            if( frame.OPC >= 20 ){ data=0; fd->write( end_frame () ); return -1; }
            if( frame.OPC ==  9 ){ data=0; fd->write( pong_frame() ); }

            if( frame.LEN==0 ){ data=0; return 1; }
        }   if( bff.empty()  ){ break ; }  

            auto sy  = min( frame.LEN, (len_t)sx );
            auto tmp = bff.slice( 0, sy );
            auto c   = tmp.size();

            if( frame.MSK ){ for( auto &x: tmp ){
                x  ^= type::cast<uchar>( &frame.KEY )[key]; 
                key = ( key + 1 ) % sizeof( uchar_32 );
            }}

            frame.LEN -= min( (len_t)c, frame.LEN );
            bff.ptr().slice( c, (ulong) -1 ); 
            memmove( bf, tmp.get(), c ); 

            if( frame.OPC >= 9 || 
              ( frame.OPC >= 3 && frame.OPC <= 7 )
            ) { data = 0; return  1; }
                data = c; return -1;

        }} while(0);

        int /*----*/ c = fd->__read ( bf, sx );

        if  ( c > 0 ){ bff += string_t ( bf, c ); }
        elif( c==-2 ){ data = 0; /*-*/ return  1; }
        else         { data = 0; /*-*/ return -1; }

        data = 0; return 1;

    }};

    GENERATOR( write ){
    protected:
            uchar_32* mask; string_t borrow; 
            ulong size=0  ; ptr_t<char> bfx;
    public: ulong data=0  ;

    protected:

        ulong write_ws_frame( char* bf, ulong sx, uchar opcode, uchar_32* mask ) {
        ulong idx = 0;

            if( opcode == 0 ){ bool b=0; for ( ulong x=0; x<sx; x++ ){
            if( !string::is_print( bf[x] ) ){ b=1; break; }}
                     bfx[idx] = !b? 0x82:0x81;
            } else { bfx[idx] = 0x80 | opcode; } ++idx; 
            
            bfx[idx] = *mask == 0UL ? 0x00 : 0x80;

            if ( sx < 126 ){
                bfx[idx]|= type::cast<uchar>( sx ); ++idx;
            } elif ( sx < 65536 ){
                bfx[idx]|= (uchar)( 126 ); ++idx; 
                auto mem = (char*) &sx; auto len = sizeof(uchar_16);
                type::copy_reverse( mem, mem + len, bfx.get() + idx ); idx += len;
            } else {
                bfx[idx]|= (uchar)( 127 ); ++idx; 
                auto mem = (char*) &sx; auto len = sizeof(uchar_64);
                type::copy_reverse( mem, mem + len, bfx.get() + idx ); idx += len;
            } 
            
            if( *mask!= 0UL ) {
                auto mem = (uchar_32*)( bfx.get() + idx );
                   * mem = *mask; idx += sizeof( uchar_32 );
            }

        return idx; }

    public: write() noexcept : bfx( 16UL ) {}

        template< class T > 
        coEmit( T* fd, char* bf, const ulong& sx ) {

            do { if( borrow.empty() ){

                mask= &fd->get_mask(); ulong sy=write_ws_frame( bf, sx, 0, mask );

                if ( *mask != 0UL ){ ulong sy=0; char* key = (char*) mask;
                for( char *y = bf ; y<bf + sx ; y++ ){ 
                     /**/ *y^= key[ sy++ % sizeof (uchar_32) ];
                }  } if ( *mask != 0UL ){ *mask = rand(); }

                borrow = string_t( bfx.get(), sy ) + string_t( bf, sx );
                size   = 0UL; 

            }   int c = fd->_write_( borrow.get(), borrow.size(), &size );

                if( c==-2 ){ break ; }
                
                if( borrow.size ()==size ){ 
                    borrow.clear();
                    data = sx; return -1;
                }
            
                        data = 0; return -1;
            } while(0); data = 0; return  1;

        }

    };

}}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/