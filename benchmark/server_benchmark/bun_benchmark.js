// BUN CODE USED AS BENCHMARK

// Import the 'http' module, which is a built-in Node.js module for creating HTTP servers.
const http = require('http');

// Define the hostname and port number where the server will listen.
const hostname = 'localhost';
const port     = 8000;

// Create an HTTP server instance.
// The callback function takes two arguments: 'req' (request) and 'res' (response).
const server = http.createServer((req, res) => {
    // Set the HTTP status code to 200 (OK) and the 'Content-Type' header to 'text/html'.
    // This tells the browser to expect an HTML response.
    res.writeHead(200, { 'Content-Type': 'text/html' });

    // Send the response body.
    // The 'res.end()' method signals that all of the response headers and body have been sent.
    res.end('<h1>Hello, World!</h1>');
});

// Start the server and make it listen for incoming requests on the specified hostname and port.
// The callback function runs once the server is successfully listening.
server.listen(port, hostname, () => {
    console.log(`Server is listening on http://${hostname}:${port}`);
});