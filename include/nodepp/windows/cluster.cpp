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
#include "pipe.cpp"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class cluster_t {
protected:

    ptr_t<_file_::read> _read1 = new _file_::read;
    ptr_t<_file_::read> _read2 = new _file_::read;

    struct NODE {
        PROCESS_INFORMATION pi;
        STARTUPINFO  si;
        int          fd;
        int    state =1;
        file_t    input;
        file_t   output;
        file_t    error;
    };  ptr_t<NODE> obj;

    template< class T >
    void _init_( T& arg, T& env ) {

        if( process::is_child() ){
            obj->output = fs::std_output();
            obj->input  = fs::std_input();
            obj->error  = fs::std_error(); return;
        }

        SECURITY_ATTRIBUTES sa;
                            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                            sa.lpSecurityDescriptor = NULL;
                            sa.bInheritHandle = TRUE;

        HANDLE fda[2]; CreatePipe( &fda[0], &fda[1], &sa, CHUNK_SIZE, FILE_FLAG_OVERLAPPED, FILE_FLAG_OVERLAPPED );
        HANDLE fdb[2]; CreatePipe( &fdb[0], &fdb[1], &sa, CHUNK_SIZE, FILE_FLAG_OVERLAPPED, FILE_FLAG_OVERLAPPED );
        HANDLE fdc[2]; CreatePipe( &fdc[0], &fdc[1], &sa, CHUNK_SIZE, FILE_FLAG_OVERLAPPED, FILE_FLAG_OVERLAPPED );

        ZeroMemory(&obj->si, sizeof(STARTUPINFO));
        ZeroMemory(&obj->pi, sizeof(PROCESS_INFORMATION));
                    obj->si.cb         = sizeof( STARTUPINFO );
                    obj->si.hStdInput  = fda[0];
                    obj->si.hStdError  = fdc[1];
                    obj->si.hStdOutput = fdb[1];
                    obj->si.dwFlags   |= STARTF_USESTDHANDLES;

        arg.unshift( process::args[0].c_str() ); auto cmd = arg.join(" ");
        env.push("CHILD=TRUE"); auto ven = env.join( "\0" );
        auto dta = LPTSTR( ven.empty() ? NULL : ven.get() );

        obj->fd = ::CreateProcess( NULL, cmd.data(), NULL, NULL, 1, 0, dta, NULL, &obj->si, &obj->pi );
        WaitForSingleObject( obj->pi.hProcess, 0 ); WaitForSingleObject( obj->pi.hThread, 0 );

        if ( obj->fd != 0 ){ // Parent process
            obj->input  = { fda[1] }; ::CloseHandle( fda[0] );
            obj->output = { fdb[0] }; ::CloseHandle( fdb[1] );
            obj->error  = { fdc[0] }; ::CloseHandle( fdc[1] );
            obj->state  = 1;
        } else {
            ::CloseHandle ( fda[0] ); ::CloseHandle ( fda[1] );
            ::CloseHandle ( fdb[0] ); ::CloseHandle ( fdb[1] );
            ::CloseHandle ( fdc[0] ); ::CloseHandle ( fdc[1] );
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

    bool is_available() const noexcept { return is_closed() == false; }
    bool is_closed()    const noexcept { return obj->state <= 0; }
    int  get_fd()       const noexcept { return obj->fd; }

    bool is_alive() const noexcept { DWORD exitCode;
        if( GetExitCodeProcess(obj->pi.hProcess,&exitCode) ) {
        if( exitCode == STILL_ACTIVE ) { return true; }      } return false;
    }

    /*─······································································─*/

    void   kill() const noexcept { ::CloseHandle( obj->pi.hProcess ); ::CloseHandle( obj->pi.hThread ); }
    void  flush() const noexcept { readable().flush(); writable().flush(); std_error().flush(); }
    void resume() const noexcept { if(obj->state== 0) { return; } obj->state= 0; onResume.emit(); }
    void  close() const noexcept { if(obj->state < 0) { return; } obj->state=-1; onDrain.emit(); }
    void   stop() const noexcept { if(obj->state==-3) { return; } obj->state=-3; onStop.emit(); }

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
