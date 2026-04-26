/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_CLUSTER
#define NODEPP_WINDOWS_CLUSTER

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class cluster_t : public generator_t {
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

        uchar     state=STATE::FS_STATE_CLOSE;
        PROCESS_INFORMATION pi;
        STARTUPINFO  si;
        int          fd;
        file_t    input;
        file_t   output;
        file_t    error;

       ~NODE(){
        if( !process::env::get("CHILD").empty() ){ return; }
            ::CloseHandle( obj->pi.hProcess ); 
            ::CloseHandle( obj->pi.hThread  ); 
        }

    };  ptr_t<NODE> obj;

    void _init_( array_t<string_t> arg, array_t<string_t> env ) {

        if( process::is_child() ){
            obj->input = fs::std_output(); obj->error = fs::std_error(); 
            obj->output= fs::std_input (); set_state( STATE::FS_STATE_OPEN ); 
        return; }

        SECURITY_ATTRIBUTES sa;
        /*---------------*/ sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        /*---------------*/ sa.lpSecurityDescriptor = NULL; 
        /*---------------*/ sa.bInheritHandle /*-*/ = TRUE;

        HANDLE fda[2]; if(!CreatePipe(&fda[0],&fda[1],&sa,CHUNK_SIZE)){ NODEPP_THROW_ERROR( "while piping stdin"  ); }
        HANDLE fdb[2]; if(!CreatePipe(&fdb[0],&fdb[1],&sa,CHUNK_SIZE)){ NODEPP_THROW_ERROR( "while piping stdout" ); }
        HANDLE fdc[2]; if(!CreatePipe(&fdc[0],&fdc[1],&sa,CHUNK_SIZE)){ NODEPP_THROW_ERROR( "while piping stderr" ); }

        ZeroMemory(&obj->si, sizeof(STARTUPINFO));
        ZeroMemory(&obj->pi, sizeof(PROCESS_INFORMATION));
                    obj->si.cb        = sizeof( STARTUPINFO );
                    obj->si.hStdInput = fda[0];
                    obj->si.hStdError = fdc[1];
                    obj->si.hStdOutput= fdb[1];
                    obj->si.dwFlags  |= STARTF_USESTDHANDLES;

        arg.unshift( process::arguments()[0].get() ); 
        arg.push( "?CHILD=TRUE" );
        
        auto CMD = arg.join( string::space() );
        auto ENV = env.join( string::null () );

        obj->fd = ::CreateProcess( NULL, CMD.get(), NULL, NULL, 1, 0, ENV.get(), NULL, &obj->si, &obj->pi );
        WaitForSingleObject( obj->pi.hProcess, 0 );
        WaitForSingleObject( obj->pi.hThread , 0 );

        if ( obj->fd != 0 ){
            obj->input  = file_t( fda[1] ); ::CloseHandle( fda[0] );
            obj->output = file_t( fdb[0] ); ::CloseHandle( fdb[1] );
            obj->error  = file_t( fdc[0] ); ::CloseHandle( fdc[1] );
            set_state( STATE::FS_STATE_OPEN );
        } else {
            ::CloseHandle ( fda[0] ); ::CloseHandle ( fda[1] );
            ::CloseHandle ( fdb[0] ); ::CloseHandle ( fdb[1] );
            ::CloseHandle ( fdc[0] ); ::CloseHandle ( fdc[1] );
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
    : obj( new NODE() ) { _init_( args, envs ); }

   ~cluster_t() noexcept { if( obj.count()>1 && !is_closed() ){ return; } free(); }

    cluster_t() : obj( new NODE() ){ _init_( nullptr, nullptr ); }

    cluster_t( const initializer_t<string_t>& args ) 
    : obj( new NODE() ) { _init_( args, nullptr ); }

    /*─······································································─*/

    void free() const noexcept {

        if( is_state( STATE::FS_STATE_REUSE ) && !readable().is_feof() && obj.count() >1 ){ return; }
        if( is_state( STATE::FS_STATE_KILL  ) ) /*-------*/ { return; } 
        if(!is_state( STATE::FS_STATE_CLOSE | STATE::FS_STATE_REUSE ) ){ onDrain.emit(); }

        onError.clear(); onDerr.clear(); 
        onOpen .clear(); onData.clear(); 
        onDout .clear(); onClose.emit(); kill();

    }

    /*─······································································─*/

    bool is_alive() const noexcept { DWORD exitCode;
        if( GetExitCodeProcess(obj->pi.hProcess,&exitCode) ){
        if( exitCode == STILL_ACTIVE ) { return true; }} return false;
    }

    /*─······································································─*/

    bool is_closed()    const noexcept { return is_state( STATE::FS_STATE_DISABLE ) || !is_alive() || readable().is_closed(); }
    bool is_available() const noexcept { return is_closed() == false; }
    int  get_fd()       const noexcept { return obj->fd; }

    /*─······································································─*/

    void close() const noexcept {
        if( is_state ( STATE::FS_STATE_DISABLE ) ) { return; }
            onDrain.emit(); set_state( STATE::FS_STATE_CLOSE );
        free(); 
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