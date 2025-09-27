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

#if !defined(GENERATOR_TIMER) && defined(NODEPP_TIMER) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_TIMER
namespace nodepp { namespace generator { namespace timer {

    GENERATOR( timer ){ public:

        template< class V, class... T >
        coEmit( V func, ulong time, const T&... args ){
        coBegin
            coDelay( time ); if( func(args...)<0 )
                   { coEnd; } coGoto(0);
        coFinish
        }

        template< class V, class... T >
        coEmit( V func, ulong* time, const T&... args ){
        coBegin
            coDelay( *time ); if( func(args...)<0 )
                   { coEnd; } coGoto(0);
        coFinish
        }

    };

    /*─······································································─*/

    GENERATOR( utimer ){ public:

        template< class V, class... T >
        coEmit( V func, ulong time, const T&... args ){
        coBegin
            coUDelay( time ); if( func(args...)<0 )
                    { coEnd; } coGoto(0);
        coFinish
        }

        template< class V, class... T >
        coEmit( V func, ulong* time, const T&... args ){
        coBegin
            coUDelay( *time ); if( func(args...)<0 )
                    { coEnd; } coGoto(0);
        coFinish
        }

    };

}}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_FILE) && defined(NODEPP_FILE) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_FILE
namespace nodepp { namespace generator { namespace file {

    GENERATOR( read ){
    protected: ulong d; ulong* r;
    public:    string_t data; int state;

    template< class T > coEmit( T* str, ulong size=CHUNK_SIZE ){
    coBegin; data.clear(); state=0; d=0;

        if( !str->is_available()       ){ coEnd; } r=str->get_range();
        if( !str->get_borrow().empty() ){ data = str->get_borrow(); }

        if( r[1] != 0  ){ auto pos = str->pos(); d = r[1]-r[0];
        if( pos < r[0] ){ str->del_borrow(); str->pos(r[0]); }
      elif( pos >=r[1] ){ coEnd; } } else { d = str->get_buffer_size(); }

        if( data.empty() ){ 
            coWait((state=str->_read(str->get_buffer_data(),min(d,size)))==-2);
        if( state<=0 ){ coEnd; }
        if( state >0 ){ data=string_t(str->get_buffer_data(),(ulong)state); }}

        state = min( data.size(), size ); /*---------------*/
        str->set_borrow( data.splice( state, data.size() ) );

    coFinish
    }};

    /*─······································································─*/

    GENERATOR( write ){
    public: ulong data; int state;

    template< class T > coEmit( T* str, const string_t& msg ){
    coBegin state=0; data=0;

        if(!str->is_available() || msg.empty() ){ coEnd; }

        do{ coWait((state=str->_write( msg.data()+data, msg.size()-data ))==-2 );
        if( state<=0 ){ coEnd; }
        if( state >0 ){ data += state; }} while ( state>=0 && data<msg.size() );

    coFinish
    }};

    /*─······································································─*/

    GENERATOR( until ){
    protected: ulong pos; file::read _read;
    public: int state; string_t data;

    template< class T > coEmit( T* str, string_t ch ){
    coBegin; state=0; pos=0; data.clear();

        coWait( _read(str) ==1 );
            if( _read.state<=0 ){ state=-1; coEnd; }
        str->set_borrow( _read.data );

        do{for( auto x: _read.data ){ ++state;
            if( ch[pos]  ==x   ){ ++pos; } else { pos=0; }
            if( ch.size()==pos ){ break; } }
        } while(0);

        if( memcmp( _read.data.get(), ch.get(), ch.size() )==0 ){
                 data=str->get_borrow().splice( 0, ch.size() );
        } elif( (ulong) state > pos ) {
                 data=str->get_borrow().splice( 0, state-pos );
        } else { data=str->get_borrow().splice( 0, state     ); }

        state = data.size();

    coFinish
    }

    template< class T > coEmit( T* str, char ch ){
    coBegin; data.clear(); state=0;

        coWait( _read(str) ==1 );
            if( _read.state<=0 ){ coEnd; }
        str->set_borrow( _read.data );

        do{ for( auto x: _read.data ){ ++state;
             if( ch ==x ){ break; } continue; }
        } while(0);

        data = str->get_borrow().splice( 0, state );

    coFinish
    }};

    /*─······································································─*/

    GENERATOR( line ){
    protected: file::read _read;
    public: string_t data; int state;

    template< class T > coEmit( T* str ){
    coBegin data.clear(); state=0;

        coWait( _read(str) ==1 );
            if( _read.state<=0 ){ coEnd; }
        str->set_borrow(_read.data);

        do{ for( auto x: _read.data ){ ++state;
             if('\n'==x ){ break; } continue; }
        } while(0);

        data = str->get_borrow().splice( 0, state );

    coFinish
    }};

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
        coBegin inp.onPipe.emit(); out.onPipe.emit(); coYield(1);

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

        coFinish
        }

    };

    /*─······································································─*/

    GENERATOR( pipe ){
    protected:

        file::write _write;
        file::read  _read;

    public:

        template< class T > coEmit( const T& inp ){
        coBegin inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break;  }
                    inp.onData.emit(_read.data);
            }       inp.close();
        coFinish
        }

        template< class T, class V > coEmit( const T& inp, const V& out ){
        coBegin inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break;  }
           coWait( _write(&out,_read.data)==1 );
               if( _write.state<=0 ){ break;  }
                    inp.onData.emit(_read.data);
            }       inp.close(); out.close();
        coFinish
        }

    };

    /*─······································································─*/

    GENERATOR( until ){
    protected:

        file::write _write;
        file::until  _read;

    public:

        template< class T, class U >
        coEmit( const T& inp, const U& val ){
        coBegin inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp,val)==1 );
               if( _read.state <=0 ){ break; }
                   inp.onData.emit(_read.data);
            }      inp.close();
        coFinish
        }

        template< class T, class V, class U >
        coEmit( const T& inp, const V& out, const U& val ){
        coBegin inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp,val)==1 );
               if( _read.state  <=0 ){ break; }
           coWait( _write(&out,_read.data)==1 );
               if( _write.state <=0 ){ break; }
                    inp.onData.emit(_read.data);
            }       inp.close(); out.close();
        coFinish
        }

    };

    /*─······································································─*/

    GENERATOR( line ){
    protected:

        file::write _write;
        file::line  _read;

    public:

        template< class T > coEmit( const T& inp ){
        coBegin inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp)==1 );
               if( _read.state<=0 ){ break;  }
                   inp.onData.emit(_read.data);
            }      inp.close();
        coFinish
        }

        template< class T, class V > coEmit( const T& inp, const V& out ){
        coBegin inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break;  }
           coWait( _write(&out,_read.data)==1 );
               if( _write.state<=0 ){ break;  }
                    inp.onData.emit(_read.data);
            }       inp.close(); out.close();
        coFinish
        }

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
        coBegin inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break; }
           borrow = zlb.update_inflate(_read.data);
           coWait( _write( &out, borrow )==1 );
               if( _write.state<=0 ){ break; }
                    inp.onData.emit( borrow );
            }       inp.close(); out.close();
        coFinish
        }

        template< class Z, class T > coEmit( const Z& zlb, const T& inp ){
        coBegin inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp)==1 );
               if( _read.state<=0 ){ break; }
            borrow = zlb.update_inflate(_read.data);
                    inp.onData.emit( borrow );
            }       inp.close();
        coFinish
        }

    };

    GENERATOR( pipe_deflate ){
    protected:

        file::write _write;
        file::read  _read;
        string_t borrow;

    public:

        template< class Z, class T, class V > coEmit( const Z& zlb, const T& inp, const V& out ){
        coBegin inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break; }
           borrow = zlb.update_deflate(_read.data);
           coWait( _write( &out, borrow )==1 );
               if( _write.state<=0 ){ break; }
                    inp.onData.emit( borrow );
            }       inp.close(); out.close();
        coFinish
        }

        template< class Z, class T > coEmit( const Z& zlb, const T& inp ){
        coBegin inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp)==1 );
               if( _read.state<=0 ){ break; }
            borrow = zlb.update_deflate(_read.data);
                    inp.onData.emit( borrow );
            }       inp.close();
        coFinish
        }

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
        bool  FIN;     //1b
        uint  RSV;     //3b
        uint  OPC;     //4b
        bool  MSK;     //1b
        char  KEY [4]; //4B
        ulong LEN;     //64b
    };

    /*─······································································─*/

    template< class T > bool server( T& cli ) { do {
        auto data = cli.read(); cli.set_borrow( data );

        int c=0; while( (c=cli.read_header())==1 ){}
        if( c != 0 ){ break; }

        if( cli.headers.has("Sec-Websocket-Key") ){

            string_t sec = cli.headers["Sec-Websocket-Key"];
                auto sha = crypto::hash::SHA1(); sha.update( sec + NODEPP_WS_SECRET );
            string_t enc = encoder::base64::get( encoder::buffer::hex2buff(sha.get()) );

            cli.write_header( 101, header_t({
                { "Sec-Websocket-Accept", enc },
                { "Connection", "upgrade" },
                { "Upgrade", "websocket" }
            }) );

            cli.stop(); return true;
        }   cli.set_borrow( data );

    } while(0); return false; }

    /*─······································································─*/

    template< class T > bool client( T& cli, string_t url ) { do {
        string_t hsh = encoder::key::generate("abcdefghiABCDEFGHI0123456789",22);
        string_t key = string::format("%s==",hsh.data());

        header_t header ({
            { "Upgrade", "websocket" },
            { "Connection", "upgrade" },
            { "Sec-Websocket-Key", key },
            { "Sec-Websocket-Version", "13" }
        });

        cli.write_header( "GET", url::path(url), "HTTP/1.1", header );
        int c=0; while( (c=cli.read_header())==1 ){}

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

        void read_ws_hdr_frame( char* bf, ulong& size ){ size=0;

            do { array_t<bool> y;

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

        void read_ws_hdr_lensk( char* bf, ulong& size ){

            if ( frame.MSK == 1 ){ size -= 4;
            for( ulong x=0; x<4; ++x ){ frame.KEY[x] = bf[x+size]; }}

            if ( frame.LEN  > 125 ){ /*---*/ frame.LEN=0;
            for( ulong x=0; x < size; ++x ){ frame.LEN=frame.LEN << 8 | (uchar) bf[x]; }}

        }

    public:

    template<class T> coEmit( T* str, char* bf, const ulong& sx ) { int c=0;
    coBegin ; memset( bf, 0, sx ); size=0; data=0; len=0; key=0;
              memset( &frame, 0, sizeof(ws_frame_t) );

        coWait(str->__read( bf, 2   )==-2); read_ws_hdr_frame( bf, len );
        coWait(str->__read( bf, len )==-2); read_ws_hdr_lensk( bf, len );

        if( frame.LEN ==  0 ){ data=0; coGoto(0); }
        if( frame.OPC ==  8 ){ data=0; coEnd;     }
        if( frame.OPC >= 20 ){ data=0; coEnd;     }

        coYield(1); len=0;

        while ( frame.LEN > 0 ){ sz = min( sx, frame.LEN );
        coWait( str->_read_( bf, sz, len )==1 );

        if( frame.MSK ){ for( ulong x=0; x<len; ++x ){
            bf[x]=bf[x]^frame.KEY[key]; key++; key%=4;
        }}

            frame.LEN -= len; data = len;
        if( frame.LEN ==0 ){ coEnd; } coStay(1); }

    coGoto(0) ; coFinish
    }};

    GENERATOR( write ){
    protected:
            string_t bff;
            ulong size=0;
    public: ulong data=0;

    protected:

        string_t write_ws_frame( char* bf, const ulong& sx ){
            auto bfx = ptr_t<char>( 16, '\0' ); uint idx = 0;
            auto byt = encoder::bytes::get( sx );

            auto x=sx; bool b=0; while( x-->0 ){
                if( !string::is_print(bf[x]) ){ b=1; break; }
            }   bfx[idx] = b ? (char) 0b10000010 : (char) 0b10000001;

            ++idx; if ( sx < 126 ){
                bfx[idx] = (uchar)(byt[byt.size()-1]); ++idx;
            } elif ( sx < 65536 ){
                bfx[idx] = (uchar)( 126 ); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-2]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-1]); ++idx;
            } else {
                bfx[idx] = (uchar)( 127 ); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-8]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-7]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-6]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-5]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-4]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-3]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-2]); ++idx;
                bfx[idx] = (uchar)(byt[byt.size()-1]); ++idx;
            }

            return string_t( &bfx, idx );
        }

    public:

        template<class T> coEmit( T* str, char* bf, const ulong& sx ) {
        coBegin

            bff  = write_ws_frame( bf, sx ) + string_t( bf, sx ); data=0;size=0;
            coWait(str->_write_( bff.get(),bff.size(),size )==1); data = sx;

        coFinish
        }

    };

}}}
#undef NODEPP_GENERATOR
#endif
