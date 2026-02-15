/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TEST
#define NODEPP_TEST

/*────────────────────────────────────────────────────────────────────────────*/

#include "event.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class test_t { 
protected:

    struct NODE {
        function_t<int> callback;
        string_t        name;
    };  

    enum STATE {
         TS_STATE_UNKNOWN = 0b00000000,
         TS_STATE_OPEN    = 0b00000001,
         TS_STATE_SKIP    = 0b00000010,
         TS_STATE_CLOSED  = 0b00000100,
    };
    
    struct DONE {
        queue_t<NODE> queue; int state = 0x01; 
    };  ptr_t<DONE> obj;

public:

    event_t<> onClose;
    event_t<> onFail;
    event_t<> onDone;
    event_t<> onSkip;

    /*─······································································─*/

   ~test_t() noexcept {
        if( obj.count()> 1 ) /*---------------*/ { return; }
        if( obj->state & STATE::TS_STATE_CLOSED ){ return; }
        obj->state = STATE::TS_STATE_CLOSED; onClose.emit(); 
    }

    test_t() noexcept : obj( new DONE() ) { 
        auto self = type::bind( this );
    }

    /*─······································································─*/

    template< class T >
    void set( const string_t& name, const T& callback ) noexcept {
        NODE node; 
             node.callback = callback;
             node.name     = name;
        obj->queue.push( node );
    }

    /*─······································································─*/

    void   ignore() const noexcept { obj->state |= STATE::TS_STATE_SKIP; }

    void unignore() const noexcept { obj->state &=~STATE::TS_STATE_SKIP; }
    
    /*─······································································─*/

    void await() const noexcept { auto self=type::bind(this);
        if( obj->state & STATE::TS_STATE_CLOSED ){ return; }

        process::await( coroutine::add( COROUTINE(){ int c=0;
        coBegin; 
            self->obj->queue.set( self->obj->queue.first() );
        coYield(1);

            if( self->obj->state & STATE::TS_STATE_SKIP ){ coEnd; } do {
                auto x = self->obj->queue.get();
            if( x==nullptr ){ break; }

            conio::log("TEST:> "); conio::log( x->data.name );
            c = x->data.callback(); if ( c == 1 ){
                conio::done( " PASSED\n" ); 
                self->onDone.emit();
            } elif ( c == -1 ) {
                conio::error( " FAILED\n" ); 
                self->onFail.emit();
            } else {
                conio::warn( " SKIPPED\n" ); 
                self->onSkip.emit();
            }

            } while(0);

            if( self->obj->queue.get()==nullptr )/*--*/{ self->onClose.emit(); coEnd; } 
            if( self->obj->queue.get()->next==nullptr ){ self->onClose.emit(); coEnd; } 
                self->obj->queue.next();
              
        coGoto(1) ; coFinish
        }));

    }
    
    /*─······································································─*/

    void run() const noexcept { auto self=type::bind( this );
        if( obj->state & STATE::TS_STATE_CLOSED ){ return; }

        process::add( coroutine::add( COROUTINE(){ int c=0;
        coBegin; 
            self->obj->queue.set( self->obj->queue.first() ); 
        coYield(1);

            if( self->obj->state & STATE::TS_STATE_SKIP ){ coEnd; } do {
                auto x = self->obj->queue.get();
            if( x==nullptr ){ break; }

            conio::log("TEST:> "); conio::log( x->data.name );
            c = x->data.callback(); if ( c == 1 ){
                conio::done( " PASSED\n" ); 
                self->onDone.emit();
            } elif ( c == -1 ) {
                conio::error( " FAILED\n" ); 
                self->onFail.emit();
            } else {
                conio::warn( " SKIPPED\n" ); 
                self->onSkip.emit();
            }

            } while(0);

            if( self->obj->queue.get()==nullptr )/*--*/{ self->onClose.emit(); coEnd; } 
            if( self->obj->queue.get()->next==nullptr ){ self->onClose.emit(); coEnd; } 
                self->obj->queue.next();
              
        coGoto(1) ; coFinish
        }));

    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#define TEST_ADD( TEST, NAME, ... ) TEST.set( NAME, __VA_ARGS__ )

#define TEST_ASSERT ( CONDITION ) if( !CONDITION ) TEST_FAIL()

#define TEST_END( TEST ) TEST_IGNORE( TEST ); return 0

/*---------------------------------------------------------------------------*/

#define TEST_UNIGNORE( TEST ) TEST.unignore()

#define TEST_IGNORE( TEST ) TEST.ignore()

/*---------------------------------------------------------------------------*/

#define TEST_AWAIT( TEST ) TEST.await()

#define TEST_CREATE() nodepp::test_t()

#define TEST_RUN( TEST ) TEST.run()

/*---------------------------------------------------------------------------*/

#define TEST_SKIP() return  0

#define TEST_FAIL() return -1

#define TEST_DONE() return  1 

/*────────────────────────────────────────────────────────────────────────────*/

#endif