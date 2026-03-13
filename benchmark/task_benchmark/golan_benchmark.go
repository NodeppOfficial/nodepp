package main

import (
    "fmt"
    "time"
)

func main() {
    for i := 0; i < 100000; i++ {
        go func(id int) {
            ticker := time.NewTicker(10 * time.Millisecond)
            for range ticker.C {
                fmt.Println("hello world!", id)
            }
        }(i)
    }
    select {} // Keep alive
}