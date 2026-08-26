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
    }

    bool is_state( uchar_16 value ) const noexcept {
        if( obj->state & value ){ return true; }
    return false; }

    void set_state( uchar_16 value ) const noexcept {
    if( obj->state & STATE::FS_STATE_KILL ){ return; }
        obj->state = value;
    }

    enum STATE : uchar_16 {
         FS_STATE_UNKNOWN = 0b000000000,
         FS_STATE_OPEN    = 0b000000001,
         FS_STATE_REUSE   = 0b001000000,
         FS_STATE_CLOSE   = 0b000000010,
         FS_STATE_READING = 0b000010000,
         FS_STATE_WRITING = 0b000100000,
         FS_STATE_WAITING = 0b010000000,
         FS_STATE_KILL    = 0b000000100,
         FS_STATE_STOP    = 0b000001000,
         FS_STATE_DISABLE = 0b000001110,
         FS_STATE_SERVER  = 0b100000000
    };

protected:

    struct DONE { OVERLAPPED ov; DWORD result; };
    struct NODE {

        HANDLE   fd      = INVALID_HANDLE_VALUE;
        DWORD    offset  = 0UL; DONE ddl[2];
        uchar_64 tag     = 0UL;
        uchar_64 pd      = 0UL;
        len_t    range[2]= { 0, 0 };
        uchar_16 state   = STATE::FS_STATE_OPEN;

        ptr_t<char> buffer; string_t borrow;
        generator::file::until _until;
        generator::file::line  _line ;
        generator::file::read  _read ;
        generator::file::write _write;

       ~NODE(){ do {
        if( fd == INVALID_HANDLE_VALUE ){ return; }

        if( fd == GetStdHandle( STD_INPUT_HANDLE ) ||
            fd == GetStdHandle( STD_OUTPUT_HANDLE) ||
            fd == GetStdHandle( STD_ERROR_HANDLE ) 
        ) { break; } CloseHandle( fd ); } while(0);

        CancelIoEx((HANDLE)fd, &ddl[0].ov);
        CancelIoEx((HANDLE)fd, &ddl[1].ov); }
        
    };  ptr_t<NODE> obj;
    
    /*─······································································─*/

    ptr_t<uint> get_fd_flag( const string_t& flag ){ 
    ptr_t<uint> fg ({ 0x00,FILE_SHARE_READ|FILE_SHARE_WRITE,0x00,FILE_FLAG_OVERLAPPED });
        if  ( flag == "r"  ){ fg[0] |= GENERIC_READ;               fg[2] |= OPEN_EXISTING; }
        elif( flag == "w"  ){ fg[0] |= GENERIC_WRITE;              fg[2] |= CREATE_ALWAYS; }
        elif( flag == "a"  ){ fg[0] |= FILE_APPEND_DATA;           fg[2] |= OPEN_ALWAYS;   }
        elif( flag == "r+" ){ fg[0] |= GENERIC_READ|GENERIC_WRITE; fg[2] |= OPEN_EXISTING; }
        elif( flag == "w+" ){ fg[0] |= GENERIC_READ|GENERIC_WRITE; fg[2] |= OPEN_ALWAYS;   }
        elif( flag == "a+" ){ fg[0] |= FILE_APPEND_DATA;           fg[2] |= OPEN_EXISTING; }
        else /*----------*/ { fg[0] |= GENERIC_READ|GENERIC_WRITE; fg[2] |= OPEN_ALWAYS;   }
    return fg; }
    
    /*─······································································─*/
    
    int set_nonbloking_mode() const noexcept { return 0; }
    
    /*─······································································─*/

    bool is_blocked( DWORD /*unused*/ ) const noexcept {
        DWORD err = GetLastError();
        return( err == ERROR_IO_INCOMPLETE || 
                err == ERROR_IO_PENDING    || 
                err == WAIT_TIMEOUT 
        );
    }

    bool is_blocked( OVERLAPPED& ov, DWORD& c ) const noexcept {
    
        if( !HasOverlappedIoCompleted(&ov) ) { return 1; }

        if( obj->state & ( STATE::FS_STATE_READING | STATE::FS_STATE_WRITING ) ){
        if( GetOverlappedResult((HANDLE)obj->fd, &ov, &c, FALSE) )
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

    file_t( const string_t& path, const string_t& mode, const ulong& _size=NODEPP_CHUNK_SIZE ) : obj( new NODE() ) {
        auto fg = get_fd_flag( mode ); obj->fd = CreateFileA( path.c_str(), fg[0], fg[1], NULL, fg[2], fg[3], NULL ); 
        if( obj->fd == INVALID_HANDLE_VALUE ){ NODEPP_THROW_ERROR("such file or directory does not exist"); }
        set_nonbloking_mode(); set_buffer_size( _size ); 
    }

    file_t( const HANDLE& fd, const ulong& _size=NODEPP_CHUNK_SIZE ) : obj( new NODE() ) {
        if( fd == INVALID_HANDLE_VALUE ){ NODEPP_THROW_ERROR("such file or directory does not exist"); }
        obj->fd = fd; set_nonbloking_mode(); set_buffer_size( _size ); 
    }
 
   ~file_t() noexcept { if( obj.count()>1 && !is_closed() ){ return; } free(); }

    file_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void  resume() const noexcept { if(!is_state(STATE::FS_STATE_STOP )){ return; } onResume .emit(); obj->state &=~ STATE::FS_STATE_STOP; }
    void    stop() const noexcept { if( is_state(STATE::FS_STATE_STOP )){ return; } onDrain  .emit(); obj->state |=  STATE::FS_STATE_STOP; }
    void   reset() const noexcept { if( is_state(STATE::FS_STATE_KILL )){ return; } resume(); pos(0); }
    void   flush() const noexcept { obj->buffer.fill(0); }
    
    /*─······································································─*/

    bool    is_closed() const noexcept { return is_state(STATE::FS_STATE_DISABLE) || obj->fd==INVALID_HANDLE_VALUE; }
    bool  is_reusable() const noexcept { return is_state(STATE::FS_STATE_REUSE  ); }
    bool   is_stopped() const noexcept { return is_state(STATE::FS_STATE_STOP   ); }
    bool   is_waiting() const noexcept { return is_state(STATE::FS_STATE_WAITING); }
    bool is_available() const noexcept { return !is_closed(); }

    /*─······································································─*/

    void close() const noexcept {
        if( is_state ( STATE::FS_STATE_DISABLE )){ return; } onDrain.emit(); 
        if( is_state ( STATE::FS_STATE_REUSE   )){ return; }
            set_state( STATE::FS_STATE_CLOSE   );
    free(); }

    /*─······································································─*/

    void    set_range( len_t x, len_t y ) const noexcept { obj->range[0] = x; obj->range[1] = y; }
    len_t* get_range() /*---------------*/ const noexcept { return obj->range; }

    /*─······································································─*/

    void set_reusable( bool mode ) const noexcept { 
    switch( (int) mode ){
        case 1 : obj->state |=  STATE::FS_STATE_REUSE; break;
        default: obj->state &=~ STATE::FS_STATE_REUSE; break;
    }}

    /*─······································································─*/

    HANDLE    get_fd() const noexcept { return obj->fd; }
    uchar_64& get_pd() const noexcept { return obj->pd; }
    uchar_64&    tag() const noexcept { return obj->tag; }

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

    len_t pos( len_t _pos ) const noexcept { obj->offset = _pos; return _pos; }

    len_t size() const noexcept { 
        len_t out=0; LARGE_INTEGER* size = (LARGE_INTEGER*) &out;
        GetFileSizeEx( obj->fd, size ); return out;
    }

    len_t pos() const noexcept { return obj->offset; }
    
    /*─······································································─*/

    ulong set_buffer_size( ulong _size ) const noexcept { 
        obj->buffer = ptr_t<char>( _size ); return _size;
    }
    
    /*─······································································─*/

    void free() const noexcept {

        if( is_state( STATE::FS_STATE_STOP  ) && obj.count()>1 ){ return; }
        if( is_state( STATE::FS_STATE_KILL  ) ){ return; } kill();
        if(!is_state( STATE::FS_STATE_CLOSE | STATE::FS_STATE_STOP ) )
          { onDrain.emit(); }
          
        onClose .emit (); onDrain .clear();

        onUnpipe.clear(); onResume.clear();
        onError .clear(); onData  .clear();
        onOpen  .clear(); /*-------------*/
        onPipe  .clear(); onClose .clear();

    }
    
    /*─······································································─*/

    char read_char() const noexcept { return read(1)[0]; }

    string_t read_until( string_t ch ) const noexcept {
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

    string_t read( ulong size=NODEPP_CHUNK_SIZE ) const noexcept {
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
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; } 
        
        auto &c  = obj->ddl[0].result;
        auto &ov = obj->ddl[0].ov    ;

        if( obj->state & STATE::FS_STATE_READING ){ 
        if( is_blocked( ov, c ) ){ 
            obj->state |= STATE::FS_STATE_WAITING;
            return -2;
        } else {
            obj->state&=~STATE::FS_STATE_READING;
            obj->state&=~STATE::FS_STATE_WAITING;
            return c==0 ? -1 : (int) c;
        }}

        obj->state|= STATE::FS_STATE_READING;
        ov = {0}; ov.Offset = obj->offset;
        ReadFile( obj->fd, bf, sx, &c, &ov );
        
        if( is_blocked(c) ){
            obj->state |= STATE::FS_STATE_WAITING;
            return -2; 
        } else { if( c >0 ){ obj->offset += c; }
            obj->state&=~STATE::FS_STATE_READING;
            obj->state&=~STATE::FS_STATE_WAITING;
        }

    return c==0 ? -1 : (int) c; }

    virtual int __write( char* bf, const ulong& sx ) const noexcept {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; }
        
        auto &c  = obj->ddl[1].result;
        auto &ov = obj->ddl[1].ov    ;

        if( obj->state & STATE::FS_STATE_WRITING ){
        if( is_blocked( ov, c ) ){ 
            obj->state |= STATE::FS_STATE_WAITING;
            return -2; 
        } else {
            obj->state&=~STATE::FS_STATE_WRITING; 
            obj->state&=~STATE::FS_STATE_WAITING;
            return c==0 ? -1 : (int) c;
        }}

        obj->state|= STATE::FS_STATE_WRITING;
        ov = {0}; ov.Offset = obj->offset;
        WriteFile( obj->fd, bf, sx, &c, &ov );
        
        if( is_blocked(c) ){ 
            obj->state&=~STATE::FS_STATE_WAITING;
            return -2; 
        } else { if( c >0 ){ obj->offset += c; }
            obj->state&=~ STATE::FS_STATE_WRITING;
        }

    return c==0 ? -1 : (int) c; }

    /*─······································································─*/

    int _write_( char* bf, const ulong& sx, ulong* sy ) const noexcept {
    if( sx==0 || is_closed() ){ return -1; } while( *sy<sx ) {
        int c = __write( bf + *sy, sx - *sy );
        if( c==-2 ) /*--*/ { return -2; }
        if( c > 0 ){ *sy+= c; continue; } 
    break; } return *sy; }

    int _read_( char* bf, const ulong& sx, ulong* sy ) const noexcept {
    if( sx==0 || is_closed() ){ return -1; } while( *sy<sx ) {
        int c = __read( bf + *sy, sx - *sy );
        if( c==-2 ) /*--*/ { return -2; }
        if( c > 0 ){ *sy+= c; continue; } 
    break; } return *sy; }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/