# High-Concurrency Benchmark: 100k Task Challenge

This benchmark evaluates the performance, memory density, and CPU utilization of Nodepp (C++), Bun (Zig/JS), and Go when managing 100,000 concurrent tasks with a 10ms resolution.

| Runtime | RSS (Memory) | CPU Load | VIRT Memory | Strategy |
| --- | --- | --- | --- | --- |
Nodepp (Balanced) | 59.1 MB | 75.9% | 153 MB | Multi-Worker Pool |
Nodepp (Single) | 59.0 MB | 59.9% | 62 MB | Single Event Loop |
Bun | 64.2 MB | 24.2% | 69.3 GB | JavaScriptCore Loop |
Go | 127.9 MB | 169.4% | 772 MB | Preemptive Goroutines |

```
#raw data
PID  | USUARIO | PR | NI | VIRT   | RES    | SHR   | S | %CPU  | %MEM | HORA+   | ORDEN   
6816 | penguin | 20 | 0  | 153660 | 59168  | 2848  | S | 75,9  | 1,5  | 0:06.80 | nodepp_balanced
6866 | penguin | 26 | 6  | 62424  | 59012  | 2564  | R | 59,9  | 1,5  | 0:02.16 | nodepp   
4478 | penguin | 20 | 0  | 69,3g  | 64244  | 33068 | R | 24,2  | 1,7  | 0:00.73 | bun
4538 | penguin | 20 | 0  | 772232 | 127992 | 1144  | R | 169,4 | 3,3  | 0:07.31 | go
```

## Key Findings
- **Memory Density Leadership:** Nodepp demonstrated the highest memory density, requiring only ~590 bytes per active task. In comparison, Go consumed more than double the memory (~1.2 KB per task) due to the overhead of the Goroutine stack management and the Go runtime.

- **The Virtual Memory Ghost:** While Bun showed impressive physical RAM (RSS) usage, its Virtual Memory (VIRT) allocation was massive (69.3 GB). This is a byproduct of the JavaScriptCore heap pre-allocation strategy, which can be a constraint in restricted container environments (K8s/Docker) where VIRT limits are enforced.

- **CPU Efficiency vs. Multi-Core Magic:** Go hit 169.4% CPU, automatically utilizing multiple cores. While this simplifies development, it increases total system energy consumption. Instead, Nodepp (Balanced) utilized 75.9% CPU by manually sharding the workload across workers. This Shared Nothing architecture ensures that each core is utilized without the lock contention often found in the Go scheduler.

## Conclusion

The data suggests that for High-Density Concurrency, Nodepp provides the most predictable and efficient resource profile. It is the ideal choice for systems where memory footprint and deterministic CPU usage are prioritized over out-of-the-box automatic scaling.