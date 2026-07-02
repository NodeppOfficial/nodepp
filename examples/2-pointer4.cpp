#include <nodepp/nodepp.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    class my_object { 
    protected: 
        
        struct NODE {
           int value = 10;
           /* other variables here */
          ~NODE(){ /*free memory if needed*/ }
        }; ptr_t<NODE> obj;

    public:
       
        my_object() : obj( new NODE() ){ console::log( "constructor" ); }

       ~my_object() { 
        if( obj.count()>1 ){ return; } 
            console::log( "destructor" ); 
        }

        int& get_value() const noexcept { return obj->value; }

        void set_value( int val ) const noexcept {
            obj->value = val;
        }

    };

    my_object obj; // <- no ptr to be safe, because its already a ptr

    timer::add([=](){
        console::log( "done", obj.get_value() );
        return obj.get_value()-->0 ? 1 : -1; 
    }, 1000 );

}