/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_LOOP
#define NODEPP_LOOP

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class loop_t {
private:

    using NODE_CLB = function_t<int>;
    using NODE_TASK= type::pair<ulong,void*>;
    using NODE_PAIR= type::pair<NODE_CLB,ref_t<task_t>>;

protected:

    struct NODE {
        queue_t<NODE_PAIR> queue;
        queue_t<void*>     normal;
        queue_t<NODE_TASK> blocked;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    void* get_nearest_timeout( ulong time ) const noexcept {
    
        auto x = obj->blocked.last(); while( x!=nullptr ){
        if( time>=x->data.first ){ return x->next; }
        x = x->prev; }

    return obj->blocked.first(); }

    /*─······································································─*/

    inline int blocked_queue_next() const {
        auto stamp = process::now();
        
        do{ auto x = obj->blocked.first(); 
        if( x == nullptr ) /*---*/ { return -1; }
        if( x->data.first > stamp ){ return -1; }

        if( x->data.first < stamp ){
            obj->normal .push ( x->data.second ); 
            obj->blocked.erase( x );
        } else { break; }} while(1); return 1;

    }

    /*─······································································─*/

    inline int normal_queue_next() const {
    
        if( obj->normal.empty() ) /*-*/ { return -1; } do {
        if( obj->normal.get()==nullptr ){ return -1; }

        auto x = obj->normal.get(); auto y = obj->queue.as(x->data);
        auto o = obj->normal.get() == obj->normal.last() ? -1 : 1;
        
        if( y->data.second->flag & TASK_STATE::USED   ){ 
            obj->normal.next(); 
        return 0; }
        
        if( y->data.second->flag & TASK_STATE::CLOSED ){ 
            obj->normal.erase(x);
            obj->queue .erase(y); 
        return 1; } 

        y->data.second->flag |= TASK_STATE::USED;

        int c=0; ulong d=0; while( ([&](){
            
            do{ c=y->data.first(); auto z=coroutine::getno();
            if( c==1 && z.flag&coroutine::STATE::CO_STATE_DELAY )
              { d=z.delay; goto GOT3; } switch(c) {
                case  1 :  goto GOT1;   break;
                case -1 :  goto GOT2;   break;
                case  0 :  goto GOT4;   break;
            } } while(0);

            GOT1:;

                y->data.second->flag &=~ TASK_STATE::USED; 
                obj->normal.next(); return -1;

            GOT2:;

                y->data.second->flag = TASK_STATE::CLOSED;
                /*---------------*/ return -1;

            GOT3:;

            do {

                y->data.second->flag &=~ TASK_STATE::USED;  
                ulong wake_time = d + process::now();

                auto z = obj->blocked.as( get_nearest_timeout( wake_time ) );
                obj->blocked.insert( z, NODE_TASK( { wake_time, y } ));
                obj->normal .erase(x); 

            return -1; } while(0);

            GOT4:;

        return -1; })() >= 0 ){ /* unused */ }
        return  o; } while(0); return -1;

    }

public: loop_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void off( ptr_t<task_t> address ) const noexcept { clear( address ); }

    void clear( ptr_t<task_t> address ) const noexcept {
        if( address.null() ) /*-*/ { return; }
        if( address->sign != &obj ){ return; }
        if( address->flag & TASK_STATE::CLOSED ){ return; }
            address->flag = TASK_STATE::CLOSED;
    }

    /*─······································································─*/

    int get_delay() const noexcept { 
        if(!obj->normal .empty() ){ return  0; }
        if( obj->blocked.empty() ){ return -1; }

        auto stm = obj->blocked.first()->data.first;
        auto now = process::now();

        return ( stm>now ) ? ( stm-now ) : 0;
    }

    /*─······································································─*/

    ulong size() const noexcept { return obj->queue.size  (); }

    bool empty() const noexcept { return obj->queue.empty (); }

    /*─······································································─*/

    inline int next() const /*--*/ { 
        /*--*/ blocked_queue_next();
        return normal_queue_next ();
    }

    void clear() const noexcept { 
        obj->queue  .clear(); 
        obj->normal .clear(); 
        obj->blocked.clear(); 
    }

    /*─······································································─*/

    template< class T, class... V >
    ptr_t<task_t> add( T cb, const V&... args ) const noexcept {
    ptr_t<task_t> tsk( 0UL, task_t() ); auto clb = type::bind( cb );

        obj->queue .push({[=](){ return (*clb)( args... );}, tsk });
        obj->normal.push( obj->queue.last() ); 

        tsk->addr = obj->queue.last();
        tsk->flag = TASK_STATE::OPEN ;
        tsk->sign = &obj;

    return tsk; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/