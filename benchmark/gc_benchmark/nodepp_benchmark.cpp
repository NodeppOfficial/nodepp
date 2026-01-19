#include <nodepp/nodepp.h>
#include <nodepp/ptr.h>

using namespace nodepp;

ulong benchmark_nodepp( int iterations ) {

    auto start = process::micros();

    for( int i = 0; i < iterations; i++ ) {
		// Allocate 128 bytes on the Heap
         ptr_t<char> churn( 128UL ); 
         churn[0] = (char)(i % 255); // avoiding optimization
    }

    auto end = process::micros();
    return ( end - start ) / 1000UL;

}

void onMain() {

    for( int x=0; x <= 1000; x++ ){
        ulong d = benchmark_nodepp( 100000 );
        console::log( x, "Nodepp Time:", d, "ms" );
    }

}