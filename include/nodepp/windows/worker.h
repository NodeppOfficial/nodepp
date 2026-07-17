/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_WORKER
#define NODEPP_WINDOWS_WORKER

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class worker_t { 
private:

    mutex_t    & get_mutex  () const noexcept { static mutex_t     out; return out; }
    invoker_t<>& get_invoker() const noexcept { static invoker_t<> out; return out; }

    enum STATE {
         WK_STATE_UNKNOWN = 0b00000000,
         WK_STATE_OPEN    = 0b00000001,
         WK_STATE_CLOSE   = 0b00000010,
         WK_STATE_KILL    = 0b10000000
    };

protected:

    struct NODE {
        int state; DWORD id; function_t<int> cb;
        uchar_64 addr; void* krn= nullptr;
        HANDLE fd;
    };  atomic_ptr_t<NODE> obj;

    static DWORD WINAPI callback( LPVOID arg ){
        auto self = type::cast<worker_t>(arg);
        self->obj->krn = &process::kernel();

        while( !self->is_closed( ) ){
        if   ( self->obj->cb()==-1 ){ break; }
            auto info = coroutine::getno();
            auto time = info.delay;
        process::delay( time==0 ? 1 :time ); }

    self->free(); return 0; }

    int emit() const noexcept { return get_mutex().emit([&](){
        
        auto krn = type::bind( process::NODEPP_EVLOOP() );
        auto self= type::bind( this );
    
        obj->state=STATE::WK_STATE_OPEN;
        obj->addr = get_invoker().add([=](){
            self->obj->state = STATE::WK_STATE_CLOSE | STATE::WK_STATE_KILL;
            self->obj->krn   = nullptr; CloseHandle( self->obj->fd );
        krn->wake(); return -1; });
    
        obj->fd= CreateThread( NULL,0, &callback, (void*) &self, 0, &obj->id );
        if ( obj->fd == NULL ){ return -1; } 
        
        WaitForSingleObject( obj->fd, 0 );

    return 1; }); }

public:

    template< class T, class... V >
    worker_t( T cb, const V&... arg ) noexcept : obj( new NODE() ){
        auto clb = function_t<int,V...>(cb);
        obj->cb  = function_t<int>([=](){ return clb(arg...); });
    if( emit()==-1 ){ free(); }; }
    
    /*─······································································─*/

   ~worker_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    worker_t() noexcept : obj( new NODE ) {}

    void free() const noexcept {
        if( obj->state == 0x00 ) /*---------*/ { return; }
        if( obj->state & STATE::WK_STATE_KILL ){ return; }
    get_mutex().lock([&](){ get_invoker().emit (obj->addr); }); }
    
    /*─······································································─*/

    expected_t<kernel_t,except_t> kernel() const noexcept {
        if( obj->krn==nullptr ){ return except_t( "kernel not found" ); }
        return * ( type::cast<kernel_t>( obj->krn ) );
    }

    bool is_sleeping() const noexcept {
        if( obj->krn==nullptr ){ return false; }
        return type::cast<kernel_t>( obj->krn )->is_sleeping();
    }

    void wake() const noexcept { 
        if( obj->krn==nullptr ){ return; }
        type::cast<kernel_t>( obj->krn )->wake();
    }
    
    /*─······································································─*/

    void   off() const noexcept { NODEPP_LOCAL_SHTDWN() = true; }
    void close() const noexcept { NODEPP_LOCAL_SHTDWN() = true; }
    
    /*─······································································─*/

    HANDLE get_fd() const noexcept { return obj->fd; }
    
    /*─······································································─*/

    bool is_closed() const noexcept { auto &x = obj->state;
        return ( x & STATE::WK_STATE_KILL  ) ||
               ( x & STATE::WK_STATE_CLOSE ) ||
                 x== STATE::WK_STATE_UNKNOWN ||
        NODEPP_LOCAL_SHTDWN();
    }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/