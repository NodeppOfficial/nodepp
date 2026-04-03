/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_CLUSTER
#define NODEPP_POSIX_CLUSTER

/*────────────────────────────────────────────────────────────────────────────*/

#include <unistd.h>
#include <sys/wait.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class cluster_t : public generator_t {
protected:

    void kill() const noexcept { 
    if( obj->fd != -1 ){ if( is_parent() ){
        ::kill( obj->fd, SIGKILL ); int c=0;
    do{ /*unused*/ } 
        while( ::waitpid( obj->fd, &c, WNOHANG )<0 );
    } } obj->state |= STATE::FS_STATE_KILL; }

    bool is_state( uchar value ) const noexcept {
        if( obj->state & value ){ return true; }
    return false; }

    void set_state( uchar value ) const noexcept {
    if( obj->state & STATE::FS_STATE_KILL ){ return; }
        obj->state = value;
    }

    enum STATE {
         FS_STATE_UNKNOWN = 0b00000000,
         FS_STATE_OPEN    = 0b00000001,
         FS_STATE_CLOSE   = 0b00000010,
         FS_STATE_KILL    = 0b00000100,
         FS_STATE_REUSE   = 0b00001000,
         FS_STATE_DISABLE = 0b00001110
    };

protected:

    struct NODE {
        uchar   state = STATE::FS_STATE_CLOSE;
        int     fd = -1;
        file_t    input;
        file_t   output;
        file_t    error;
    };  ptr_t<NODE> obj;

    template< class T > void _init_( T& arg, T& env ) {

        if( process::is_child() ){
            obj->input = fs::std_output(); obj->error = fs::std_error(); 
            obj->output= fs::std_input (); set_state( STATE::FS_STATE_OPEN ); 
        return; }

        int fda[2]; if( ::pipe( fda )==-1 ){ throw except_t( "while piping stdin"  ); }
        int fdb[2]; if( ::pipe( fdb )==-1 ){ throw except_t( "while piping stdout" ); }
        int fdc[2]; if( ::pipe( fdc )==-1 ){ throw except_t( "while piping stderr" ); } 
        
        obj->fd = ::fork();

        if( obj->fd == 0 ){
            auto chl = string::format( "CHILD=TRUE", fda[0], fdb[1] ); 
            arg.unshift( process::arguments()[0].get() ); /*-*/ env.push( chl.c_str() );
            ::dup2( fda[0], STDIN_FILENO  ); ::close( fda[1] ); arg.push( nullptr );
            ::dup2( fdb[1], STDOUT_FILENO ); ::close( fdb[0] ); env.push( nullptr );
            ::dup2( fdc[1], STDERR_FILENO ); ::close( fdc[0] ); /*----------------*/
            ::execvpe( arg[0], (char**) arg.data(), (char**) env.data() );
            throw except_t("while spawning new cluster");
        } elif ( obj->fd > 0 ) {
            obj->input  = file_t(fda[1]); ::close( fda[0] );
            obj->output = file_t(fdb[0]); ::close( fdb[1] );
            obj->error  = file_t(fdc[0]); ::close( fdc[1] );
            set_state( STATE::FS_STATE_OPEN );
        } else {
            ::close( fda[0] ); ::close( fda[1] );
            ::close( fdb[0] ); ::close( fdb[1] );
            ::close( fdc[0] ); ::close( fdc[1] );
            set_state( STATE::FS_STATE_CLOSE );
        }

    }

public:

    event_t<except_t>  onError;
    event_t<>          onClose;
    event_t<>          onDrain;
    event_t<>          onOpen;

    event_t<string_t>  onData;
    event_t<string_t>  onDout;
    event_t<string_t>  onDerr;

    cluster_t( const initializer_t<string_t>& args, const initializer_t<string_t>& envs )
    : obj( new NODE() ) {
        array_t<const char*> arg; array_t<const char*> env;
        for( auto x : args ) { arg.push( x.get() ); } /*---------------*/
        for( auto x : envs ) { env.push( x.get() ); } _init_( arg, env );
    }

    cluster_t( const initializer_t<string_t>& args ) : obj( new NODE() ){
        array_t<const char*> arg; array_t<const char*> env; /*---------*/
        for( auto x : args ) { arg.push( x.get() ); } _init_( arg, env );
    }

    cluster_t() : obj( new NODE() ) {
        array_t<const char*> arg; array_t<const char*> env; 
        _init_( arg, env ); /*---------------------------*/
    }

   ~cluster_t() noexcept { if( obj.count()>1 && !is_closed() ){ return; } free(); }

    /*─······································································─*/

    void free() const noexcept {

        if( is_state( STATE::FS_STATE_REUSE ) && !readable().is_feof() && obj.count()>1 ){ return; }
        if( is_state( STATE::FS_STATE_KILL  ) ) /*-------*/ { return; } 
        if(!is_state( STATE::FS_STATE_CLOSE | STATE::FS_STATE_REUSE ) )
          { kill(); onDrain.emit(); } else { kill(); }

        onError.clear(); onDerr.clear(); 
        onOpen .clear(); onData.clear(); 
        onDout .clear(); onClose.emit();

    }

    /*─······································································─*/

    bool is_alive() const noexcept {
        if( is_parent() && ::kill( obj->fd , 0 ) ==-1 ){ return false; }
        if( readable ().is_available() ){ return true; }
        if( std_error().is_available() ){ return true; } return false; 
    }

    /*─······································································─*/

    bool is_closed()    const noexcept { return is_state( STATE::FS_STATE_DISABLE ) || !is_alive() || readable().is_closed(); }
    bool is_available() const noexcept { return is_closed() == false; }
    int  get_fd()       const noexcept { return obj->fd; }

    /*─······································································─*/

    void close() const noexcept {
        if( is_state ( STATE::FS_STATE_DISABLE ) ){ return; }
            set_state( STATE::FS_STATE_CLOSE   );
        onDrain.emit(); free(); 
    }

    /*─······································································─*/

    file_t& readable()  const noexcept { return obj->output; }
    file_t& writable()  const noexcept { return obj->input;  }
    file_t& std_error() const noexcept { return obj->error;  }

    /*─······································································─*/

    bool  is_child() const noexcept { return !process::env::get("CHILD").empty(); }
    bool is_parent() const noexcept { return  process::env::get("CHILD").empty(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/