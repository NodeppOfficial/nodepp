# Nodepp
> **The DOOM of Async Frameworks: Write Once, Build Everywhere, Process Everything.**

[![Build Status](https://github.com/NodeppOfficial/nodepp/actions/workflows/main.yml/badge.svg)](https://github.com/NodeppOfficial/nodepp/actions)
[![Platform](https://img.shields.io/badge/platform-%20Linux%20|%20Windows%20|%20Mac%20|%20Android%20|%20IOS%20-blue)](https://github.com/NodeppOfficial/nodepp)
[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Nodepp is a groundbreaking C++ framework that bridges the gap between the language's raw performance and the developer-friendly abstractions of Node.js. By providing a high-level, asynchronous API, Nodepp empowers you to write C++ with a familiar syntax—enabling seamless development across cloud servers, desktop apps, and microcontrollers.

At its core, Nodepp features a 100% asynchronous architecture powered by an internal Event Loop. This allows for massive concurrency and scalable task management with minimal code complexity, effectively bringing the "Write Once, Run Everywhere" philosophy to the world of high-performance C++.

🔗: [Nodepp The MOST Powerful Framework for Asynchronous Programming in C++](https://medium.com/p/c01b84eee67a)

## 💡 Featured Project: Asynchronous Enigma Machine

To showcase [Nodepp for Arduino](https://github.com/NodeppOfficial/nodepp-arduino) efficiency on "bare metal" hardware, we implemented a fully functional Enigma Machine on an Arduino Nano.

https://github.com/user-attachments/assets/9b870671-3854-444f-893d-40fdce31a629

Try it now: [Enigma Machine Simulation](https://wokwi.com/projects/449104127751150593)

## 💡 Featured Project: Duck Hunt VR (WASM Edition)

To showcase [Nodepp for Web](https://github.com/NodeppOfficial/nodepp-wasm), we ported the classic Duck Hunt to Virtual Reality, running natively in the browser via WebAssembly. This project pushes the limits of web-based VR by combining low-level C++ performance with modern Web APIs.

https://github.com/user-attachments/assets/ab26287e-bd73-4ee8-941b-d97382e203c9

Play it now: [Duck Hunt VR on Itch.io](https://edbcrepo.itch.io/duck-hunt-vr)

## Dependencies & Cmake Integration
```bash
# Openssl
    🪟: pacman -S mingw-w64-ucrt-x86_64-openssl
    🐧: sudo apt install libssl-dev
# Zlib
    🪟: pacman -S mingw-w64-ucrt-x86_64-zlib
    🐧: sudo apt install zlib1g-dev
```
```bash
include(FetchContent)

FetchContent_Declare(
	nodepp
	GIT_REPOSITORY   https://github.com/NodeppOfficial/nodepp
	GIT_TAG          origin/main
	GIT_PROGRESS     ON
)
FetchContent_MakeAvailable(nodepp)

#[...]

target_link_libraries( #[...]
	PUBLIC nodepp #[...]
)
```

## Features

- 📌: **Lightweight:** Minimal dependencies, making it ideal for IoT and embedded systems.
- 📌: **Cross-Platform:** Write once, run on Linux, Windows, Mac, Android, WASM and Arduino/ESP32.
- 📌: **Memory Efficient:** Custom `ptr_t`, `queue_t`, `array_t` and `string_t` primitives provide SSO (Small Stack Optimization) and zero-copy slicing.
- 📌: **Scalability:** Build applications that can handle large workloads and grow with your needs.
- 📌: **Open-source:** Contribute to the project's development and customize it to your specific requirements.

## Projects made with NodePP

- 🔗: [Computer Vision VR Controllers for phones Demo](https://github.com/PocketVR/Barely_VR_AR_Controller_Test)
- 🔗: [Draw on your PC using your smartphone](https://github.com/ScreenDraw/PCDraw)
- 🔗: [Simple multiplayer Game With Raylib](https://medium.com/@EDBCBlog/create-your-own-online-multiplayer-small-fast-and-fun-with-raylib-nodepp-and-websockets-190f5c174094)
- 🔗: [Cursed Luna - A simple Raylib Game](https://github.com/EDBCREPO/Space-Shocker)
- 🔗: [Smart Card Reader(Nodepp-Arduino)](https://github.com/EDBCREPO/emv-reader)
- 🔗: [Serial Port arduino using Nodepp](https://github.com/EDBCREPO/Arduino_PC)
- 🔗: [Simple Raylib Real-Time Chat](https://github.com/EDBCREPO/simple-raylib-websocket-chat)
- 🔗: [Simple Bitget Trading Bot](https://github.com/EDBCREPO/simple-binance-bot-nodepp)

Check out some articles on [Medium](https://medium.com/@EDBCBlog)

## Batteries Included

- 📌: UTF Support: Comprehensive manipulation for UTF8, UTF16, and UTF32.
- 📌: Networking: Native support for TCP, TLS, UDP, HTTP, and WebSockets.
- 📌: Built-in JSON & RegExp: Full parsing and text processing engines.
- 📌: I/O Multiplexing: Support for Poll, Epoll, Kqueue, and WSAPoll.
- 📌: Reactive Programming: Built-in Events and Observers system.

## Quick Start

### Clone The Repository
```bash
#!/usr/bin/env bash
git clone https://github.com/NodeppOfficial/nodepp ; cd nodepp
```

### Create a main.cpp File
```bash
#!/usr/bin/env bash
touch main.cpp
```
```cpp
#include <nodepp/nodepp.h>
#include <nodepp/regex.h>
#include <nodepp/http.h>
#include <nodepp/date.h>

using namespace nodepp;

void onMain(){

    auto server = http::server([=]( http_t cli ){

        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }));

        cli.write( regex::format( R"(
            <h1> Hello World </h1>
            <h2> ${0} </h2>
        )", date::fulltime() ));

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}
```

### Build Your Code
```bash
#!/usr/bin/env bash
🐧: g++ -o main main.cpp -O3 -I ./include          ; ./main #(Linux)
🪟: g++ -o main main.cpp -O3 -I ./include -lws2_32 ; ./main #(Windows)
```

## Nodepp Supports Other Platforms Too
- 🔗: [NodePP for Window | Linux | Mac | Bsd ](https://github.com/NodeppOfficial/nodepp)
- 🔗: [NodePP for Arduino](https://github.com/NodeppOfficial/nodepp-arduino)
- 🔗: [Nodepp for WASM](https://github.com/NodeppOfficial/nodepp-wasm)

## Ecosystem
- 🔗: [ExpressPP](https://github.com/NodeppOfficial/nodepp-express)   -> Express equivalent for Nodepp
- 🔗: [ApifyPP](https://github.com/NodeppOfficial/nodepp-apify)       -> Socket.io equivalent for Nodepp
- 🔗: [Bluetooth](https://github.com/NodeppOfficial/nodepp-bluetooth) -> Bluetooth Port for Nodepp
- 🔗: [SerialPP](https://github.com/NodeppOfficial/nodepp-serial)     -> Serial Port for Nodepp
- 🔗: [Argon2](https://github.com/NodeppOfficial/nodepp-argon2)       -> Argon2 for Nodepp
- 🔗: [Torify](https://github.com/NodeppOfficial/nodepp-torify)       -> HTTP|Ws over Tor
- 🔗: [GPUPP](https://github.com/NodeppOfficial/nodepp-gpu)           -> GPGPU for Nodepp
- 🔗: [NginxPP](https://github.com/NodeppOfficial/nodepp-nginx)       -> Reverse Proxy
- 🔗: [InputPP](https://github.com/NodeppOfficial/nodepp-input)       -> Fake Inputs
- 🔗: [XML](https://github.com/NodeppOfficial/nodepp-xml)             -> XML for Nodepp
- 🔗: [JWT](https://github.com/NodeppOfficial/nodepp-jwt)             -> JSON Web Token
- 🔗: [NmapPP](https://github.com/NodeppOfficial/nodepp-nmap)         -> Scan IPs and Ports
- 🔗: [Redis](https://github.com/NodeppOfficial/nodepp-redis)         -> Redis Client for Nodepp
- 🔗: [Sqlite](https://github.com/NodeppOfficial/nodepp-sqlite)       -> Sqlite Client for Nodepp
- 🔗: [MariaDB](https://github.com/NodeppOfficial/nodepp-mariadb)     -> MariaDB Client for Nodepp
- 🔗: [Postgres](https://github.com/NodeppOfficial/nodepp-postgres)   -> Postgres Client for Nodepp

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
