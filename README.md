# Nodepp: The Unified Asynchronous Real-Time C++ Runtime

[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20Arduino%20%7C%20WASM-blue)](https://github.com/NodeppOfficial/nodepp)
[![Build Status](https://github.com/NodeppOfficial/nodepp/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/NodeppOfficial/nodepp/actions)

Nodepp is a high-performance C++ runtime designed to solve the Efficiency Paradox of modern cloud and edge computing. While traditional managed runtimes prioritize developer speed at the cost of massive hardware overhead, Nodepp provides a Vertically Integrated Architecture that achieves industry-leading throughput with a near-zero resource footprint.

By unifying the event-reactor, protocol parsers, and memory management into a single Architectural DNA, Nodepp eliminates the Abstraction Tax that typically inflates infrastructure costs.

## Whitepaper

[Green Computing at the Edge: Scaling Resource-Dense Applications through Deterministic RAII and Silicon-Logic Parity.](https://github.com/NodeppOfficial/nodepp/blob/main/WHITEPAPER.md) Read the full technical breakdown, including architectural deep-dives into `ptr_t`, `kernel_t` and `loop_t`.

## Sustainability & Performance (Green Computing)

Nodepp is designed for Resource Density. While modern runtimes often throw hardware at the problem, Nodepp optimizes the software to respect the silicon. By reducing memory overhead and CPU jitter, we directly decrease the energy footprint of digital infrastructure.

### Performance Benchmark: HTTP Throughput vs. Resource Tax

> **Test:** 100k requests | 1k Concurrency | Environment: Localhost | Device: Educational-grade Dual-Core Apollo lake Chromebook [see menchmark](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/server_benchmark/readme.md)

| Metric | Bun (v1.3.5) | Go (net/http) | Nodepp (V1.4.0) | Impact |
| --- | --- | --- | --- | --- |
| Requests / Sec | 5,985 | 6,139 | 6,219 | +4% Performance |
| Memory (RSS) | 69.5 MB | 14.1 MB | 2.9 MB | 95% Reduction |
| Max Latency | 1,452 ms | 326 ms | 323 ms | Zero GC Jitter |
| Energy Efficiency | Low | Medium | Extreme | High Work-per-Watt |

### Performace Benchmark: Resource Management & Latency Jitter Analysis

> **Test:** 1k Cycles | 100k Allocations | Environment: Educational-grade Dual-Core Apollo lake Chromebook [see menchmark](https://github.com/NodeppOfficial/nodepp/blob/main/benchmark/gc_benchmark/readme.md)

| Runtime | Avg. Cycle Time | VIRT (Address Space) | RES (Physical RAM) | Memory Model |
| --- | --- | --- | --- | --- |
| Nodepp | 3.0 ms | 6.1 MB | 2.7 MB | Deterministic RAII |
| Bun | 7.2 ms (avg) | 69.3 GB | 72.6 MB | Generational GC |
| Go | < 1.0 ms* | 703.1 MB | 2.2 MB | Concurrent GC |

> **Note:** Go's <1ms result reflects Deferred Debt, where deallocation is bypassed during the measurement window.

### Why this matters for Green IT:

- **Infrastructure Density:** Nodepp's 2.9MB footprint allows you to run 23x more services on the same physical hardware compared to Bun. This eliminates the need for expensive vertical scaling and reduces data center cooling requirements.

- **Deterministic Energy Draw:** By removing Garbage Collection (GC) spikes, Nodepp prevents sudden CPU bursts. This leads to a stable, lower power draw, extending the battery life of IoT devices and reducing the carbon intensity of cloud workloads.

- **E-Waste Reduction:** Nodepp’s ability to run advanced asynchronous logic on 8-bit/32-bit MCUs prevents Hardware Forced Obsolescence, allowing legacy hardware to perform like modern high-end silicon.

## Architectural Design Choices

### Engineering Philosophy: Mechanical Sympathy

Rooted in **Electrical Engineering principles**, Nodepp implements hardware-centric design patterns to mitigate the systemic flaws of high-level runtimes:

- **Deterministic Latency:** Eliminates the unpredictable spikes of Garbage Collection (GC) through manual, reference-counted memory safety.

- **Zero Heap Fragmentation:** Employs a fragmentation-resistant memory model `ptr_t` optimized for long-running asynchronous processes.

- **Scale-Invariant Logic:** Enables the same high-level logic to run with parity across the hardware spectrum — from 8-bit microcontrollers to 64-bit clustered cloud servers.

### Integrated Protocol Stack

Nodepp implements its own protocol parsers ( HTTP, WebSocket ) specifically to ensure they are Poll-Aware. By building these engines directly on the `kernel_t` reactor core:

- **Mechanical Sympathy:** Data moves from the system-socket directly into logic. This removes the Copy Tax found in generic libraries.

- **Cache Locality:** Sharing the memory logic with the core reactor keeps data hot in the L1/L2 cache, significantly reducing latency jitter.

### Core Implementation & Platform Reach

- **Reactor Backends:** Supports native event-multiplexing via Epoll (Linux), IOCP (Windows), Kqueue (BSD/Mac) and busy-while loop ( embedded systems ).

- **Concurrency Model:** Utilizes a Shared-Nothing architecture combined with isolated workers to prevent lock contention.

- **Cross-Platform Parity:** Designed to maintain logic consistency across 8-bit/32-bit Microcontrollers (Arduino), WebAssembly (WASM), and POSIX-compliant operating systems.

## Technical Example: Asynchronous Observation

This example demonstrates the integration of the event loop with a basic HTTP reactor, showcasing the unified API style.

```cpp
#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/regex.h>
#include <nodepp/http.h>
#include <nodepp/date.h>
#include <nodepp/os.h>

using namespace nodepp;

void worker_isolated_main() {

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

void onMain() {

    for( auto x=os::cpus(); x-->0; ){

        worker::add([=](){
            worker_isolated_main();
            process::wait();
            return -1;
        });

    }

}
```

> The example above demonstrates an isolated event-loop per CPU core. Because `date::fulltime()` and `regex::format` are built on the `ptr_t` architecture, they utilize the same non-blocking, fragmentation-resistant memory model as the server itself.

## Research & Ecosystem

The Nodepp project is supported by a suite of modular extensions designed to follow the same unified design patterns:

- **Data Parsing:** [XML](https://github.com/NodeppOfficial/nodepp-xml)
- **Tor:** [Torify](https://github.com/NodeppOfficial/nodepp-torify), [JWT](https://github.com/NodeppOfficial/nodepp-jwt).
- **Security:** [Argon2](https://github.com/NodeppOfficial/nodepp-argon2), 
- **Web:** [ExpressPP](https://github.com/NodeppOfficial/nodepp-express), [ApifyPP](https://github.com/NodeppOfficial/nodepp-apify).
- **IoT/Embedded:** [SerialPort](https://github.com/NodeppOfficial/nodepp-serial), [Bluetooth](https://github.com/NodeppOfficial/nodepp-bluetooth).
- **Databases:** [Redis](https://github.com/NodeppOfficial/nodepp-redis), [Postgres](https://github.com/NodeppOfficial/nodepp-postgres), [MariaDB](https://github.com/NodeppOfficial/nodepp-mariadb), [Sqlite](https://github.com/NodeppOfficial/nodepp-sqlite).

## One Codebase, Every Screen
Nodepp is the only framework that lets you share logic between the deepest embedded layers and the highest web layers.

- **Hardware:** [NodePP for Arduino](https://github.com/NodeppOfficial/nodepp-arduino)
- **Desktop:** [Nodepp for Desktop](https://github.com/NodeppOfficial/nodepp)
- **Browser:** [Nodepp for WASM](https://github.com/NodeppOfficial/nodepp-wasm)

## Contributing

Nodepp is an open-source project that values Mechanical Sympathy and Technical Excellence.

- **Sponsorship:** Support the project via [Ko-fi](https://ko-fi.com/edbc_repo).
- **Bug Reports:** Open an issue via GitHub.
- **License:** MIT.

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/edbc_repo)

## License
**Nodepp** is distributed under the MIT License. See the LICENSE file for more details.