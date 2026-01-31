package main

import (
	"fmt"
	"time"
	"runtime"
)

func benchmarkGo(iterations int) int64 {
    start := time.Now()
    for i := 0; i < iterations; i++ {
		// Allocate 128 bytes on the Heap
        churn := make([]byte, 128)
        churn[0] = byte(i % 255) // avoiding optimization
        _ = churn
    }
    // FORCE GO TO DO THE WORK NODEPP DOES AUTOMATICALLY
    runtime.GC() 
    return time.Since(start).Milliseconds()
}

func main() {

	for x := 0; x <= 1000; x++ {
		d := benchmarkGo( 100000 )
		fmt.Printf( "%d Go Time: %d ms\n", x, d )
	}

}