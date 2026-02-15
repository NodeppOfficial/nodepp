/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_FILE
#define NODEPP_WINDOWS_FILE

/*────────────────────────────────────────────────────────────────────────────*/

#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class file_t {
protected:

    void kill() const noexcept { 
        obj->state |= STATE::FS_STATE_KILL;
        CancelIoEx((HANDLE)obj->fd, &obj->ovr);
        CancelIoEx((HANDLE)obj->fd, &obj->ovw);
    if( !is_std() ){ CloseHandle( obj->fd ); }}

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
         FS_STATE_READING = 0b00010000,
         FS_STATE_WRITING = 0b00100000,
         FS_STATE_KILL    = 0b00000100,
         FS_STATE_REUSE   = 0b00001000,
         FS_STATE_DISABLE = 0b00001110
    };

protected:

    struct NODE {

        OVERLAPPED  ovr, ovw ;

        HANDLE      fd       = INVALID_HANDLE_VALUE;
        ulong       range[2] = { 0, 0 };
        DWORD       offset   = 0;
        int         feof     = 1;
        uchar       state    = STATE::FS_STATE_OPEN;

        ptr_t<char> buffer; string_t borrow;
        generator::file::until _until;
        generator::file::line  _line ;
        generator::file::read  _read ;
        generator::file::write _write;
    };  ptr_t<NODE> obj;
    
    /*─······································································─*/

    bool is_std() const noexcept { 
        return obj->fd == GetStdHandle( STD_INPUT_HANDLE ) ||
               obj->fd == GetStdHandle( STD_OUTPUT_HANDLE) ||
               obj->fd == GetStdHandle( STD_ERROR_HANDLE ) ;
    }
    
    /*─······································································─*/

    ptr_t<uint> get_fd_flag( const string_t& flag ){ 
        ptr_t<uint> fg ({ 0x00,FILE_SHARE_READ|FILE_SHARE_WRITE,0x00,FILE_FLAG_OVERLAPPED });
        if  ( flag == "r"  ){ fg[0] |= GENERIC_READ;               fg[2] |= OPEN_EXISTING; }
        elif( flag == "w"  ){ fg[0] |= GENERIC_WRITE;              fg[2] |= CREATE_ALWAYS; }
        elif( flag == "a"  ){ fg[0] |= FILE_APPEND_DATA;           fg[2] |= OPEN_ALWAYS;   }
        elif( flag == "r+" ){ fg[0] |= GENERIC_READ|GENERIC_WRITE; fg[2] |= OPEN_EXISTING; }
        elif( flag == "w+" ){ fg[0] |= GENERIC_READ|GENERIC_WRITE; fg[2] |= OPEN_ALWAYS;   }
        elif( flag == "a+" ){ fg[0] |= FILE_APPEND_DATA;           fg[2] |= OPEN_EXISTING; }
        else                { fg[0] |= GENERIC_READ|GENERIC_WRITE; fg[2] |= OPEN_EXISTING; }
        return  fg;
    }
    
    /*─······································································─*/
    
    int set_nonbloking_mode() const noexcept { return 0; }
    
    /*─······································································─*/

    bool is_blocked( DWORD /*unused*/ ) const noexcept {
        DWORD err = GetLastError();
        return( err == ERROR_IO_INCOMPLETE || err == ERROR_IO_PENDING );
    }

    bool is_blocked( bool mode, DWORD& c ) const noexcept {
    auto ov = mode ? &obj->ovw : &obj->ovr;
    
        if( !HasOverlappedIoCompleted(ov) ) { return 1; }

        if( obj->state & ( STATE::FS_STATE_READING | STATE::FS_STATE_WRITING ) ){
        if( GetOverlappedResult((HANDLE)obj->fd, ov, &c, FALSE) )
          { goto DONE; }} else { goto DONE; }

        if( is_blocked( c ) ){ return 1; }

    DONE:; obj->offset+= c; return 0; }
    
public:

    event_t<>          onUnpipe;
    event_t<>          onResume;
    event_t<except_t>  onError;
    event_t<>          onDrain;
    event_t<>          onClose;
    event_t<>          onOpen;
    event_t<>          onPipe;
    event_t<string_t>  onData;
    
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
 
   ~file_t() noexcept { if( obj.count()>1 && !is_closed() ){ return; } free(); }

    file_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    bool     is_closed() const noexcept { return is_state(STATE::FS_STATE_DISABLE) || is_feof() || obj->fd==INVALID_HANDLE_VALUE; }
    bool       is_feof() const noexcept { return obj->feof <= 0 && obj->feof != -2; }
    bool    is_waiting() const noexcept { return obj->feof == -2; }
    bool  is_available() const noexcept { return !is_closed(); }


    /*─······································································─*/

    void close() const noexcept {
        if( is_state ( STATE::FS_STATE_DISABLE ) ){ return; }
            set_state( STATE::FS_STATE_CLOSE   );
    onDrain.emit(); free(); }

    /*─······································································─*/

    void  resume() const noexcept { if(is_state(STATE::FS_STATE_OPEN )){ return; } set_state(STATE::FS_STATE_OPEN ); onResume.emit(); }
    void    stop() const noexcept { if(is_state(STATE::FS_STATE_REUSE)){ return; } set_state(STATE::FS_STATE_REUSE); onDrain .emit(); }
    void   reset() const noexcept { if(is_state(STATE::FS_STATE_KILL )){ return; } resume(); pos(0); }
    void   flush() const noexcept { obj->buffer.fill(0); }
    
    /*─······································································─*/

    void   set_range( ulong x, ulong y ) const noexcept { obj->range[0] = x; obj->range[1] = y; }
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

    ulong pos( ulong _pos ) const noexcept { obj->offset = _pos; return _pos; }

    ulong size() const noexcept { LARGE_INTEGER size;
          GetFileSizeEx(obj->fd,&size); return size.QuadPart;
    }

    ulong pos() const noexcept { return obj->offset; }
    
    /*─······································································─*/

    ulong set_buffer_size( ulong _size ) const noexcept { 
        obj->buffer = ptr_t<char>( _size ); return _size;
    }
    
    /*─······································································─*/

    void free() const noexcept {

        if( is_state( STATE::FS_STATE_REUSE ) && !is_feof() && obj.count()>1 ){ return; }
        if( is_state( STATE::FS_STATE_KILL  ) ) /*-------*/ { return; }
        if(!is_state( STATE::FS_STATE_CLOSE | STATE::FS_STATE_REUSE ) )
          { kill(); onDrain.emit(); } else { kill(); }
       
        onUnpipe.clear(); onResume.clear();
        onError .clear(); onData  .clear();
        onOpen  .clear(); onPipe  .clear(); onClose.emit();

    }
    
    /*─······································································─*/

    char read_char() const noexcept { return read(1)[0]; }

    string_t read_until( string_t ch ) const noexcept {
        while( obj->_until( this, ch ) == 1 )
             { process::next(); }
        return obj->_until.data;
    }

    string_t read_until( char ch ) const noexcept {
        while( obj->_until( this, ch ) == 1 )
             { process::next(); }
        return obj->_until.data;
    }

    string_t read_line() const noexcept {
        while( obj->_line( this ) == 1 )
             { process::next(); }
        return obj->_line.data;
    }

    /*─······································································─*/

    string_t read( ulong size=CHUNK_SIZE ) const noexcept {
        while( obj->_read( this, size ) == 1 )
             { process::next(); }
        return obj->_read.data;
    }

    ulong write( const string_t& msg ) const noexcept {
        while( obj->_write( this, msg ) == 1 )
             { process::next(); }
        return obj->_write.data;
    }
    
    /*─······································································─*/

    virtual int _read ( char* bf, const ulong& sx ) const noexcept { return __read ( bf, sx ); }
    virtual int _write( char* bf, const ulong& sx ) const noexcept { return __write( bf, sx ); }
    
    /*─······································································─*/

    virtual int __read( char* bf, const ulong& sx ) const noexcept {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; } DWORD c=0;

        if( obj->state & STATE::FS_STATE_READING ){ 
        if( is_blocked( false, c ) ){ return -2; }
            obj->state&=~ STATE::FS_STATE_READING; 
            obj->feof = (int)c; return obj->feof; 
        }

        memset( &obj->ovr, 0, sizeof(OVERLAPPED) );
        obj->state|= STATE::FS_STATE_READING;
        obj->ovr.Offset = obj->offset;
        
        if( ReadFile( obj->fd, bf, sx, &c, &obj->ovr ) ){
            obj->feof = c==0 ? -1 : (int) c; obj->offset+= c;
            obj->state&=~ STATE::FS_STATE_READING;
        } elif( is_blocked( c ) ) { obj->feof = -2; return -2; }

    return ( obj->feof <= 0 && obj->feof != -2 ) ? -1 : obj->feof; }

    virtual int __write( char* bf, const ulong& sx ) const noexcept {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; } DWORD c=0;

        if( obj->state & STATE::FS_STATE_WRITING ){
        if( is_blocked( true, c ) ){ return -2; }
            obj->state&=~ STATE::FS_STATE_WRITING; 
            obj->feof = (int)c; return obj->feof;  
        }

        memset( &obj->ovw, 0, sizeof(OVERLAPPED) );
        obj->state|= STATE::FS_STATE_WRITING;
        obj->ovw.Offset = obj->offset;
        
        if( WriteFile( obj->fd, bf, sx, &c, &obj->ovw ) ){
            obj->feof = c==0 ? -1 : (int) c; obj->offset+= c;
            obj->state&=~ STATE::FS_STATE_WRITING;
        } elif( is_blocked( c ) ) { obj->feof = -2; return -2; }

    return ( obj->feof <= 0 && obj->feof != -2 ) ? -1 : obj->feof; }

    /*─······································································─*/

    int _write_( char* bf, const ulong& sx, ulong* sy ) const noexcept {
        if( sx==0 || is_closed() ){ return -1; } while( *sy<sx ) {
            int c = __write( bf + *sy, sx - *sy );
            if( c <= 0 && c != -2 ) /*----*/ { return -2; }
            if( c >  0 ){ *sy+= c; continue; } break/**/;
        }   return sx;
    }

    int _read_( char* bf, const ulong& sx, ulong* sy ) const noexcept {
        if( sx==0 || is_closed() ){ return -1; } while( *sy<sx ) {
            int c = __read( bf + *sy, sx - *sy );
            if( c <= 0 && c != -2 ) /*----*/ { return -2; }
            if( c >  0 ){ *sy+= c; continue; } break/**/;
        }   return sx;
    }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/