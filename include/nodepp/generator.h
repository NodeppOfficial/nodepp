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
namespace nodepp { namespace _timer_ {

    GENERATOR( timer ){ public:

        template< class V, class... T >
        coEmit( V func, ulong time, const T&... args ){
        gnStart
            coDelay( time ); if( func(args...)<0 )
                   { coEnd; } coGoto(0);
        gnStop
        }

        template< class V, class... T >
        coEmit( V func, ulong* time, const T&... args ){
        gnStart
            coDelay( *time ); if( func(args...)<0 )
                   { coEnd; } coGoto(0);
        gnStop
        }

    };

    /*─······································································─*/

    GENERATOR( utimer ){ public:

        template< class V, class... T >
        coEmit( V func, ulong time, const T&... args ){
        gnStart
            coUDelay( time ); if( func(args...)<0 )
                    { coEnd; } coGoto(0);
        gnStop
        }

        template< class V, class... T >
        coEmit( V func, ulong* time, const T&... args ){
        gnStart
            coUDelay( *time ); if( func(args...)<0 )
                    { coEnd; } coGoto(0);
        gnStop
        }

    };

}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_PROMISE) && defined(NODE_PROMISE) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_PROMISE
namespace nodepp { namespace _promise_ {

    GENERATOR( resolve ){ public:

        template< class T, class U, class V >
        coEmit( ptr_t<bool> state, const T& func, const U& res, const V& rej ){
        gnStart
            func( res, rej ); coWait( *state==1 );
        gnStop
        }

        template< class T, class U >
        coEmit( ptr_t<bool> state, const T& func, const U& res ){
        gnStart
            func( res ); coWait( *state==1 );
        gnStop
        }

    };

}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_FILE) && defined(NODEPP_FILE) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_FILE
namespace nodepp { namespace _file_ {

    GENERATOR( read ){
    private:
        ulong    d;
        ulong*   r;

    public:
        string_t data ;
        int      state;

    template< class T > coEmit( T* str, ulong size=CHUNK_SIZE ){
    gnStart state=0; d=0; data.clear(); str->flush();

    coWait( str->is_used()==1 ); r = str->get_range();

        if(!str->is_available()       ){ coEnd; }
        if(!str->get_borrow().empty() ){ data = str->get_borrow(); }

          if ( r[1] != 0  ){ auto pos = str->pos(); d = r[1]-r[0];
          if ( pos < r[0] ){ str->del_borrow(); str->pos( r[0] ); }
        elif ( pos >=r[1] ){ coEnd; } }
        else { d = str->get_buffer_size(); }

        if( data.empty() )
          { coWait((state=str->_read(str->get_buffer_data(),min(d,size)))==-2); }

        if( state > 0 ){
            data  = string_t( str->get_buffer_data(), (ulong) state );
        }   state = min( data.size(), size ); str->del_borrow();

        str->set_borrow( data.splice( size, data.size() ) );

    gnStop
    }};

    /*─······································································─*/

    GENERATOR( write ){
    private:
        string_t b ;

    public:
        ulong    data ;
        int      state;

    template< class T > coEmit( T* str, const string_t& msg ){
    gnStart state=0; data=0; str->flush();

        coWait( str->is_used()==1 ); str->use();

        if(!str->is_available() || msg.empty() ){ str->release(); coEnd; }
        if( b.empty() )                         { b = msg;               }

        do{ coWait((state=str->_write( b.data()+data, b.size()-data ))==-2 );
        if( state>0 ){ data += state; }} while ( state>=0 && data<b.size() );

        b.clear(); str->release();

    gnStop
    }};

    /*─······································································─*/

    GENERATOR( until ){
    private:

        _file_::read _read; uint pos=0;

    public:
        string_t  data ;
        ulong     state;

    template< class T > coEmit( T* str, string_t ch ){
    gnStart data.clear(); str->flush(); state=0; pos=0;

        coWait( _read(str) ==1 );
            if( _read.state<=0 ){ coEnd; }

        do{for( auto x: _read.data ){ state++;
            if( ch[pos]  ==x   ){ pos++; } else { pos=0; }
            if( ch.size()==pos ){ break; } }
        } while(0);

        str->set_borrow( _read.data );

        if( memcmp( _read.data.get(), ch.get(), ch.size() )==0 ){
                 data=str->get_borrow().splice( 0, ch.size() );
        } elif( state > pos ) {
                 data=str->get_borrow().splice( 0, state-pos );
        } else { data=str->get_borrow().splice( 0, state     ); }

        state = data.size();

    gnStop
    }

    template< class T > coEmit( T* str, char ch ){
    gnStart data.clear(); str->flush(); state=0;

        coWait( _read(str) ==1 );
            if( _read.state<=0 ){ coEnd; }

        do{ for( auto x: _read.data ){ state++;
             if( ch ==x ){ break; } continue; }
        } while(0);

               str->set_borrow(_read.data);
        data = str->get_borrow().splice( 0, state );

    gnStop
    }};

    /*─······································································─*/

    GENERATOR( line ){
    private:

        _file_::read _read;

    public:
        string_t  data ;
        ulong     state;

    template< class T > coEmit( T* str ){
    gnStart data.clear(); str->flush(); state=0;

        coWait( _read(str) ==1 );
            if( _read.state<=0 ){ coEnd; }

        do{ for( auto x: _read.data ){ state++;
             if('\n'==x ){ break; } continue; }
        } while(0);

               str->set_borrow(_read.data);
        data = str->get_borrow().splice( 0, state );

    gnStop
    }};

}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_STREAM) && defined(NODEPP_STREAM) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_STREAM
namespace nodepp { namespace _stream_ {

    GENERATOR( duplex ){
    private:

        _file_::write _write1, _write2;
        _file_::read  _read1 , _read2;

    public:

        template< class T, class V > coEmit( const T& inp, const V& out ){
        gnStart inp.onPipe.emit(); out.onPipe.emit(); coYield(1);

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

        gnStop
        }

    };

    /*─······································································─*/

    GENERATOR( pipe ){
    private:

        _file_::write _write;
        _file_::read  _read;

    public:

        template< class T > coEmit( const T& inp ){
        gnStart inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break;  }
                    inp.onData.emit(_read.data);
            }       inp.close();
        gnStop
        }

        template< class T, class V > coEmit( const T& inp, const V& out ){
        gnStart inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break;  }
           coWait( _write(&out,_read.data)==1 );
               if( _write.state<=0 ){ break;  }
                    inp.onData.emit(_read.data);
            }       inp.close(); out.close();
        gnStop
        }

    };

    /*─······································································─*/

    GENERATOR( until ){
    private:

        _file_::write _write;
        _file_::until  _read;

    public:

        template< class T, class U >
        coEmit( const T& inp, const U& val ){
        gnStart inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp,val)==1 );
               if( _read.state <=0 ){ break; }
                   inp.onData.emit(_read.data);
            }      inp.close();
        gnStop
        }

        template< class T, class V, class U >
        coEmit( const T& inp, const V& out, const U& val ){
        gnStart inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp,val)==1 );
               if( _read.state  <=0 ){ break; }
           coWait( _write(&out,_read.data)==1 );
               if( _write.state <=0 ){ break; }
                    inp.onData.emit(_read.data);
            }       inp.close(); out.close();
        gnStop
        }

    };

    /*─······································································─*/

    GENERATOR( line ){
    private:

        _file_::write _write;
        _file_::line  _read;

    public:

        template< class T > coEmit( const T& inp ){
        gnStart inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp)==1 );
               if( _read.state<=0 ){ break;  }
                   inp.onData.emit(_read.data);
            }      inp.close();
        gnStop
        }

        template< class T, class V > coEmit( const T& inp, const V& out ){
        gnStart inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break;  }
           coWait( _write(&out,_read.data)==1 );
               if( _write.state<=0 ){ break;  }
                    inp.onData.emit(_read.data);
            }       inp.close(); out.close();
        gnStop
        }

    };

}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_POLL) && defined(NODEPP_SOCKET) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_POLL
namespace nodepp { namespace _poll_ {

    GENERATOR( poll ){ public:

        template< class V, class T, class U >
        coEmit( V ctx, T str, U cb ){
        gnStart
            str->onSocket.emit( ctx ); cb(ctx);
        gnStop
        }

    };

}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_ZLIB) && defined(NODEPP_ZLIB) && defined(NODEPP_GENERATOR)
    #define  GENERATOR_ZLIB
namespace nodepp { namespace _zlib_ {

    GENERATOR( pipe_inflate ){
    private:

        _file_::write _write;
        _file_::read  _read;
        string_t borrow;

    public:

        template< class Z, class T, class V > coEmit( const Z& zlb, const T& inp, const V& out ){
        gnStart inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break; }
           borrow = zlb.update_inflate(_read.data);
           coWait( _write( &out, borrow )==1 );
               if( _write.state<=0 ){ break; }
                    inp.onData.emit( borrow );
            }       inp.close(); out.close();
        gnStop
        }

        template< class Z, class T > coEmit( const Z& zlb, const T& inp ){
        gnStart inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp)==1 );
               if( _read.state<=0 ){ break; }
            borrow = zlb.update_inflate(_read.data);
                    inp.onData.emit( borrow );
            }       inp.close();
        gnStop
        }

    };

    GENERATOR( pipe_deflate ){
    private:

        _file_::write _write;
        _file_::read  _read;
        string_t borrow;

    public:

        template< class Z, class T, class V > coEmit( const Z& zlb, const T& inp, const V& out ){
        gnStart inp.onPipe.emit(); out.onPipe.emit();
            while( inp.is_available() && out.is_available() ){
           coWait( _read(&inp) ==1 );
               if( _read.state <=0 ){ break; }
           borrow = zlb.update_deflate(_read.data);
           coWait( _write( &out, borrow )==1 );
               if( _write.state<=0 ){ break; }
                    inp.onData.emit( borrow );
            }       inp.close(); out.close();
        gnStop
        }

        template< class Z, class T > coEmit( const Z& zlb, const T& inp ){
        gnStart inp.onPipe.emit();
            while( inp.is_available() ){
           coWait( _read(&inp)==1 );
               if( _read.state<=0 ){ break; }
            borrow = zlb.update_deflate(_read.data);
                    inp.onData.emit( borrow );
            }       inp.close();
        gnStop
        }

    };

}}
#undef NODEPP_GENERATOR
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_WS) && defined(NODEPP_GENERATOR) && ( defined(NODEPP_WS) || defined(NODEPP_WSS) )
    #define  GENERATOR_WS
    #include "encoder.h"
    #include "crypto.h"
namespace nodepp { namespace _ws_ {

    struct ws_frame_t {
        bool  FIN;     //1b
        uint  RSV;     //3b
        uint  OPC;     //4b
        bool  MSK;     //1b
        char  KEY [4]; //4B
        ulong LEN;     //64b
    };

    /*─······································································─*/

    template< class T > bool server( T& cli ) {
        auto data = cli.read(); cli.set_borrow( data );

        if(  cli.read_header()!=0 ){ return 0; }
        if( !cli.headers["Sec-Websocket-Key"].empty() ){

            string_t sec = cli.headers["Sec-Websocket-Key"];
                auto sha = crypto::hash::SHA1(); sha.update( sec + SECRET );
            string_t enc = encoder::base64::get( encoder::buffer::hex2buff(sha.get()) );

            cli.write_header( 101, header_t({
                { "Sec-Websocket-Accept", enc },
                { "Connection", "upgrade" },
                { "Upgrade", "websocket" }
            }) );

            cli.stop();             return 1;
        }   cli.set_borrow( data ); return 0;
    }

    /*─······································································─*/

    template< class T > bool client( T& cli, string_t url ) {
        string_t hsh = encoder::key::generate("abcdefghiABCDEFGHI0123456789",22);
        string_t key = string::format("%s==",hsh.data());

        header_t header ({
            { "Upgrade", "websocket" },
            { "Connection", "upgrade" },
            { "Sec-Websocket-Key", key },
            { "Sec-Websocket-Version", "13" }
        });

        cli.write_header( "GET", url::path(url), "HTTP/1.0", header );

        if( cli.read_header()!=0 ){
            _EERROR(cli.onError,"Could not connect to server");
            cli.close(); return false;
        }

        if( cli.status != 101 ){
            _EERROR(cli.onError,string::format("Can't connect to WS Server -> status %d",cli.status));
            cli.close(); return false;
        }

        if(!cli.headers["Sec-Websocket-Accept"].empty() ){

            string_t dta = cli.headers["Sec-Websocket-Accept"];
                auto sha = crypto::hash::SHA1(); sha.update( key + SECRET );
            string_t enc = encoder::base64::get( encoder::buffer::hex2buff(sha.get()) );

            if( dta != enc ){
                _ERROR("secret key does not match");
                cli.close(); return false;
            }   cli.stop ();

        }

        return true;
    }

    /*─······································································─*/

    GENERATOR( read ){
    private:

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
            if ( frame.LEN == 127 ){ size =4; }}
            if ( frame.MSK == 1   ){ size+=4; }

        }

        void read_ws_hdr_lensk( char* bf, ulong& size ){ size=0;

            if ( frame.LEN  > 125 ){
            if ( frame.LEN == 126 ){ size=2; }
            if ( frame.LEN == 127 ){ size=4; } frame.LEN=0;
            for( ulong x=0; x < size; x++ )  { frame.LEN=frame.LEN << 8 | (uchar) bf[x]; }
            }

            if ( frame.MSK == 1 ){ size=4; 
            for( ulong x=0; x<size; x++ ){ frame.KEY[x] = bf[x]; }
            }

        }

    public:

    template<class T> coEmit( T* str, char* bf, const ulong& sx ) {
    gnStart ; memset( bf, 0, sx ); size=0; data=0; len=0;
              memset( &frame, 0, sizeof(ws_frame_t) );

        coWait(str->__read( bf, 2   )==-2); read_ws_hdr_frame( bf, len );
        coWait(str->__read( bf, len )==-2); read_ws_hdr_lensk( bf, len );

        if( frame.LEN ==  0 ){ data=0; coGoto(0); }
        if( frame.OPC ==  8 ){ data=0; coEnd;     }
        if( frame.OPC >= 20 ){ data=0; coEnd;     }

        coYield(1); len=0;

        while ( frame.LEN > 0 ){ sz = min( sx, frame.LEN );
        coWait( str->_read_( bf, sz, len )==1 );

        do{ for( auto x=len; x-->0; ){
            bf[x]=bf[x]^frame.KEY[key]; key=(key+1) % 4;
        } } while(0);

            frame.LEN -= len; data = len;
        if( frame.LEN ==0 ){ coEnd; } coStay(1); }

    coGoto(0) ; gnStop
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

            idx++;

            if ( sx < 126 ){
                bfx[idx] = (uchar)(byt[byt.size()-1]); idx++;
            } elif ( sx < 65536 ){
                bfx[idx] = (uchar)( 126 ); idx++;
                bfx[idx] = (uchar)(byt[byt.size()-2]); idx++;
                bfx[idx] = (uchar)(byt[byt.size()-1]); idx++;
            } else {
                bfx[idx] = (uchar)( 127 ); idx++;
                bfx[idx] = (uchar)(byt[byt.size()-4]); idx++;
                bfx[idx] = (uchar)(byt[byt.size()-3]); idx++;
                bfx[idx] = (uchar)(byt[byt.size()-2]); idx++;
                bfx[idx] = (uchar)(byt[byt.size()-1]); idx++;
            }

            return string_t( &bfx, idx );
        }

    public:

        template<class T> coEmit( T* str, char* bf, const ulong& sx ) {
        gnStart

            bff  = write_ws_frame( bf, sx ) + string_t( bf, sx ); data=0;size=0;
            coWait(str->_write_( bff.get(),bff.size(),size )==1); data = sx;

        gnStop
        }

    };

}}
#endif
