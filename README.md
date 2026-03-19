# Nodepp: The Unified Asynchronous Real-Time C++ Runtime

[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20WASM-blue)](https://github.com/NodeppOfficial/nodepp)
[![Build Status](https://github.com/NodeppOfficial/nodepp/actions/workflows/main.yml/badge.svg)](https://github.com/NodeppOfficial/nodepp/actions)
[![Valgrind Memory Test](https://img.shields.io/badge/memory-zero_leaks-green)](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/valgrind_benchmark/readme.md)

Nodepp is the end of excuses. For too long, developers have settled for **fragmented glue-ware** or managed runtimes that treat hardware resources like garbage. Nodepp is a vertically-integrated C++ framework that proves you don't need a massive Virtual Machine or a bloated Garbage Collector to write high-level async code.

While others are busy **masturbating the CPU** — burning millions of cycles on garbage collection, context switching, and runtime management — Nodepp focuses on Pure Execution. It provides a unified world architecture where every module shares the same high-efficiency DNA, scaling from an 8-bit Arduino to an Intel XEON cloud server.

```
NODEPP UNIFIED ARCHITECTURE: Co-designed components MODEL
=========================================================

[ APPLICATION LAYER ]   Logic: High-Level Async
          ||
+---------||--------------------------------------------+
|         ||   UNIFIED ptr_t DATA CARRIER               |
|         || (Zero-Copy / Reference Counted)            |
|         \/                                            |
|  [ PROTOCOL LAYER ]   Protocol Layer: HTTP / WS / TLS |
|         ||            Parser: ptr_t Slicing           |
|         ||                                            |
|         \/                                            |
|  [ REACTOR LAYER ]    Reactor Layer: kernel_t         |
|         ||            Engine: Epoll/KQUEUE/IOCP/NPOLL |
+---------||--------------------------------------------+
          ||
          \/            OS Layer: LINUX / WINDOWS / MAC
[ HARDWARE / KERNEL ]   Source: Sockets / Registers
```

## 📃 Whitepaper

[Nodepp: Closing the Gap Between Bare-Metal Performance and Scripting Agility through Silicon-Logic Parity](https://nodeppofficial.github.io/nodepp-doc/whitepaper) Read the full technical breakdown, including architectural deep-dives into `ptr_t`, `kernel_t` and `coroutine_t`.

## ⭐ Architectural Philosophy

- **📌: 1. Deterministic RAII (`ptr_t`):** Eliminates the unpredictable latency spikes (Stop-the-World) of Garbage Collectors. By utilizing Small Stack Optimization (SSO) and reference counting, memory is reclaimed with microsecond precision.

- **📌: 2. Cooperative Multitasking (`coroutine_t`):** Stackless coroutines eliminate context-switching overhead. This allows for massive connection density on low-power hardware, from 8-bit industrial sensors to cloud-scale reactors.

- **📌: 3. Platform-Agnostic Reactor (`kernel_t`):** A unified abstraction over native kernel I/O `(Epoll, Kqueue, IOCP, and Npoll)`. It provides a consistent non-blocking interface across Linux, Windows, Mac, and Bare-Metal, ensuring that I/O multiplexing is always native to the silicon.

## 🖕 Technical Reality Check: Data vs. Hype

We didn't test this on a supercomputer. We tested it on an educational-grade Dual-Core Apollo Lake potato. If your framework can't perform here, it's not "scalable" — it's just hiding behind hardware.

**1 - Performance Benchmark: HTTP Throughput vs. Resource Tax**

> **Test:** 100k requests | 1k Concurrency | Environment: Localhost [see benchmark](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/server_benchmark/readme.md)

| Metric | Bun (v1.3.5) | Go (v1.18.1) | Nodepp (V1.4.0) | Impact |
| --- | --- | --- | --- | --- |
| Requests / Sec | 5,985 | 6,139 | 6,851.33 | +11.6% Performance |
| Memory (RSS) | 69.5 MB | 14.1 MB | 2.9 MB | 95.8% Reduction |
| Max Latency | 1,452 ms | 326 ms | 245 ms | Elimination of GC Spikes |
| p99 Latency | 1,159 ms | 249 ms | 187 ms | High-precision SLA stability |
| Energy Efficiency | Low | Medium | Extreme | Maximum hardware utilization |

**2 - Performace Benchmark: Resource Management & Latency Jitter Analysis**

> **Test:** 1k Cycles | 100k Allocations [see benchmark](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/gc_benchmark/readme.md)

| Runtime | Avg. Cycle Time | VIRT (Address Space) | RES (Physical RAM) | Memory Model |
| --- | --- | --- | --- | --- |
| Nodepp | 3.0 ms (±0.1 ms) | 6.1 MB | 2.7 MB | Deterministic RAII |
| Bun | 7.2 ms (5-11 ms range) | 69.3 GB | 72.6 MB | Generational GC |
| Go | < 1.0 ms* | 703.1 MB | 2.2 MB | Concurrent GC |

> **Note:** Go's <1ms measurement is a lie — it only reflects allocation latency. Reclamation is deferred to concurrent GC cycles, creating "ghost" resource pressure.

**3 - Performace Benchmark: High-Concurrency Benchmark - 100k Task Challenge**

> **Test:** 100k asynchronous tasks [see benchmark](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/task_benchmark/readme.md)

| Runtime | RSS (Memory) | CPU Load | VIRT Memory | Strategy |
| --- | --- | --- | --- | --- |
Nodepp (Balanced) | 59.1 MB | 75.9% | 153 MB | Multi-Worker Pool |
Nodepp (Single) | 59.0 MB | 59.9% | 62 MB | Single Event Loop |
Bun | 64.2 MB | 24.2% | 69.3 GB | JavaScriptCore Loop |
Go | 127.9 MB | 169.4% | 772 MB | Preemptive Goroutines |

**4 - Performace Benchmark: Nodepp Stability & Memory Benchmarks**

> **Test:** 4 Valgrind-based stress tests  [see benchmark](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/valgrind_benchmark/readme.md)

| Test Case | Objective | Iterations / Load | Memory Leaks | Result |
| --- | --- | --- | --- | --- |
| Atomic Longevity | High-concurrency HTTP | 100k requests | 0 bytes | PASSED |
| Rapid Lifecycle | Smart Pointer stress | 1M object cycles | 0 bytes | PASSED |
| Broken Pipe | Resilience to I/O failure | 100k interruptions | 0 bytes | PASSED |
| Multi-Thread Atomicity | race conditions stress | 100k Messages * 2 workers | 0 bytes | PASSED |

## 🚀 Why Nodepp Exists

The Nodepp Project did not originate in a laboratory; it was forged in the trenches of mission-critical Edge Computing and WASM development. While architecting ecosystems that bridge ESP32 hardware, web browsers, and cloud infrastructure, we identified a systemic crisis: the forced fragmentation of a single business logic across three incompatible execution environments.

- **The Edge:** Native C/C++ for low-level hardware (High performance, near-zero agility).
- **The Frontend:** JavaScript/WASM for browser interfaces (High agility, massive memory churn).
- **The Infrastructure:** Managed Runtimes like Python, Go, or Node.js for server-side orchestration (High operational cost, unpredictable latency due to Garbage Collection).

Nodepp was built to collapse these silos. By providing a unified, asynchronous C++ runtime that mirrors the productivity of scripting languages, we enable Resource-Dense Computing.

```cpp
#include <nodepp/nodepp.h>
#include <nodepp/http.h>

using namespace nodepp;

void onMain() {

    fetch_t args;
            args.method  = "GET";
            args.url     = "http://ip-api.com/json/?fields";
            args.headers = header_t({ 
                { "Host", url::host(args.url) } 
            });

    http::fetch( args )

    .then([]( http_t cli ){
        auto data = stream::await( cli );
        console::log("->", data.value());
    })

    .fail([]( except_t err ){
        console::error( err );
    });

}
```

We restore the direct relationship between code and hardware through Deterministic RAII and Stackless Coroutines, allowing you to deploy the same high-level logic from an 8-bit microcontroller to a 64-core cloud reactor without changing your mental model.

**Still Skeptical?**

Watch logic-parity in action. This isn't a "concept"—it's [ A Fully Functional Enigma Machine running in a Literal Potato board ](https://wokwi.com/projects/449104127751150593):

https://github.com/user-attachments/assets/9b870671-3854-444f-893d-40fdce31a629

## 🧭 Quick Start: High-Density HTTP
Nodepp abstracts complex socket management into a clean, event-driven API.

```cpp
#include <nodepp/nodepp.h>
#include <nodepp/regex.h>
#include <nodepp/http.h>
#include <nodepp/date.h>
#include <nodepp/os.h>

using namespace nodepp;

void onMain() {

    auto server = http::server([]( http_t cli ){
        
        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }) );

        cli.write( regex::format( R"(
            <h1> hello world </h1>
            <h2> ${0} </h2>
        )", date::fulltime() ));

        cli.close();

    });

    server.listen( "0.0.0.0", 8000, []( socket_t /*unused*/ ){
        console::log("Server listening on port 8000");
    });

}
```

## 🛟 Ecosystem

The Nodepp project is supported by a suite of modular extensions designed to follow the same unified design patterns:

- **📌: Data Parsing:** [XML](https://github.com/NodeppOfficial/nodepp-xml)
- **📌: Tor:** [Torify](https://github.com/NodeppOfficial/nodepp-torify), [JWT](https://github.com/NodeppOfficial/nodepp-jwt).
- **📌: Security:** [Argon2](https://github.com/NodeppOfficial/nodepp-argon2), 
- **📌: Web:** [ExpressPP](https://github.com/NodeppOfficial/nodepp-express), [ApifyPP](https://github.com/NodeppOfficial/nodepp-apify).
- **📌: IoT/Embedded:** [SerialPort](https://github.com/NodeppOfficial/nodepp-serial), [Bluetooth](https://github.com/NodeppOfficial/nodepp-bluetooth).
- **📌: Databases:** [Redis](https://github.com/NodeppOfficial/nodepp-redis), [Postgres](https://github.com/NodeppOfficial/nodepp-postgres), [MariaDB](https://github.com/NodeppOfficial/nodepp-mariadb), [Sqlite](https://github.com/NodeppOfficial/nodepp-sqlite).

## 🌐 One Codebase, Every Platform
Nodepp is the only framework that lets you share logic between the deepest embedded layers and the highest web layers.

- **📌: Hardware:** [NodePP for Arduino](https://github.com/NodeppOfficial/nodepp-arduino)
- **📌: Desktop:** [Nodepp for Desktop](https://github.com/NodeppOfficial/nodepp)
- **📌: Browser:** [Nodepp for WASM](https://github.com/NodeppOfficial/nodepp-wasm)
- **📌: ESP32:** [Nodepp for ESP32](https://github.com/NodeppOfficial/nodepp-esp32)
- **📌: EPS8266:** [Nodepp for ESP8266](https://github.com/NodeppOfficial/nodepp-esp8266)

## ❤️‍🩹 Contributing

Nodepp is an open-source project that values Mechanical Sympathy and Technical Excellence.

- **📌: Sponsorship:** Support the project via [Ko-fi](https://ko-fi.com/edbc_repo).
- **📌: Bug Reports:** Open an issue via GitHub.
- **📌: License:** MIT.

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/edbc_repo)

## 🛡️ License
**Nodepp** is distributed under the MIT License. See the LICENSE file for more details.
