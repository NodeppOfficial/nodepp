# Nodepp: The Unified Asynchronous Real-Time C++ Runtime

[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20WASM-blue)](https://github.com/NodeppOfficial/nodepp)
[![Build Status](https://github.com/NodeppOfficial/nodepp/actions/workflows/main.yml/badge.svg)](https://github.com/NodeppOfficial/nodepp/actions)
[![Valgrind Memory Test](https://img.shields.io/badge/memory-zero_leaks-green)](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/valgrind_benchmark/readme.md)

Nodepp is a vertically integrated C++ runtime engineered to solve the Efficiency Paradox of modern distributed computing. While managed runtimes prioritize developer velocity at the cost of hardware overhead, Nodepp delivers industry-leading throughput with a near-zero resource footprint through deterministic memory management and hardware-aligned architecture.

By unifying the event reactor, protocol parsers, and memory controller into a single architectural DNA, Nodepp eliminates the Abstraction Tax that inflates infrastructure costs and energy consumption.

## 📃 Whitepaper

[Scaling the Talent Bridge for Green Computing: Achieving Silicon-Logic Parity through Deterministic RAII](https://github.com/NodeppOfficial/nodepp/blob/main/WHITEPAPER.md) Read the full technical breakdown, including architectural deep-dives into `ptr_t`, `kernel_t` and `coroutine_t`.

## 🌿 Sustainability & Performance (Green Computing)

Nodepp is designed for Resource Density. While modern runtimes often throw hardware at the problem, Nodepp optimizes the software to respect the silicon. By reducing memory overhead and CPU jitter, we directly decrease the energy footprint of digital infrastructure.

### 📈 Performance Benchmark: HTTP Throughput vs. Resource Tax

> **Test:** 100k requests | 1k Concurrency | Environment: Localhost | Device: Educational-grade Dual-Core Apollo lake Chromebook [see benchmark](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/server_benchmark/readme.md)

| Metric | Bun (v1.3.5) | Go (v1.18.1) | Nodepp (V1.4.0) | Impact |
| --- | --- | --- | --- | --- |
| Requests / Sec | 5,985 | 6,139 | 6,851.33 | +11.6% Performance |
| Memory (RSS) | 69.5 MB | 14.1 MB | 2.9 MB | 95.8% Reduction |
| Max Latency | 1,452 ms | 326 ms | 245 ms | Elimination of GC Spikes |
| p99 Latency | 1,159 ms | 249 ms | 187 ms | High-precision SLA stability |
| Energy Efficiency | Low | Medium | Extreme | Maximum hardware utilization |

### 📈 Performace Benchmark: Resource Management & Latency Jitter Analysis

> **Test:** 1k Cycles | 100k Allocations | Environment: Educational-grade Dual-Core Apollo lake Chromebook [see benchmark](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/gc_benchmark/readme.md)

| Runtime | Avg. Cycle Time | VIRT (Address Space) | RES (Physical RAM) | Memory Model |
| --- | --- | --- | --- | --- |
| Nodepp | 3.0 ms (±0.1 ms) | 6.1 MB | 2.7 MB | Deterministic RAII |
| Bun | 7.2 ms (5-11 ms range) | 69.3 GB | 72.6 MB | Generational GC |
| Go | < 1.0 ms* | 703.1 MB | 2.2 MB | Concurrent GC |

> **Note:** Go's <1ms measurement reflects allocation latency only; reclamation is deferred to concurrent garbage collection cycles.

### 📈 Performace Benchmark: High-Concurrency Benchmark - 100k Task Challenge

> **Test:** 100k asynchronous tasks | Environment: Educational-grade Dual-Core Apollo lake Chromebook [see benchmark](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/task_benchmark/readme.md)

| Runtime | RSS (Memory) | CPU Load | VIRT Memory | Strategy |
| --- | --- | --- | --- | --- |
Nodepp (Balanced) | 59.1 MB | 75.9% | 153 MB | Multi-Worker Pool |
Nodepp (Single) | 59.0 MB | 59.9% | 62 MB | Single Event Loop |
Bun | 64.2 MB | 24.2% | 69.3 GB | JavaScriptCore Loop |
Go | 127.9 MB | 169.4% | 772 MB | Preemptive Goroutines |

## ⭐ Architectural Philosophy

- **📌: 1. Mechanical Sympathy & Hardware-Centric Design:**  Nodepp replaces high-level abstractions with a reference-counted memory model (`ptr_t`) that eliminates the unpredictable latency of Garbage Collection. This ensures Deterministic Latency and zero heap fragmentation, allowing the system to maintain a flat memory profile even under extreme network saturation.

- **📌: 2. Integrated Poll-Aware Protocol Stack:** By implementing native parsers (HTTP, WebSocket, JSON) directly on the `kernel_t` reactor core, Nodepp eliminates the "Copy Tax" typical of generic libraries. This architecture maximizes Cache Locality, keeping data "hot" in L1/L2 caches and reducing context-switch overhead during protocol negotiation.

- **📌: 3. Universal Reactor & Platform Parity:** The "Shared-Nothing" architecture scales across the entire hardware spectrum—from 8-bit Microcontrollers to 64-bit Cloud Clusters. By supporting native backends like `Epoll, IOCP, and Kqueue`, Nodepp ensures total logic parity between embedded IoT devices, WebAssembly, and POSIX-compliant servers.

## 🧭 Technical Example: Asynchronous HTTP Server

This example demonstrates the integration of the event loop with a basic HTTP reactor, showcasing the unified API style.

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
- **📌: IOT:** [ Nodepp for ESP32 ](https://github.com/NodeppOfficial/nodepp-esp32)

## ❤️‍🩹 Contributing

Nodepp is an open-source project that values Mechanical Sympathy and Technical Excellence.

- **📌: Sponsorship:** Support the project via [Ko-fi](https://ko-fi.com/edbc_repo).
- **📌: Bug Reports:** Open an issue via GitHub.
- **📌: License:** MIT.

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/edbc_repo)

## 🛡️ License
**Nodepp** is distributed under the MIT License. See the LICENSE file for more details.
