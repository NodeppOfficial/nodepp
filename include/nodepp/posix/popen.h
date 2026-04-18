/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_POPEN
#define NODEPP_POSIX_POPEN

/*────────────────────────────────────────────────────────────────────────────*/

#include <unistd.h>
#include <sys/wait.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class popen_t : public generator_t {
protected:

    void kill() const noexcept { 
        obj->state |= STATE::FS_STATE_KILL; 
    }

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
        int     fd    = -1;
        file_t  std_input ;
        file_t  std_error ;
        file_t  std_output;

       ~NODE(){ 
        if( fd == -1 ) /*------*/ { return; }
            ::kill( fd, SIGKILL ); int c=0;
        do{ /*unused*/ } while( ::waitpid( fd, &c, WNOHANG )<0 ); }

    };  ptr_t<NODE> obj;

    template< class T >
    void _init_( const string_t& path, T& arg, T& env ) {

        int fda[2]; if( ::pipe( fda )==-1 ){ throw except_t( "while piping stdin"  ); }
        int fdb[2]; if( ::pipe( fdb )==-1 ){ throw except_t( "while piping stdout" ); }
        int fdc[2]; if( ::pipe( fdc )==-1 ){ throw except_t( "while piping stderr" ); } 
        
        obj->fd = ::fork();

        if( obj->fd == 0 ){
            ::dup2( fda[0], STDIN_FILENO  ); ::close( fda[1] );
            ::dup2( fdb[1], STDOUT_FILENO ); ::close( fdb[0] ); arg.push( nullptr );
            ::dup2( fdc[1], STDERR_FILENO ); ::close( fdc[0] ); env.push( nullptr );
            ::execvpe( path.c_str(), (char**)arg.data(),(char**)env.data() );
            throw except_t("while spawning new popen");
        } elif ( obj->fd > 0 ){
            obj->std_input  = file_t( fda[1] ); ::close( fda[0] );
            obj->std_output = file_t( fdb[0] ); ::close( fdb[1] );
            obj->std_error  = file_t( fdc[0] ); ::close( fdc[1] );
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

    popen_t( const string_t& path, const initializer_t<string_t>& args, const initializer_t<string_t>& envs )
    : obj( new NODE() ) { if( path.empty() ){ throw except_t("invalid command"); }
        array_t<const char*> arg; array_t<const char*> env;
        for( auto x : args ) { arg.push( x.get() ); } /*---------------------*/
        for( auto x : envs ) { env.push( x.get() ); } _init_( path, arg, env );
    }

    popen_t( const string_t& path ) 
    : obj( new NODE() ) { if( path.empty() ){ throw except_t("invalid command"); }
        array_t<const char*> arg; array_t<const char*> env; auto cmd = regex::match_all( path, "[^ ]+" );
        for( auto x: cmd ){ arg.push( x.get() ); } _init_( cmd[0], arg, env ); /*----------------------*/
    }

    popen_t( const string_t& path, const initializer_t<string_t>& args ) 
    : obj( new NODE() ) { if ( path.empty() ){ throw except_t("invalid command"); }
        array_t<const char*> arg; array_t<const char*> env;
        for( auto x : args ) { arg.push( x.get() ); }
        _init_( path, arg, env ); /*---------------*/
    }

    popen_t() noexcept : obj( new NODE() ) {}

   ~popen_t() noexcept { if( obj.count()>1 && !is_closed() ){ return; } free(); }

    /*─······································································─*/

    void free() const noexcept {

        if( is_state( STATE::FS_STATE_REUSE ) && !std_input().is_feof() && obj.count()>1 ){ return; }
        if( is_state( STATE::FS_STATE_KILL  ) ) /*-------*/ { return; } 
        if(!is_state( STATE::FS_STATE_CLOSE | STATE::FS_STATE_REUSE ) ){ onDrain.emit(); }

        onError.clear(); onDerr.clear(); 
        onOpen .clear(); onData.clear(); 
        onDout .clear(); onClose.emit(); kill();

    }

    /*─······································································─*/

    bool is_alive() const noexcept {
        if( ::kill( obj->fd , 0 ) == -1 ){ return false; }
        if( std_error ().is_available() ){ return true;  }
        if( std_output().is_available() ){ return true;  } return false; 
    }

    /*─······································································─*/

    bool is_closed()    const noexcept { return is_state( STATE::FS_STATE_DISABLE ) || !is_alive() || std_output().is_closed(); }
    bool is_available() const noexcept { return is_closed() == false; }
    int  get_fd()       const noexcept { return obj->fd; }

    /*─······································································─*/

    void close() const noexcept {
        if( is_state ( STATE::FS_STATE_DISABLE ) ) { return; }
            onDrain.emit(); set_state( STATE::FS_STATE_CLOSE );
        free(); 
    }

    /*─······································································─*/

    file_t& std_error () const noexcept { return obj->std_error;  }
    file_t& std_output() const noexcept { return obj->std_output; }
    file_t& std_input () const noexcept { return obj->std_input;  }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/