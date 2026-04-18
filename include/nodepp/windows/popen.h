/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_POPEN
#define NODEPP_WINDOWS_POPEN

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

        uchar /*------*/ state=STATE::FS_STATE_CLOSE;
        PROCESS_INFORMATION pi;
        file_t std_output;
        file_t std_input;
        file_t std_error;
        STARTUPINFO   si;
        int           fd;

       ~NODE(){
            ::CloseHandle( obj->pi.hProcess ); 
            ::CloseHandle( obj->pi.hThread  ); 
        }

    };  ptr_t<NODE> obj;

    void _init_( const string_t& path, array_t<string_t> arg, array_t<string_t> env ) {

        SECURITY_ATTRIBUTES sa;
        /*---------------*/ sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        /*---------------*/ sa.lpSecurityDescriptor = NULL; 
        /*---------------*/ sa.bInheritHandle /*-*/ = TRUE;

        HANDLE fda[2]; if(!CreatePipe(&fda[0],&fda[1],&sa,CHUNK_SIZE)){ throw except_t( "while piping stdin"  ); }
        HANDLE fdb[2]; if(!CreatePipe(&fdb[0],&fdb[1],&sa,CHUNK_SIZE)){ throw except_t( "while piping stdout" ); }
        HANDLE fdc[2]; if(!CreatePipe(&fdc[0],&fdc[1],&sa,CHUNK_SIZE)){ throw except_t( "while piping stderr" ); }

        ZeroMemory(&obj->si, sizeof(STARTUPINFO));
        ZeroMemory(&obj->pi, sizeof(PROCESS_INFORMATION));
                    obj->si.cb        = sizeof( STARTUPINFO );
                    obj->si.hStdInput = fda[0];
                    obj->si.hStdError = fdc[1];
                    obj->si.hStdOutput= fdb[1];
                    obj->si.dwFlags  |= STARTF_USESTDHANDLES;

        arg.unshift( path ); 
        auto CMD = arg.join( string::space() ); 
        auto ENV = env.join( string::null () );

        obj->fd = ::CreateProcess( NULL, CMD.get(), NULL, NULL, 1, 0, ENV.get(), NULL, &obj->si, &obj->pi );
        WaitForSingleObject( obj->pi.hProcess, 0 ); 
        WaitForSingleObject( obj->pi.hThread , 0 );

        if( obj->fd != 0 ){
            obj->std_input  = file_t( fda[1] ); ::CloseHandle( fda[0] );
            obj->std_output = file_t( fdb[0] ); ::CloseHandle( fdb[1] );
            obj->std_error  = file_t( fdc[0] ); ::CloseHandle( fdc[1] );
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

    popen_t( const string_t& path, const initializer_t<string_t>& args, const initializer_t<string_t>& envs )
    : obj( new NODE() ) { _init_( path, args, envs ); }

    popen_t( const string_t& path, const initializer_t<string_t>& args )
    : obj( new NODE() ) { _init_( path, args, nullptr ); }

    popen_t( const string_t& path ) 
    : obj( new NODE() ) { auto cmd = regex::match_all( path, "[^ ]+" );
        if ( cmd.empty() ){ throw except_t("invalid command"); }
        _init_( cmd[0], cmd.slice(1), nullptr );
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

    bool is_alive() const noexcept { DWORD exitCode;
        if( GetExitCodeProcess(obj->pi.hProcess,&exitCode) ){
        if( exitCode == STILL_ACTIVE ) { return true; }} return false;
    }

    /*─······································································─*/

    bool is_closed()    const noexcept { return is_state( STATE::FS_STATE_DISABLE ) || !is_alive() || std_output().is_closed(); }
    bool is_available() const noexcept { return is_closed() == false; }
    int  get_fd()       const noexcept { return obj->fd; }

    /*─······································································─*/

    void close() const noexcept {
    if( is_state ( STATE::FS_STATE_DISABLE ) ) { return; }
        onDrain.emit(); set_state( STATE::FS_STATE_CLOSE );
    free(); }

    /*─······································································─*/

    file_t& std_error()  const noexcept { return obj->std_error;  }
    file_t& std_output() const noexcept { return obj->std_output; }
    file_t& std_input()  const noexcept { return obj->std_input;  }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/