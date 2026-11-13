# Nodepp: An Asynchronous Runtime for C++

[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20Mac-blue)](https://github.com/NodeppOfficial/nodepp)
[![Build Status](https://github.com/NodeppOfficial/nodepp/actions/workflows/main.yml/badge.svg)](https://github.com/NodeppOfficial/nodepp/actions)
[![Valgrind Memory Test](https://img.shields.io/badge/memory-zero_leaks-green)](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/valgrind_benchmark/readme.md)
[![XERJ Corpus Hub contributor](https://hub.xerj.org/contributors/EDBCREPO/badge.svg)](https://hub.xerj.org/contributors/EDBCREPO)

Nodepp is an asynchronous C++ runtime that unifies application development under a single syntax, a single codebase, and a single execution model. It starts from a simple premise: you don't need a different language for every layer of your application. The same code that runs on a Linux server can run in a browser via WASM, on an ESP32, or on an Arduino Nano — without changes.

The inspiration doesn't come from a modern framework, but from Doom. Doom wasn't revolutionary for its graphics, but for its portability: it ran on hardware nobody imagined because its design didn't depend on a specific platform. That same philosophy guided Nodepp — a runtime so universal it runs on anything with a C++11 compiler, from 2KB of RAM to 64 cores. It's not a framework tied to an ecosystem. It's a runtime that adapts to the hardware, not the other way around.

Most runtimes grow until they can't be understood. Nodepp went the other direction. ~3,800 lines for the entire core. 2KB for the binary. 0% CPU when idle. No STL, no exceptions, no RTTI — not because they're bad, but because they aren't necessary. The result is a runtime small enough to fit on any controller and simple enough to audit completely. Nothing to trust. Nothing to hide.

## Featured Projects

### **[Enigma Machine - Arduino Nano](https://wokwi.com/projects/449104127751150593)**: A fully functional Enigma machine running on 2KB of RAM.
https://github.com/user-attachments/assets/9b870671-3854-444f-893d-40fdce31a629

### **[Spacesus.io](https://spacesus.io)**: P2P multiplayer game built on Nodepp, WebRTC, and Raylib.
https://github.com/user-attachments/assets/a8b4d56d-0291-4aa8-98e6-926116215f8f

## Benchmarks

| Metric | Bun (v1.3.5) | Go (v1.18.1) | Nodepp (v1.4.0) |
| --- | --- | --- | --- |
| Req / Second | 5,985.74 | 6,139.41 | 6,851.33 |
| Memory (RSS) | 69.5 MB | 14.1 MB | 2.9 MB |
| Max Latency | 1,452 ms | 326 ms | 245 ms |
| p99 Latency | 1,159 ms | 249 ms | 187 ms |
| CPU Usage | 98.7% | 99.3% | 100.0% |

*Benchmark on Chromebook Apollo Lake, 1000 concurrent connections. [View full benchmarks](https://github.com/NodeppOfficial/nodepp/tree/main/benchmark)*

## Quick Start

```cpp
#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>

using namespace nodepp;

void onParallel() {

    auto server = http::server([]( http_t cli ){
         cli.write_header( 200, header_t({}) );
         cli.write( "hello world" );
    });

    server.listen( "0.0.0.0", 8000 );
    console::log( "http://localhost:8000" );

}

void onMain(){ worker::parallel( &onParallel, os::cpus() ); }
```

## Ecosystem

| Module | Description |
|--------|-------------|
| [ExpressPP](https://github.com/NodeppOfficial/nodepp-express) | Express-style HTTP server |
| [ApifyPP](https://github.com/NodeppOfficial/nodepp-apify) | 64-byte RPC |
| [Nodepp-Redis](https://github.com/NodeppOfficial/nodepp-redis) | Redis client |
| [Nodepp-SQLite](https://github.com/NodeppOfficial/nodepp-sqlite) | SQLite client |
| [Nodepp-Argon2](https://github.com/NodeppOfficial/nodepp-argon2) | Password hashing |

## Platforms

| Platform | status |
|------------|--------|
| Linux | ✅ |
| Windows | ✅ |
| macOS | ✅ |
| WASM (browser) | ✅ |
| ESP32 | ✅ |
| Arduino | ✅ |

## License
MIT. Use it anywhere.
