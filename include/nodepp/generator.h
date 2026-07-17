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
    protected: ulong d; len_t* r;
    public:    string_t data; int state;

    template< class T > coEmit( T* fd, ulong size = NODEPP_CHUNK_SIZE ){
    coBegin; data.clear(); state=0; d=0;

        if( !fd->is_available()       ){ coEnd; } r=fd->get_range ();
        if( !fd->get_borrow().empty() ){ data =/*-*/fd->get_borrow(); }

        if( r[1] != 0  ){ auto pos=fd->pos(); d=r[1]-r[0];
        if( pos < r[0] ){ fd->del_borrow(); fd->pos(r[0]); }
      elif( pos >=r[1] ){ fd->close(); coEnd; }} else { d = fd->get_buffer_size(); }

        if( data.empty() ){ 
            coWait((state=fd->_read(fd->get_buffer_data(),min(d,size)))==-2);
        if( state <= 0 ) { fd->close(); coEnd; } else { 
            data=string_t( fd->get_buffer_data(), state );
        }}

        state=/**/ min( data.size(), size );
        fd->set_borrow( data.splice( state, data.size() ) );

    coFinish }};

    /*─······································································─*/

    GENERATOR( write ){
    public: ulong data; int state;

    template< class T > coEmit( T* fd, string_t msg ){
    coBegin state=0; data=0;

        if(!fd->is_available() || msg.empty() ){ coEnd; }

        do{ coWait((state=fd->_write( msg.data()+data, msg.size()-data ))==-2 );
        if( state<=0 ){ fd->close(); coEnd; } else { data += state; }} 
        while( state>=0 && data<msg.size() );

    coFinish }};

    /*─······································································─*/

    GENERATOR( until ){
    protected: ulong pos; file::read _read;
    public: int state; string_t data;

    template< class T > coEmit( T* fd, string_t ch ){
    coBegin; state=0; pos=0; data.clear();

        coWait( _read(fd) ==1 );
            if( _read.state<=0 )
              { state = data.size(); coEnd; }
        fd->set_borrow( _read.data );

        do{for( auto x: _read.data ){ ++state;
           if ( ch[pos]  ==x   ){ ++pos; } else { pos=0; }
           if ( ch.size()==pos ){ break; } }
        } while(0);

        if( memcmp( _read.data.get(), ch.get(), ch.size() )==0 ){
                 data=fd->get_borrow().splice( 0, ch.size() );
        } elif( (ulong) state > pos ) {
                 data=fd->get_borrow().splice( 0, state-pos );
        } else { data=fd->get_borrow().splice( 0, state     ); }

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

        data +=fd->get_borrow().splice( 0, state );
        state =data.size();

        if( data[ data.size()-1 ] == ch ){ coEnd; }

    coGoto(1) ; coFinish }};

    /*─······································································─*/

    GENERATOR( line ){
    protected: file::read _read;
    public: string_t data; int state;

    template< class T > coEmit( T* fd ){
    coBegin data.clear(); coYield(1); state=0;

        coWait( _read(fd) ==1 );
            if( _read.state<=0 )
              { state = data.size(); coEnd; }
        fd->set_borrow(_read.data);

        do{ for( auto x: _read.data ){ ++state;
             if('\n'==x ){ break; } continue; }
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

            while( inp.is_available() && out.is_available() ){
            while( _read1(&inp) ==1 )            { coGoto(2); }
               if( _read1.state <=0 )            { break;  }
           coWait( _write1(&out,_read1.data)==1 );
               if( _write1.state<=0 )            { break;  }
                    inp.onData.emit( _read1.data );
            }       inp.close(); out.close();

            coEnd; coYield(2);

            while( inp.is_available() && out.is_available() ){
            while( _read2(&out) ==1 )            { coGoto(1); }
               if( _read2.state <=0 )            { break;  }
           coWait( _write2(&inp,_read2.data)==1 );
               if( _write2.state<=0 )            { break;  }
                    out.onData.emit( _read2.data );
            }       out.close(); inp.close();

        coFinish }

    };

    /*─······································································─*/

    GENERATOR( pipe ){
    protected:

        file::write _write;
        file::read  _read;

    public:

        template< class T > coEmit( const T& inp ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();

            while( inp.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break;  }
                    inp.onData.emit(_read.data);
            }       inp.close();

        coFinish }

        template< class T, class V > coEmit( const T& inp, const V& out ){
        coBegin 

            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();

            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break;  }
           coWait( _write(&out,_read.data)==1 );
               if( _write.state<=0 ){ break;  }
                    inp.onData.emit(_read.data);
            }       inp.close(); out.close();

        coFinish }

    };

    /*─······································································─*/

    GENERATOR( until ){
    protected:

        file::write _write;
        file::until  _read;

    public:

        template< class T, class U >
        coEmit( const T& inp, const U& val ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            
            while( inp.is_available() ){
           coWait( _read(&inp,val)==1 );
               if( _read.state <=0 ){ break; }
                   inp.onData.emit(_read.data);
            }      inp.close();
        
        coFinish }

        template< class T, class V, class U >
        coEmit( const T& inp, const V& out, const U& val ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();
            
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp,val)==1 );
               if( _read.state  <=0 ){ break; }
           coWait( _write(&out,_read.data)==1 );
               if( _write.state <=0 ){ break; }
                    inp.onData.emit(_read.data);
            }       inp.close(); out.close();
        
        coFinish }

    };

    /*─······································································─*/

    GENERATOR( line ){
    protected:

        file::write _write;
        file::line  _read;

    public:

        template< class T > coEmit( const T& inp ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
        
            while( inp.is_available() ){
           coWait( _read(&inp)==1 );
               if( _read.state<=0 ){ break;  }
                   inp.onData.emit(_read.data);
            }      inp.close();
        
        coFinish }

        template< class T, class V > coEmit( const T& inp, const V& out ){
        coBegin 
            
            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();
        
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break;  }
           coWait( _write(&out,_read.data)==1 );
               if( _write.state<=0 ){ break;  }
                    inp.onData.emit(_read.data);
            }       inp.close(); out.close();
        
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
        file::read  _read;
        string_t borrow;

    public:

        template< class Z, class T, class V > coEmit( const Z& zlb, const T& inp, const V& out ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();

            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break; }
           borrow = zlb.update_inflate(_read.data);
           coWait( _write( &out, borrow )==1 );
               if( _write.state<=0 ){ break; }
                    inp.onData.emit( borrow );
            }       inp.close(); out.close();

        coFinish }

        template< class Z, class T > coEmit( const Z& zlb, const T& inp ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();

            while( inp.is_available() ){
           coWait( _read(&inp)==1 );
               if( _read.state<=0 ){ break; }
            borrow = zlb.update_inflate(_read.data);
                    inp.onData.emit( borrow );
            }       inp.close();

        coFinish }

    };

    GENERATOR( pipe_deflate ){
    protected:

        file::write _write;
        file::read  _read;
        string_t borrow;

    public:

        template< class Z, class T, class V > coEmit( const Z& zlb, const T& inp, const V& out ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            out.onPipe.emit(); out.resume();
        
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break; }
           borrow = zlb.update_deflate(_read.data);
           coWait( _write( &out, borrow )==1 );
               if( _write.state<=0 ){ break; }
                    inp.onData.emit( borrow );
            }       inp.close(); out.close();
        
        coFinish }

        template< class Z, class T > coEmit( const Z& zlb, const T& inp ){
        coBegin 
        
            inp.onPipe.emit(); inp.resume();
            
            while( inp.is_available() ){
           coWait( _read(&inp)==1 );
               if( _read.state<=0 ){ break; }
            borrow = zlb.update_deflate(_read.data);
                    inp.onData.emit( borrow );
            }       inp.close();

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

            auto &bff = borrow; do { if( mode.size==0 ){ 

                if( bff.starts_with("\r\n") ){ bff.splice( 0, 2 ); }

                auto x = bff.find("\r\n"); if( x.null() ){ break; }
                auto y = bff.slice_view( 0, x[0] ).find(";");

                if( y.null() ){
                    mode.size = encoder::hex::btoa<len_t>( bff.slice_view( 0, x[0] ) );
                } else {
                    mode.size = encoder::hex::btoa<len_t>( bff.slice_view( 0, y[0] ) );
                }
                
                if( mode.size==0 ){ data=0; return -1; } bff.splice( 0, x[1] );

            } if( bff.empty() ) { break; }

                auto sy  = min( mode.size, (len_t)sx );
                auto tmp = bff.slice_view( 0, sy );
                auto c   = tmp.size();

                mode.size -= min( (len_t)c, mode.size );
                memcpy( bf, tmp.get(), c ); 

                bff.splice ( 0, c );
                data = c; return -1;

            } while( 0 );
            
            int /*----*/ c = fd->__read ( bf, sx );

            if  ( c > 0 ){ bff += string_t( bf, c  ); }
            elif( c==-2 ){ data = 0; /*-*/ return  1; }
            else         { data = 0; /*-*/ return -1; }

            data = 0; return 1;

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

                string_t tmp = borrow.splice( 0, mode.size ); 
                auto c = tmp.size();

                memcpy( bf, tmp.get(), tmp.size() );
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

                string_t tmp = borrow.splice( 0, sx );
                memcpy( bf, tmp.get(), tmp.size() );
                data = tmp.size(); /*--*/ return -1;

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

        string_t bff; ulong size;

    public: 
    
        ulong data=0;

    public: 

        template< class T, class V >
        int chunk_http_chunked( T* fd, char* bf, ulong sx, V& mode ){

            if( bff.empty() ){ 

                bff = encoder::hex::atob( sx ) + "\r\n" + string_t( bf, sx ) + "\r\n"; 
                size= 0UL;

            } else {

                int c = fd->_write_( bff.get(), bff.size(), &size );

                if( c==-2 ){ data=0; return  1; }
                if( c<= 0 ){ data=0; return -1; } 
                
                if( bff.size()==size ){ bff.clear(); size=0UL; }
            
                data = c; return -1;
            }   data = 0; return  1;

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
        bool   FIN;     //1b
        uint   RSV;     //3b
        uint   OPC;     //4b
        bool   MSK;     //1b
        char   KEY [4]; //4B
        len_t  LEN;     //64b
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

        ulong size=0, len=0, key=0, sz=0;
        ws_frame_t frame;

    public:

        ulong data=0;

    protected:

        void read_ws_hdr_frame( char* bf, ulong& size ) { 
            
            size=0; do { array_t<bool> y;

                y = array_t<bool>(encoder::bin::get( bf[0] ));

                frame.FIN   = y.splice(0,1)[0] == 1;
                for( auto x : y.splice(0,3) ) frame.RSV = frame.RSV<<1 | x;
                for( auto x : y.splice(0,4) ) frame.OPC = frame.OPC<<1 | x;

                y = array_t<bool>(encoder::bin::get( bf[1] ));

                frame.MSK   = y.splice(0,1)[0] == 1;
                for( auto x : y.splice(0,7) ) frame.LEN = frame.LEN<<1 | x;

            } while(0);

            if ( frame.LEN  > 125 ){
            if ( frame.LEN == 126 ){ size =2; }
            if ( frame.LEN == 127 ){ size =8; }}
            if ( frame.MSK == 1   ){ size+=4; }

        }

        void read_ws_hdr_lensk( char* bf, ulong& size ) {

            if ( frame.MSK == 1 ){ size -= 4;
            for( ulong x=0; x<4; ++x ){ frame.KEY[x] = bf[x+size]; }}

            if ( frame.LEN  > 125 ){ /*---*/ frame.LEN=0;
            for( ulong x=0; x < size; ++x ){ frame.LEN=frame.LEN << 8 | (uchar) bf[x]; }}

        }
        
        string_t pong_frame() const noexcept { return ptr_t<char>({ 0x8A, 0x00 }); }
        string_t ping_frame() const noexcept { return ptr_t<char>({ 0x89, 0x00 }); }
        string_t end_frame () const noexcept { return ptr_t<char>({ 0x88, 0x00 }); }

    public:

    template<class T> coEmit( T* fd, char* bf, const ulong& sx ) {
    coBegin ; memset( bf, 0, sx ); size=0; data=0; len=0; key=0;
              memset( &frame, 0, sizeof(ws_frame_t) );

        coWait(fd->__read( bf, 2   )==-2); read_ws_hdr_frame( bf, len );
        coWait(fd->__read( bf, len )==-2); read_ws_hdr_lensk( bf, len );

        if( frame.OPC ==  8 ){ data=0; fd->write( end_frame () ); coEnd;     }
        if( frame.OPC >= 20 ){ data=0; fd->write( end_frame () ); coEnd;     }
        if( frame.OPC ==  9 ){ data=0; fd->write( pong_frame() ); coGoto(0); }

        if( frame.OPC >= 11 || frame.OPC == 10 ||
          ( frame.OPC >=  3 && frame.OPC <= 7  )
        ) { data=0; coGoto(0); }
        
        if( frame.LEN ==  0 ){ data=0; coGoto(0); }
        
        coYield(1); len=0;

        while ( frame.LEN > 0 ){ sz = min ( (len_t)sx, frame.LEN );
        coWait( fd->_read_( bf, sz, &len )==-2 );

        if( frame.MSK ){ for( ulong x=0; x<len; ++x ){
            bf[x]=bf[x]^frame.KEY[key]; key++; key%=4;
        }}

            frame.LEN -= len; data = len;
        if( frame.LEN ==0 ){ coEnd; } coStay(1); }

    coGoto(0) ; coFinish
    }};

    GENERATOR( write ){
    protected:
            ptr_t<char> bfx;
            string_t    bff; char* mask;
            ulong    size=0;
    public: ulong    data=0;

    protected:

        string_t write_ws_frame( char* bf, ulong sx, uchar opcode, char* mask ) {
            auto byt = encoder::bytes::get( sx ); uint idx = 0;

            if( opcode == 0 ){ bool b=0; for ( ulong x=0; x<sx; x++ ){
            if( !string::is_print( bf[x] ) ){ b=1; break; }}
                     bfx[idx] = !b? 0x82:0x81;
            } else { bfx[idx] = 0x80 | opcode; } ++idx; 
            
            bfx[idx] = mask==nullptr ? 0x00 : 0x80;

            if ( sx < 126 ){
                bfx[idx]|= (uchar)(byt[byt.size()-1]); ++idx;
            } elif ( sx < 65536 ){
                bfx[idx]|= (uchar)( 126 ); /*-------*/ ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-2]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-1]); ++idx;
            } else {
                bfx[idx]|= (uchar)( 127 ); /*-------*/ ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-8]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-7]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-6]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-5]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-4]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-3]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-2]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-1]); ++idx;
            } if ( mask != nullptr ) {
                bfx[idx] = (uchar)(mask[0]); /*-----*/ ++idx;
                bfx[idx] = (uchar)(mask[1]); /*-----*/ ++idx;
                bfx[idx] = (uchar)(mask[2]); /*-----*/ ++idx;
                bfx[idx] = (uchar)(mask[3]); /*-----*/ ++idx;
            }

            return string_t( &bfx, idx );
        }

    public: write() noexcept : bfx( 16UL ) {}

        template<class T> coEmit( T* fd, char* bf, const ulong& sx ) {
        coBegin

            mask=fd->get_mask(); bff=write_ws_frame( bf, sx, 0, mask )
                +string_t( bf, sx ); data=0; size=0;
            
            if ( mask!=nullptr ){  ulong sy=0; 
            for( char *y = bff.end()-sx; y<bff.end(); y++ )
               { *y ^= mask[ sy++%4 ];
            }  }

            bff=write_ws_frame( bf, sx, 0, nullptr )+string_t( bf, sx ); data=0; size=0;
            coWait( fd->_write_( bff.get(), bff.size(), &size) == -2 ); data=sx;

        coFinish }

    };

}}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/