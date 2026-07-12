// server.mjs
import { createServer } from 'node:http';

const server = createServer((req, res) => {
  res.writeHead(200, { 'Content-Type': 'text/plain' });
  res.end('Hello World!\n');
});

// starts a simple http server locally on port 8000
server.listen( 8000, 'localhost', () => {
  console.log('Listening on http://localhost:8000');
});

// run with `node server.mjs`