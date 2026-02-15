// GO CODE USED AS BENCHMARK
 
package main

import (
 "fmt"
 "log"
 "net/http"
)

func main() {
    // http.HandleFunc registers a handler function for a given pattern.
    // The "/" pattern matches all requests.

    http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
        // w is the http.ResponseWriter, which is used to write the response.
        // r is the *http.Request, which contains information about the request.

        // Set the Content-Type header.
        w.Header().Set("Content-Type", "text/html")

        // Write the response body.
        fmt.Fprintf(w, "<h1>Hello, World!</h1>")
    })

    // Start the server on port 8000.
    // log.Fatal will print the error and exit if the server fails to start.
    fmt.Println("-> Server is listening on http://localhost:8000")
    log.Fatal(http.ListenAndServe(":8000", nil))
}