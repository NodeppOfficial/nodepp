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

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class cluster_t : public generator_t {
private:

    using _read_ = generator::file::read;

protected:

    ptr_t<_read_> _read1 = new _read_();
    ptr_t<_read_> _read2 = new _read_();

    struct NODE {
        PROCESS_INFORMATION pi;
        STARTUPINFO  si;
        int          fd;
        int    state =0;
        file_t    input;
        file_t   output;
        file_t    error;
    };  ptr_t<NODE> obj;

    void _init_( array_t<string_t> arg, array_t<string_t> env ) {

        if( process::is_child() ){
            obj->input = fs::std_output(); obj->error = fs::std_error(); 
            obj->output= fs::std_input (); obj->state = 1; return;
        }

        SECURITY_ATTRIBUTES sa;
                            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                            sa.lpSecurityDescriptor = NULL; 
                            sa.bInheritHandle = TRUE;

        HANDLE fda[2]; CreatePipe( &fda[0], &fda[1], &sa, CHUNK_SIZE );
        HANDLE fdb[2]; CreatePipe( &fdb[0], &fdb[1], &sa, CHUNK_SIZE );
        HANDLE fdc[2]; CreatePipe( &fdc[0], &fdc[1], &sa, CHUNK_SIZE );

        ZeroMemory(&obj->si, sizeof(STARTUPINFO));
        ZeroMemory(&obj->pi, sizeof(PROCESS_INFORMATION));
                    obj->si.cb        = sizeof( STARTUPINFO );
                    obj->si.hStdInput = fda[0];
                    obj->si.hStdError = fdc[1];
                    obj->si.hStdOutput= fdb[1];
                    obj->si.dwFlags  |= STARTF_USESTDHANDLES;

        arg.unshift( process::args[0].get () ); arg.push( "?CHILD=TRUE" );
        auto CMD = arg.join( string::space() );
        auto ENV = env.join( string::null () );

        obj->fd = ::CreateProcess( NULL, CMD.get(), NULL, NULL, 1, 0, ENV.get(), NULL, &obj->si, &obj->pi );
        WaitForSingleObject( obj->pi.hProcess, 0 );
        WaitForSingleObject( obj->pi.hThread , 0 );

        if ( obj->fd != 0 ){
            obj->input  = file_t( fda[1] ); ::CloseHandle( fda[0] );
            obj->output = file_t( fdb[0] ); ::CloseHandle( fdb[1] );
            obj->error  = file_t( fdc[0] ); ::CloseHandle( fdc[1] );
            obj->state  = 1;
        } else {
            ::CloseHandle ( fda[0] ); ::CloseHandle ( fda[1] );
            ::CloseHandle ( fdb[0] ); ::CloseHandle ( fdb[1] );
            ::CloseHandle ( fdc[0] ); ::CloseHandle ( fdc[1] );
            obj->state  = 0;
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

    cluster_t( const initializer_t<string_t>& args, const initializer_t<string_t>& envs ) 
    : obj( new NODE() ) { _init_( args, envs ); }

   ~cluster_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    cluster_t() : obj( new NODE() ){ _init_( nullptr, nullptr ); }

    cluster_t( const initializer_t<string_t>& args ) 
    : obj( new NODE() ) { _init_( args, nullptr ); }

    /*─······································································─*/

    void free() const noexcept {
        
        if( obj->state == -3 && obj.count() > 1 ){ resume(); return; }
        if( obj->state == -2 ){ return; } close(); obj->state = -2;
            obj->input.close(); obj->output.close();
            obj->error.close();

        if( is_parent() ){ kill(); }

        onResume.clear(); onError.clear(); 
        onStop  .clear(); onOpen .clear();
        onData  .clear(); onDout .clear(); 
        onDerr  .clear(); onClose.emit ();
        
    }

    /*─······································································─*/

    int next() noexcept {
    coBegin; if( !is_closed() ){ 

        onOpen.emit(); coYield(1);

        if((*_read1)(&readable())==1)  { coGoto(2); }
        if(  _read1->state <= 0 )      { coGoto(2); }
        onData.emit(_read1->data);
        onDout.emit(_read1->data);       coYield(2);

        if( !is_alive()&&_read1->state<=0 ){ break; }
        if( process::is_child() )      { coGoto(1); }

        if((*_read2)(&std_error())==1 ){ coGoto(1); }
        if(  _read2->state <= 0 )      { coGoto(1); }
        onData.emit(_read2->data);
        onDerr.emit(_read2->data);       
        
    coGoto(1); } coFinish
    }

    /*─······································································─*/

    bool is_alive() const noexcept { DWORD exitCode;
        if( GetExitCodeProcess(obj->pi.hProcess,&exitCode) ){
        if( exitCode == STILL_ACTIVE ) { return true; }     } return false;
    }

    bool is_available() const noexcept { return is_closed()== false; }
    bool is_closed()    const noexcept { return obj->state <= 0; }
    int  get_fd()       const noexcept { return obj->fd; }

    /*─······································································─*/

    void   kill() const noexcept { ::CloseHandle( obj->pi.hProcess ); ::CloseHandle( obj->pi.hThread ); }
    void resume() const noexcept { if(obj->state== 0) { return; } obj->state= 0; onResume.emit(); }
    void  close() const noexcept { if(obj->state < 0) { return; } obj->state=-1; onDrain.emit(); }
    void   stop() const noexcept { if(obj->state==-3) { return; } obj->state=-3; onStop.emit(); }
    void  flush() const noexcept { writable().flush(); readable().flush(); std_error().flush(); }

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