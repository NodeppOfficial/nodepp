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
#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class file_t {
protected:

    void kill() const noexcept { 
        obj->state |= FILE_STATE::KILL;
        CloseHandle( obj->fd ); 
    }

    bool is_state( uchar value ) const noexcept {
        if( obj->state & value ){ return true; }
    return false; }

    void set_state( uchar value ) const noexcept {
    if( obj->state & KILL ){ return; }
        obj->state = value;
    }

    enum FILE_STATE {
        UNKNOWN = 0b00000000,
        OPEN    = 0b00000001,
        CLOSE   = 0b00000010,
        KILL    = 0b00000100,
        REUSE   = 0b00001000,
        DISABLE = 0b00001110
    };

protected:

    struct NODE {
        HANDLE       fd       = INVALID_HANDLE_VALUE;
        uchar        state    = FILE_STATE::OPEN;
        ulong        range[2] = { 0, 0 };
        bool         keep     = false;
        int          feof     =   1;
        OVERLAPPED   ov    ;
        ptr_t<char>  buffer;
        string_t     borrow;
        limit::probe_t limit_probe;
    };  ptr_t<NODE> obj;
    
    /*─······································································─*/

    ptr_t<uint> get_fd_flag( const string_t& flag ){ 
        ptr_t<uint> fg ({ 0x00, FILE_SHARE_READ|FILE_SHARE_WRITE, 0x00, FILE_FLAG_OVERLAPPED });
          if( flag == "r"  ){ fg[0] |= GENERIC_READ;               fg[2] |= OPEN_EXISTING; }
        elif( flag == "w"  ){ fg[0] |= GENERIC_WRITE;              fg[2] |= CREATE_ALWAYS; }
        elif( flag == "a"  ){ fg[0] |= FILE_APPEND_DATA;           fg[2] |= OPEN_ALWAYS;   }
        elif( flag == "r+" ){ fg[0] |= GENERIC_READ|GENERIC_WRITE; fg[2] |= OPEN_EXISTING; }
        elif( flag == "w+" ){ fg[0] |= GENERIC_READ|GENERIC_WRITE; fg[2] |= OPEN_ALWAYS;   }
        elif( flag == "a+" ){ fg[0] |= FILE_APPEND_DATA;           fg[2] |= OPEN_EXISTING; }
        else                { fg[0] |= GENERIC_READ|GENERIC_WRITE; fg[2] |= OPEN_EXISTING; }
        return  fg;
    }
    
    /*─······································································─*/
    
    virtual int set_nonbloking_mode() const noexcept { return 0; }
    
    /*─······································································─*/

    virtual bool is_blocked( const bool& x, DWORD& c ) const noexcept {
        if ( x || c != 0 ){ return 0; }
        if ( GetOverlappedResult( obj->fd, &obj->ov, &c, 0 ) )
           { obj->ov.Offset += c; } 
        return os::error() == ERROR_IO_INCOMPLETE ? 1 : //c==0
               os::error() == ERROR_HANDLE_EOF    ? 0 :
               os::error() == ERROR_IO_PENDING    ? 1 : 0;
    }
    
public:

    event_t<>          onUnpipe;
    event_t<>          onResume;
    event_t<except_t>  onError;
    event_t<>          onDrain;
    event_t<>          onClose;
    event_t<>          onStop;
    event_t<>          onOpen;
    event_t<>          onPipe;
    event_t<string_t>  onData;
    
    /*─······································································─*/
    
    virtual ~file_t() noexcept { if( obj.count()>1 ){ return; } free(); }
    
    /*─······································································─*/

    file_t( const string_t& path, const string_t& mode, const ulong& _size=CHUNK_SIZE ) : obj( new NODE() ) {
        auto fg = get_fd_flag( mode ); obj->fd = CreateFileA( path.c_str(), fg[0], fg[1], NULL, fg[2], fg[3], NULL ); 
        if( obj->fd == INVALID_HANDLE_VALUE ){ throw except_t("such file or directory does not exist"); }
        set_nonbloking_mode(); set_buffer_size( _size ); 
    }

    file_t( const HANDLE& fd, const ulong& _size=CHUNK_SIZE ) : obj( new NODE() ) {
        if( fd == INVALID_HANDLE_VALUE ){ throw except_t("such file or directory does not exist"); }
        obj->fd = fd; set_nonbloking_mode(); set_buffer_size( _size ); 
    }

    file_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    bool     is_closed() const noexcept { return is_state(FILE_STATE::DISABLE) || is_feof() || obj->fd==INVALID_HANDLE_VALUE; }
    bool       is_feof() const noexcept { return obj->feof <= 0 && obj->feof != -2; }
    bool  is_available() const noexcept { return !is_closed(); }
    bool is_persistent() const noexcept { return obj->keep; }

    /*─······································································─*/

    void  resume() const noexcept { if(is_state(FILE_STATE::OPEN )){ return; } set_state(FILE_STATE::OPEN ); onResume.emit(); }
    void    stop() const noexcept { if(is_state(FILE_STATE::REUSE)){ return; } set_state(FILE_STATE::REUSE); onStop  .emit(); }
    void   reset() const noexcept { if(is_state(FILE_STATE::KILL )){ return; } resume(); pos(0); }
    void   flush() const noexcept { obj->buffer.fill(0); }

    /*─······································································─*/

    void close() const noexcept { 
        if( is_state(FILE_STATE::DISABLE) ){ return; }
        if( obj->keep==1 ){ stop(); goto DONE; }
         set_state( FILE_STATE::CLOSE ); DONE:; 
    onDrain.emit(); if( is_feof() ){ free(); } }
    
    /*─······································································─*/

    void set_range( ulong x, ulong y ) const noexcept { obj->range[0] = x; obj->range[1] = y; }
    ulong* get_range() const noexcept { return obj == nullptr ? nullptr : obj->range; }
    HANDLE    get_fd() const noexcept { return obj == nullptr ? nullptr : obj->fd; }
    
    /*─······································································─*/

    void   set_borrow( const string_t& brr ) const noexcept { obj->borrow = brr; }
    ulong  get_borrow_size() const noexcept { return obj->borrow.size(); }
    char*  get_borrow_data() const noexcept { return obj->borrow.data(); }
    void        del_borrow() const noexcept { obj->borrow.clear(); }
    string_t&   get_borrow() const noexcept { return obj->borrow; }
    
    /*─······································································─*/

    ulong   get_buffer_size() const noexcept { return obj->buffer.size(); }
    char*   get_buffer_data() const noexcept { return obj->buffer.data(); }
    ptr_t<char>& get_buffer() const noexcept { return obj->buffer; }
    
    /*─······································································─*/

    ulong pos( ulong _pos ) const noexcept { obj->ov.Offset = _pos; return _pos; }

    ulong size() const noexcept { LARGE_INTEGER size;
        while( !GetFileSizeEx(obj->fd,&size) ){ process::next(); }
        return size.QuadPart;
    }

    ulong pos() const noexcept { return obj->ov.Offset; }
    
    /*─······································································─*/

    virtual ulong set_buffer_size( ulong _size ) const noexcept { 
        obj->buffer = ptr_t<char>( _size ); return _size;
    }
    
    /*─······································································─*/

    virtual void free() const noexcept {

        if( is_state(FILE_STATE::REUSE) && obj.count() > 1 ){ resume(); return; }
        if( is_state(FILE_STATE::KILL ) ){ return; } close(); kill();
       
        onUnpipe.clear(); onResume.clear();
        onError .clear(); onStop  .clear();
        onOpen  .clear(); onPipe  .clear();
        onData  .clear(); /*-------------*/ onClose.emit();

    }
    
    /*─······································································─*/

    char read_char() const noexcept { return read(1)[0]; }

    string_t read_until( string_t ch ) const noexcept {
        auto gen = generator::file::until();
        while( gen( this, ch ) == 1 )
             { process::next(); }
        return gen.data;
    }

    string_t read_until( char ch ) const noexcept {
        auto gen = generator::file::until();
        while( gen( this, ch ) == 1 )
             { process::next(); }
        return gen.data;
    }

    string_t read_line() const noexcept {
        auto gen = generator::file::line();
        while( gen( this ) == 1 )
             { process::next(); }
        return gen.data;
    }
    
    /*─······································································─*/

    string_t read( ulong size=CHUNK_SIZE ) const noexcept {
        auto gen = generator::file::read();
        while( gen( this, size ) == 1 )
             { process::next(); }
        return gen.data;
    }

    ulong write( const string_t& msg ) const noexcept {
        auto gen = generator::file::write();
        while( gen( this, msg ) == 1 )
             { process::next(); }
        return gen.data;
    }
    
    /*─······································································─*/

    virtual int _read ( char* bf, const ulong& sx ) const noexcept { return __read ( bf, sx ); }
    virtual int _write( char* bf, const ulong& sx ) const noexcept { return __write( bf, sx ); }
    
    /*─······································································─*/

    virtual int __read( char* bf, const ulong& sx ) const noexcept {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; } DWORD c = 0; 
        obj->feof = ReadFile( obj->fd, bf, sx, &c, &obj->ov );
        obj->feof = is_blocked( obj->feof, c ) ? -2 : c;
        if( obj->feof <= 0 && obj->feof != -2 ){ return -1; }
        return obj->feof;
    }

    virtual int __write( char* bf, const ulong& sx ) const noexcept {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; } DWORD c = 0; 
        obj->feof = WriteFile( obj->fd, bf, sx, &c, &obj->ov );
        obj->feof = is_blocked( obj->feof, c ) ? -2 : c;
        if( obj->feof <= 0 && obj->feof != -2 ){ return -1; }
        return obj->feof;
    }

    /*─······································································─*/

    bool _write_( char* bf, const ulong& sx, ulong& sy ) const noexcept {
        if( sx==0 || is_closed() ){ return 1; } while( sy < sx ) {
            int c = __write( bf+sy, sx-sy );
            if( c <= 0 && c != -2 ) /*----*/ { return 0; }
            if( c >  0 ){ sy += c; continue; } return 1;
        }   return 0;
    }

    bool _read_( char* bf, const ulong& sx, ulong& sy ) const noexcept {
        if( sx==0 || is_closed() ){ return 1; } while( sy < sx ) {
            int c = __read( bf+sy, sx-sy );
            if( c <= 0 && c != -2 ) /*----*/ { return 0; }
            if( c >  0 ){ sy += c; continue; } return 1;
        }   return 0;
    }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/