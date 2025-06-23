# Nodepp

Nodepp is a groundbreaking open-source project that simplifies C++ application development by bridging the gap between the language's raw power and the developer-friendly abstractions of Node.js. By providing a high-level API, Nodepp empowers developers to write C++ code in a familiar, Node.js-inspired style.

One of the standout features of Nodepp is its 100% asynchronous architecture, powered by an internal Event Loop. This design efficiently manages Nodepp’s tasks, enabling you to develop scalable and concurrent applications with minimal code. Experience the power and flexibility of Nodepp as you streamline your development process and create robust applications effortlessly!

## Dependencies
```bash
# Openssl
    🪟: pacman -S mingw-w64-ucrt-x86_64-openssl
    🐧: sudo apt install libssl-dev

# Zlib
    🪟: pacman -S mingw-w64-ucrt-x86_64-zlib
    🐧: sudo apt install zlib1g-dev
```

## Features

- 📌: **Node.js-like API:** Write C++ code in a syntax and structure similar to Node.js, making it easier to learn and use.
- 📌: **High-performance:** Leverage the speed and efficiency of C++ for demanding applications.
- 📌: **Scalability:** Build applications that can handle large workloads and grow with your needs.
- 📌: **Open-source:** Contribute to the project's development and customize it to your specific requirements.

## Bateries Included

- 📌: Include a **build-in JSON** parser / stringify system.
- 📌: Include a **build-in RegExp** engine for processing text strings.
- 📌: Include a **build-in System** that make every object **Async Task** safety.
- 📌: Include a **Smart Pointer** base **Garbage Collector** to avoid **Memory Leaks**.
- 📌: Include support for **Reactive Programming** based on **Events** and **Observers**.
- 📌: Include an **Event Loop** that can handle multiple events and tasks on a single thread.
- 📌: Include support for **TCP | TLS | UDP | HTTP | WS** making it easy to create networked applications.
- 📌: Include Support for **Poll | Epoll | Kqueue | WSAPoll** making it easy to handle multiple file descriptors.

## Build & Run
```bash
🐧: g++ -o main main.cpp -I ./include ; ./main
🪟: g++ -o main main.cpp -I ./include -lws2_32 ; ./main
```

## Examples
### Hello world
```cpp
#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain() { 
    console::log("Hello World!");
}

// note that we are using onMain() instead of main()
```

### HTTP Client
```cpp
#include <nodepp/nodepp.h>
#include <nodepp/http.h>

using namespace nodepp;

void onMain(){

    fetch_t args;
            args.method = "GET";
            args.url = "http://www.google.com/";
            args.headers = header_t({
                { "Host", url::host(args.url) }
            });

    http::fetch( args )

    .then([]( http_t cli ){
        console::log( stream::await( cli ) );
    })

    .fail([]( except_t err ){
        console::error( err );
    });

}
```

### HTTP Server
```cpp
#include <nodepp/nodepp.h>
#include <nodepp/http.h>
#include <nodepp/date.h>

using namespace nodepp;

void onMain(){

    auto server = http::server([=]( http_t cli ){ 

        console::log( cli.path, cli.get_fd() );
        
        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }));
        
        cli.write( date::fulltime() );
        cli.close(); // optional | GC automaticaly close unused sockets

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}
```

### More Examples [here](https://github.com/NodeppOfficial/Nodepp/tree/main/examples)

## Projects made with NodePP
- 🔗: [Draw on your PC using your smartphone](https://github.com/ScreenDraw/PCDraw)
- 🔗: [Simple multiplayer Game With Raylib](https://medium.com/@EDBCBlog/create-your-own-online-multiplayer-small-fast-and-fun-with-raylib-nodepp-and-websockets-190f5c174094)
- 🔗: [Cursed Luna - A simple Raylib Game](https://github.com/EDBCREPO/Space-Shocker)
- 🔗: [Smart Card Reader(Nodepp-Arduino)](https://github.com/EDBCREPO/emv-reader)
- 🔗: [Serial Port arduino using Nodepp](https://github.com/EDBCREPO/Arduino_PC)
- 🔗: [Simple Raylib Real-Time Chat](https://github.com/EDBCREPO/simple-raylib-websocket-chat)
- 🔗: [Simple Bitget Trading Bot](https://github.com/EDBCREPO/simple-binance-bot-nodepp)

Check out some articles on [Medium](https://medium.com/@EDBCBlog)

## Compatibility
- 🔗: [NodePP for Window | Linux | Mac | Bsd ](https://github.com/NodeppOfficial/nodepp)
- 🔗: [NodePP for Arduino](https://github.com/NodeppOfficial/nodepp-arduino)
- 🔗: [Nodepp for ESP32](https://github.com/NodeppOfficial/nodepp-ESPXX)
- 🔗: [Nodepp for WASM](https://github.com/NodeppOfficial/nodepp-wasm)

## Official Libraries for Nodepp
- 🔗: [ExpressPP](https://github.com/NodeppOfficial/nodepp-express) -> Express equivalent for Nodepp
- 🔗: [ApifyPP](https://github.com/NodeppOfficial/nodepp-apify)     -> Socket.io equivalent for Nodepp
- 🔗: [SerialPP](https://github.com/NodeppOfficial/nodepp-serial)   -> Serial Port for Nodepp
- 🔗: [Argon2](https://github.com/NodeppOfficial/nodepp-argon2)     -> Argon2 for Nodepp
- 🔗: [Torify](https://github.com/NodeppOfficial/nodepp-torify)     -> HTTP|Ws over Tor
- 🔗: [NginxPP](https://github.com/NodeppOfficial/nodepp-nginx)     -> Reverse Proxy
- 🔗: [InputPP](https://github.com/NodeppOfficial/nodepp-input)     -> Fake Inputs
- 🔗: [JWT](https://github.com/NodeppOfficial/nodepp-jwt)           -> JSON Web Token
- 🔗: [NmapPP](https://github.com/NodeppOfficial/nodepp-nmap)       -> Scan IPs and Ports
- 🔗: [Redis](https://github.com/NodeppOfficial/nodepp-redis)       -> Redis Client for Nodepp
- 🔗: [Sqlite](https://github.com/NodeppOfficial/nodepp-sqlite)     -> Sqlite Client for Nodepp
- 🔗: [MariaDB](https://github.com/NodeppOfficial/nodepp-mariadb)   -> MariaDB Client for Nodepp
- 🔗: [Postgres](https://github.com/NodeppOfficial/nodepp-postgres) -> Postgres Client for Nodepp
  
## Contribution

If you want to contribute to **Nodepp**, you are welcome to do so! You can contribute in several ways:

- ☕ Buying me a Coffee
- 📢 Reporting bugs and issues
- 📝 Improving the documentation
- 📌 Adding new features or improving existing ones
- 🧪 Writing tests and ensuring compatibility with different platforms
- 🔍 Before submitting a pull request, make sure to read the contribution guidelines.

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/edbc_repo)

## License

**Nodepp** is distributed under the MIT License. See the LICENSE file for more details.
