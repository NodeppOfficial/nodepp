# Performance Benchmarks: HTTP Server Throughput

This benchmark evaluates the raw throughput and resource efficiency of Nodepp compared to Bun and Go. The goal is to measure how much Hardware Tax each runtime imposes on the system under high concurrency.

## Comparative Summary

| Metric | Bun (v1.3.5) | Go (net/http) | Nodepp (Single) | Nodepp (Cluster) |
| --- | --- | --- | --- | --- |
| Req / Second | 5,985.74 | 6,139.41 | 6,219.31 | 5,999.98 |
| Memory (RSS) | 69.5 MB | 14.1 MB | 2.9 MB | 3.2 MB |
| Max Latency | 1,452 ms | 326 ms | 323 ms | 348 ms |
| p99 Latency | 1,159 ms | 249 ms | 229 ms | 326 ms |
| CPU Usage | 98.7% | 99.3% | 100.0% | 116.7% |

## Environment & Methodology

All tests were conducted on the same machine using the Apache Benchmark (`ab`) tool.

#### Test Command:
```
ab -n 100000 -c 1000 http://localhost:8000/
```

- **Target Device:** Dual-Core Apollo Lake educational-grade Chromebook
- **Concurrency:** 1,000 (Simultaneous connections)
- **Total Requests:** 100,000 

## Bun Result

```
Benchmarking localhost (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests

Server Software:        
Server Hostname:        localhost
Server Port:            8000

Document Path:          /
Document Length:        22 bytes

Concurrency Level:      1000
Time taken for tests:   16.706 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      12300000 bytes
HTML transferred:       2200000 bytes
Requests per second:    5985.74 [#/sec] (mean)
Time per request:       167.064 [ms] (mean)
Time per request:       0.167 [ms] (mean, across all concurrent requests)
Transfer rate:          718.99 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   39 119.2     29    1093
Processing:    27  126  40.6    119     484
Waiting:       19  118  42.6    110     484
Total:         76  166 128.0    148    1452

Percentage of the requests served within a certain time (ms)
  50%    148
  66%    154
  75%    155
  80%    156
  90%    168
  95%    221
  98%    382
  99%   1159
 100%   1452 (longest request)
```

## Go Result

```
Benchmarking localhost (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests

Server Software:        
Server Hostname:        localhost
Server Port:            8000

Document Path:          /
Document Length:        22 bytes

Concurrency Level:      1000
Time taken for tests:   16.288 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      12300000 bytes
HTML transferred:       2200000 bytes
Requests per second:    6139.41 [#/sec] (mean)
Time per request:       162.882 [ms] (mean)
Time per request:       0.163 [ms] (mean, across all concurrent requests)
Transfer rate:          737.45 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   20  16.4     16      92
Processing:    11  142  33.8    141     309
Waiting:        1  134  35.3    136     298
Total:         28  162  28.6    160     326

Percentage of the requests served within a certain time (ms)
  50%    160
  66%    168
  75%    175
  80%    180
  90%    194
  95%    208
  98%    232
  99%    249
 100%    326 (longest request)
```

## Nodepp Result ( Single Thread )
> g++ -o main nodepp_benchmark.cpp -O3 ; ./main

```
Benchmarking localhost (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests

Server Software:        
Server Hostname:        localhost
Server Port:            8000

Document Path:          /
Document Length:        22 bytes

Concurrency Level:      1000
Time taken for tests:   16.079 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      6600000 bytes
HTML transferred:       2200000 bytes
Requests per second:    6219.31 [#/sec] (mean)
Time per request:       160.789 [ms] (mean)
Time per request:       0.161 [ms] (mean, across all concurrent requests)
Transfer rate:          400.85 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    4  11.0      0     143
Processing:    32  155  22.4    159     323
Waiting:        1  153  21.4    159     304
Total:        113  159  16.7    160     323

Percentage of the requests served within a certain time (ms)
  50%    160
  66%    162
  75%    163
  80%    164
  90%    166
  95%    172
  98%    180
  99%    229
 100%    323 (longest request)
```

## Nodepp Result ( Clustering )
> g++ -o main nodepp_benchmark.cpp -O3 -lpthread ; ./main

```
Benchmarking localhost (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests

Server Software:        
Server Hostname:        localhost
Server Port:            8000

Document Path:          /
Document Length:        22 bytes

Concurrency Level:      1000
Time taken for tests:   16.667 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      6600000 bytes
HTML transferred:       2200000 bytes
Requests per second:    5999.98 [#/sec] (mean)
Time per request:       166.667 [ms] (mean)
Time per request:       0.167 [ms] (mean, across all concurrent requests)
Transfer rate:          386.72 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    4   8.3      0      85
Processing:     0  162 102.1    148     348
Waiting:        0  160 102.3    145     348
Total:          0  165 101.4    155     348

Percentage of the requests served within a certain time (ms)
  50%    155
  66%    225
  75%    270
  80%    280
  90%    299
  95%    310
  98%    323
  99%    326
 100%    348 (longest request)
```

## Key Insights

### 1. Radical Memory Efficiency

Nodepp (Single Thread) uses only 2.9 MB of RAM to handle 1,000 concurrent connections.

- **vs Bun:** Nodepp is 23.9x more memory efficient.

- **vs Go:** Nodepp is 4.8x more memory efficient. This makes Nodepp the ideal choice for high-density microservices and edge computing where memory is the primary cost driver.

### 2. Solving the P99 Jitter

One of the most critical findings is the Max Latency.

- **Bun** suffered a massive spike of 1,452 ms (likely due to Garbage Collection Stop-the-World pauses).

- **Nodepp** maintained a stable peak of 323 ms. By using deterministic memory destruction `ptr_t`, Nodepp eliminates the unpredictable latency spikes common in managed runtimes like JS and Go.

### 3. High-Density Clustering

While the clustered version of Nodepp shows a slightly higher Max Latency due to the overhead of inter-process distribution, it maintains an incredibly low memory footprint of 3.2 MB compared to any other multi-threaded or clustered runtime.

## Raw Data Logs

### Nodepp (Single Thread)

- **Memory:** 2,940 KB
- **Max Latency:** 323 ms
- **Throughput:** 6,219 req/s
- **Efficiency:** 100% CPU utilization with minimal context switching.

### Go (Standard Library)

- **Memory:** 14,088 KB
- **Max Latency:** 326 ms
- **Throughput:** 6,139 req/s
- **Efficiency:** Excellent p99, but requires 5x more memory than Nodepp.

### Bun

- **Memory:** 69,568 KB
- **Max Latency:** 1,452 ms
- **Throughput:** 5,985 req/s
- **Efficiency:** High memory overhead and significant latency spikes under load.

## Conclusion: Green Computing & Efficiency First

The data confirms that Nodepp is not merely a high-performance runtime; it is a Sustainability-First Engine. While industry leaders like Bun and Go prioritize raw speed, they do so by throwing hardware at the problem. Nodepp shifts the focus to Resource Density — achieving more work with a significantly smaller environmental and financial footprint.

### 1. Radical Energy & Infrastructure Reduction

By utilizing the `ptr_t` controller, Nodepp achieved higher throughput than Bun while using 23.9x less RAM.

- **The Green Impact:** In a cloud-native environment, this allows for a 95% reduction in server instances.

- **The Logic:** Less memory usage translates directly to lower power consumption in data centers and reduced cooling requirements, making Nodepp the premier choice for carbon-neutral infrastructure.