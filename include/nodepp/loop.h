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

namespace nodepp { class loop_t : public generator_t {
private:

    using NODE_CLB = function_t<int>;
    struct waiter { bool blk; bool out; };
    using NODE_PAIR= type::pair<NODE_CLB,ref_t<waiter>>;

protected:

    struct NODE {
        ulong min_stamp=0, dif=0;
        queue_t<NODE_PAIR> /*---------*/ queue;
        queue_t<void*> /*-------------*/ normal;
        queue_t<type::pair<ulong,void*>> blocked;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    inline int blocked_queue_next() const noexcept {
    
        if( obj->blocked.empty() ) /*---*/ { return -1; } do {
        if( obj->blocked.get() == nullptr ){ return -1; }
        if( obj->min_stamp>process::now() ){ return -1; }

        auto x = obj->blocked.get();

        if( x->data.first < process::now() ){
            obj->normal .push ( x->data.second ); 
            obj->blocked.erase( x );
        } else {
            obj->blocked.next();
        }

        } while(0); return 1;

    }

    /*─······································································─*/

    inline int normal_queue_next() const noexcept {
    
        if( obj->normal.empty() ) /*-*/ { return -1; } do {
        if( obj->normal.get()==nullptr ){ return -1; }

        auto x = obj->normal.get(); auto y = obj->queue.as(x->data);
        auto o = obj->normal.get() == obj->normal.last();
        
        if( y->data.second->blk==1 ){ 
            obj->normal.next(); 
        return o; }
        
        if( y->data.second->out==0 ){ 
            obj->queue .erase(y); 
            obj->normal.erase(x);
        return o; } 

        y->data.second->blk =1;

        int c=0, _state_=0; ulong d=0, _time_=0; while( ([&](){
            
            do{ c=y->data.first(); auto z=coroutine::getno();
            if( c==1 && z.flag&coroutine::STATE::CO_STATE_DELAY )
              { d=z.delay; goto GOT3; } switch(c) {
                case  1 :  goto GOT1;   break;
                case -1 :  goto GOT2;   break;
                case  0 :  goto GOT4;   break;
            } } while(0);

            GOT1:;

                obj->normal.next(); 
                obj->dif            = 0;
                y->data.second->blk = 0; return -1;

            GOT2:;

                y->data.second->out = 0;
                y->data.second->blk = 0; return -1;

            GOT3:;

            do {

                ulong wake_time = d + process::now();
                y->data.second->blk = 0;
                
                obj->blocked.push({ wake_time, y });
                obj->normal .erase(x); 

                if( obj->min_stamp>wake_time||  
                    obj->blocked.empty()
                ) { 
                    obj->min_stamp=wake_time; 
                    obj->dif      =d;
                }

            } while(0);

            GOT4:;

        return -1; })() >= 0 ){ /* unused */ }

        return o ? -1 : 1; } while(0); return -1;

    }

public:

    virtual ~loop_t() noexcept { /*-----*/ }

    loop_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void     clear() const noexcept { /*--*/ obj->queue.clear(); obj->normal.clear(); obj->blocked.clear(); }

    int  get_delay() const noexcept { return empty()?-1:type::cast<int>(obj->dif); }

    bool     empty() const noexcept { return obj->queue.empty(); }

    /*─······································································─*/

    ulong blocked_size() const noexcept { return obj->blocked.size(); }

    ulong running_size() const noexcept { return obj->normal.size(); }

    ulong         size() const noexcept { return obj->queue.size (); }

    /*─······································································─*/

    inline int next() const noexcept { 
        /*--*/ blocked_queue_next();
        return normal_queue_next ();
    }

    /*─······································································─*/

    template< class T, class... V >
    inline void* add( T cb, const V&... args ) const noexcept {
        ptr_t<waiter> tsk = new waiter(); tsk->blk=0; tsk->out=1; 
        auto clb = type::bind( cb );
        obj->queue .push({ [=](){ return (*clb)( args... ); }, tsk });
        obj->normal.push( obj->queue.last() ); return (void*)&tsk->out;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
