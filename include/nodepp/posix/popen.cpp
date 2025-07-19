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
#include <unistd.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class popen_t : public generator_t {
private:

    using _read_ = generator::file::read;

protected:

    ptr_t<_read_> _read1 = new _read_();
    ptr_t<_read_> _read2 = new _read_();

    struct NODE {
        int           fd;
        int     state =0;
        file_t  std_input;
        file_t  std_error;
        file_t  std_output;
    };  ptr_t<NODE> obj;

    template< class T >
    void _init_( const string_t& path, T& arg, T& env ) {

        int fda[2]; ::pipe( fda );
        int fdb[2]; ::pipe( fdb );
        int fdc[2]; ::pipe( fdc ); obj->fd = ::fork();

        if( obj->fd == 0 ){
            ::dup2( fda[0], STDIN_FILENO  ); ::close( fda[1] );
            ::dup2( fdb[1], STDOUT_FILENO ); ::close( fdb[0] ); arg.push( nullptr );
            ::dup2( fdc[1], STDERR_FILENO ); ::close( fdc[0] ); env.push( nullptr );
            ::execvpe( path.c_str(), (char**)arg.data(),(char**)env.data() );
            throw except_t("while spawning new popen"); process::exit(1);
        } elif ( obj->fd > 0 ){
            obj->std_input  = file_t( fda[1] ); ::close( fda[0] );
            obj->std_output = file_t( fdb[0] ); ::close( fdb[1] );
            obj->std_error  = file_t( fdc[0] ); ::close( fdc[1] );
            obj->state      = 1;
        } else {
            ::close( fda[0] ); ::close( fda[1] );
            ::close( fdb[0] ); ::close( fdb[1] );
            ::close( fdc[0] ); ::close( fdc[1] );
            obj->state      = 0;
        }

    }

public:

    event_t<>          onResume;
    event_t<except_t>  onError;
    event_t<>          onClose;
    event_t<>          onStop;
    event_t<>          onDrain;
    event_t<>          onOpen;

    event_t<string_t>  onData;
    event_t<string_t>  onDout;
    event_t<string_t>  onDerr;

    popen_t( const string_t& path, const initializer_t<string_t>& args, const initializer_t<string_t>& envs )
    : obj( new NODE() ) { if( path.empty() ){ throw except_t("invalid command"); }
        array_t<const char*> arg; array_t<const char*> env;
        for( auto x : args ) { arg.push( x.get() ); }
        for( auto x : envs ) { env.push( x.get() ); } _init_( path, arg, env );
    }

    popen_t( const string_t& path ) 
    : obj( new NODE() ) { if( path.empty() ){ throw except_t("invalid command"); }
        array_t<const char*> arg; array_t<const char*> env; auto cmd = regex::match_all( path, "[^ ]+" );
        for( auto x: cmd ){ arg.push( x.get() ); } _init_( cmd[0], arg, env );
    }

    popen_t( const string_t& path, const initializer_t<string_t>& args ) : obj( new NODE() ) { 
        if ( path.empty() ){ throw except_t("invalid command"); }
        array_t<const char*> arg; array_t<const char*> env;
        for( auto x : args ) { arg.push( x.get() ); }
        _init_( path, arg, env );
    }

   ~popen_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    popen_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void free() const noexcept {

        if( obj->state == -3 && obj.count() > 1 ){ resume(); return; }
        if( obj->state == -2 ){ return; } close(); obj->state = -2;
            obj->std_error.close(); obj->std_output.close();
            obj->std_input.close(); kill();

        onResume.clear(); onError.clear(); 
        onStop  .clear(); onOpen .clear();
        onData  .clear(); onDout .clear(); 
        onDerr  .clear(); onClose.emit (); 
        
    }

    /*─······································································─*/

    int next() noexcept {
    coBegin; if( !is_closed() ){
        
        onOpen.emit(); coYield(1);

        if((*_read1)(&std_output())==1){ coGoto(2); }
        if(  _read1->state <= 0 )      { coGoto(2); }
        onData.emit(_read1->data);
        onDout.emit(_read1->data);

        coYield(2); if( !is_alive() )  { break; }

        if((*_read2)(&std_error())==1 ){ coGoto(1); }
        if(  _read2->state <= 0 )      { coGoto(1); }
        onData.emit(_read2->data);
        onDerr.emit(_read2->data);
        
    coGoto(1); } coFinish
    }

    /*─······································································─*/

    bool is_alive() const noexcept { 
        if( std_error ().is_available() ){ return true; }
        if( std_output().is_available() ){ return true; } return false; 
    }

    bool is_available() const noexcept { return is_closed()== false; }
    bool is_closed()    const noexcept { return obj->state <= 0; }
    int  get_fd()       const noexcept { return obj->fd; }

    /*─······································································─*/

    void  flush() const noexcept { std_input().flush(); std_output().flush(); std_error().flush(); }
    void resume() const noexcept { if(obj->state== 0) { return; } obj->state= 0; onResume.emit(); }
    void  close() const noexcept { if(obj->state < 0) { return; } obj->state=-1; onDrain.emit(); }
    void   stop() const noexcept { if(obj->state==-3) { return; } obj->state=-3; onStop.emit(); }
    void   kill() const noexcept { ::kill( obj->fd, SIGKILL ); }

    /*─······································································─*/

    template< class... T >
    int write( const T&... args )     const noexcept { return std_input().write( args... ); }

    template< class... T >
    string_t read( const T&... args ) const noexcept { return std_output().read( args... ); }

    /*─······································································─*/

    template< class... T >
    int _write( const T&... args ) const noexcept { return std_input()._write( args... ); }

    template< class... T >
    int _read( const T&... args )  const noexcept { return std_output()._read( args... ); }

    /*─······································································─*/

    file_t& std_error()  const noexcept { return obj->std_error;  }
    file_t& std_output() const noexcept { return obj->std_output; }
    file_t& std_input()  const noexcept { return obj->std_input;  }

};}

/*────────────────────────────────────────────────────────────────────────────*/
