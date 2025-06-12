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

namespace nodepp { class cluster_t {
protected:

    ptr_t<_file_::read> _read1 = new _file_::read;
    ptr_t<_file_::read> _read2 = new _file_::read;

    struct NODE {
        int          fd;
        int    state =1;
        file_t    input;
        file_t   output;
        file_t    error;
    };  ptr_t<NODE> obj;

    template< class T >
    void _init_( T& arg, T& env ) {

        if( process::is_child() ){
            int fd[2] = { 0, 0 }; string_t ch = process::env::get("CHILD");
            string::parse( ch.data(), "%d|%d", &fd[0], &fd[1] );
            obj->error  = { STDERR_FILENO };
            obj->output = { fd[0] };
            obj->input  = { fd[1] }; return;
        }

        int fda[2]; ::pipe( fda );
        int fdb[2]; ::pipe( fdb );
        int fdc[2]; ::pipe( fdc ); obj->fd = ::fork();

        if( obj->fd == 0 ){
            auto chl = string::format( "CHILD=%d|%d", fda[0], fdb[1] ); env.push( chl.c_str() );
            arg.unshift( process::args[0].c_str() ); ::close( fda[1] ); arg.push( NULL );
                                                     ::close( fdb[0] ); env.push( NULL );
            ::dup2( fdc[1], STDERR_FILENO  );        ::close( fdc[0] );
            ::execvpe( arg[0], (char**) arg.data(), (char**)env.data() );
            process::error("while spawning new cluster"); process::exit(1);
        } elif ( obj->fd > 0 ) { // Parent process
            obj->input  = { fda[1] }; ::close( fda[0] );
            obj->output = { fdb[0] }; ::close( fdb[1] );
            obj->error  = { fdc[0] }; ::close( fdc[1] );
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

    virtual ~cluster_t() noexcept {
        if( obj.count() > 1 ){ return; }
        if( obj->state == 0 ){ return; } free();
    }

    cluster_t( const initializer_t<string_t>& args ) : obj( new NODE() ) {
        array_t<const char*> arg; array_t<const char*> env; bool y=0;

        for ( auto x : args ) {
          if( x != nullptr && !y ) arg.push( x.c_str() );
        elif( x != nullptr &&  y ) env.push( x.c_str() ); else y =! y; }

        _init_( arg, env );
    }

    cluster_t() : obj( new NODE() ){
        array_t<const char*> arg; array_t<const char*> env;
        _init_( arg, env );
    }

    /*─······································································─*/

    virtual void free() const noexcept {
        
        if( obj->state == -3 && obj.count() > 1 ){ resume(); return; }
        if( obj->state == -2 ){ return; } close(); obj->state = -2;
        obj->input.close(); obj->output.close();
        obj->error.close(); onClose.emit(); 

        if( is_parent() ){ kill(); }

    //  onClose .clear(); onDrain.clear(); 
        onResume.clear(); onError.clear();
        onStop  .clear(); onOpen .clear();
        onData  .clear(); onDout .clear(); 
        onDerr  .clear();

    }

    /*─······································································─*/

    int next() const noexcept {
        if( readable() .is_closed() ){ close(); return -1; }
        if( writable() .is_closed() ){ close(); return -1; }
        if( std_error().is_closed() ){ close(); return -1; }
    coStart; onOpen.emit(); coYield(1);

        if((*_read1)(&readable())==1 )       { coGoto(2); }
        if(  _read1->state <= 0 )            { coGoto(2); }
        onData.emit(_read1->data);
        onDout.emit(_read1->data);             coGoto(2);

        coYield(2); if( process::is_child() ){ coGoto(1); }

        if((*_read2)(&std_error())==1 )      { coGoto(1); }
        if(  _read2->state <= 0 )            { coGoto(1); }
        onData.emit(_read2->data);
        onDerr.emit(_read2->data);             coGoto(1);

    coStop
    }

    /*─······································································─*/

    bool is_alive()     const noexcept { return ::kill( obj->fd, 0 ) == 0; }
    bool is_available() const noexcept { return is_closed() == false; }
    bool is_closed()    const noexcept { return obj->state <= 0; }
    int  get_fd()       const noexcept { return obj->fd; }

    /*─······································································─*/

    void resume() const noexcept { if(obj->state== 0) { return; } obj->state= 0; onResume.emit(); }
    void  close() const noexcept { if(obj->state < 0) { return; } obj->state=-1; onDrain.emit(); }
    void   stop() const noexcept { if(obj->state==-3) { return; } obj->state=-3; onStop.emit(); }
    void  flush() const noexcept { writable().flush(); readable().flush(); std_error().flush(); }
    void   kill() const noexcept { ::kill( obj->fd, SIGKILL ); }

    /*─······································································─*/

    template< class... T >
    int werror( const T&... args )    const noexcept { return std_error().write( args... ); }

    template< class... T >
    int write( const T&... args )     const noexcept { return writable().write( args... ); }

    template< class... T >
    string_t read( const T&... args ) const noexcept { return readable().read( args... ); }

    /*─······································································─*/

    template< class... T >
    int _werror( const T&... args ) const noexcept { return std_error()._write( args... ); }

    template< class... T >
    int _write( const T&... args ) const noexcept { return writable()._write( args... ); }

    template< class... T >
    int _read( const T&... args )  const noexcept { return readable()._read( args... ); }

    /*─······································································─*/

    file_t& readable()  const noexcept { return obj->output; }
    file_t& writable()  const noexcept { return obj->input;  }
    file_t& std_error() const noexcept { return obj->error;  }

    /*─······································································─*/

    bool  is_child() const noexcept { return !process::env::get("CHILD").empty(); }
    bool is_parent() const noexcept { return  process::env::get("CHILD").empty(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/
