const { WebSocketServer } = require('ws');

// Initialize the server on port 8080
const wss = new WebSocketServer({ port: 8000 });

wss.on('connection', (ws) => {
  console.log('New client connected!');

  // Handle incoming messages from clients
  ws.on('message', (message) => {
    console.log(`Received message: ${message}`);
    
    // Echo the message back to the client
    ws.send(`Server received: ${message}`);
  });

  // Handle client disconnection
  ws.on('close', () => {
    console.log('Client has disconnected');
  });
});

console.log('WebSocket server is running on ws://localhost:8000');