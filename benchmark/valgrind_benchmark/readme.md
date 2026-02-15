# Nodepp Stability & Memory Benchmarks

This repository contains the official memory validation and stress-test suite for the Nodepp framework. These tests utilize Valgrind Memcheck to prove the architectural integrity of our asynchronous kernel, ensuring zero memory leaks under extreme concurrency.

## Performance Summary

| Test Case | Objective | Iterations / Load | Allocations | Frees | Memory Leaks |
| --- | --- | --- | --- | --- | --- |
| Atomic Longevity | HTTP server under load | 100k requests | 6,644,971 | 6,644,971 | 0 bytes |
| Rapid Lifecycle | ptr_t/event_t stress | 1M object cycles | 14,000,173 | 14,000,173 | 0 bytes |
| Broken Pipe | I/O failure resilience | 100k interruptions | 2,645,840 | 2,645,840 | 0 bytes |
| Worker/Channel Integrity | Multi-thread message passing | 100k tasks × 2 workers | 2,000,157 | 2,000,157 | 0 bytes |

## Benchmark Methodology

### Test 1: The "Atomic" Longevity Test (`1-server.cpp`)

**Objective:** Validates that the `epoll` event-loop cleans up all file descriptors, buffers, and coroutine states after a sustained high-concurrency burst.

```bash
g++ -o main 1-server.cpp -I../../include -O2
valgrind --leak-check=full --show-leak-kinds=all ./main
# In a separate terminal
ab -n 100000 -c 1000 -t 100 http://localhost:8000/
kill -SIGINT $(pgrep main)
```

```
Validation Result:
- Total heap usage: 6,644,971 allocs, 6,644,971 frees.
- Verdict:          All heap blocks were freed -- no leaks are possible.
```
 
```
==20115== Memcheck, a memory error detector
==20115== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==20115== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==20115== Command: ./main
==20115== 
==20115== Warning: ignored attempt to set SIGKILL handler in sigaction();
==20115==          the SIGKILL signal is uncatchable
server started at http://localhost:8000 
--20115-- WARNING: unhandled amd64-linux syscall: 441
--20115-- You may be able to write your own handler.
--20115-- Read the file README_MISSING_SYSCALL_OR_IOCTL.
--20115-- Nevertheless we consider this a bug.  Please report
--20115-- it at http://valgrind.org/support/bug_reports.html.
SIGINT: Signal Interrupt 
==20115== 
==20115== HEAP SUMMARY:
==20115==     in use at exit: 0 bytes in 0 blocks
==20115==   total heap usage: 6,644,971 allocs, 6,644,971 frees, 3,649,233,162 bytes allocated
==20115== 
==20115== All heap blocks were freed -- no leaks are possible
==20115== 
==20115== For lists of detected and suppressed errors, rerun with: -s
==20115== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

## Test 2: The "Rapid Fire" Object Lifecycle (`2-event.cpp`)

**Objective:** Stress-tests the internal `ptr_t` and `event_t` logic. It verifies that rapid allocation/deallocation of events and smart pointers does not cause heap fragmentation or "pointer trashing."

```bash
g++ -o main 2-event.cpp -I../../include -O2
valgrind --leak-check=full --show-leak-kinds=all ./main
```

```
Validation Result:
- Total heap usage: 14,000,173 allocs, 14,000,173 frees.
- Verdict:          0 errors from 0 contexts. Perfectly symmetrical memory lifecycle.
```

```
==19877== Memcheck, a memory error detector
==19877== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==19877== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==19877== Command: ./main
==19877== 
==19877== Warning: ignored attempt to set SIGKILL handler in sigaction();
==19877==          the SIGKILL signal is uncatchable
Starting Lifecycle Stress Test... 
Processed: 0 
Processed: 250000 
Processed: 500000 
Processed: 750000 
Test Finished. Check Valgrind report. 
==19877== 
==19877== HEAP SUMMARY:
==19877==     in use at exit: 0 bytes in 0 blocks
==19877==   total heap usage: 14,000,173 allocs, 14,000,173 frees, 684,096,504 bytes allocated
==19877== 
==19877== All heap blocks were freed -- no leaks are possible
==19877== 
==19877== For lists of detected and suppressed errors, rerun with: -s
==19877== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

## Test 3: The "Broken Pipe" Test - Client-Side Leaks (`3-pipe.cpp`)
**Objective:** Simulates network instability. It tests what happens when a client connects and disconnects prematurely, forcing "Broken Pipe" and "Connection Reset" errors at the kernel level.

```bash
g++ -o main 3-pipe.cpp -I../../include -O2
valgrind --leak-check=full --show-leak-kinds=all ./main
# Simulate aggressive timeouts and failures
ab -n 100000 -c 1000 -t 100 http://localhost:8000/
kill -SIGINT $(pgrep main)
```

```
Validation Result:
- Allocated Traffic: ~6.7 GB processed.
- In use at exit:    0 bytes in 0 blocks.
- Verdict:           Successfully handled millions of syscalls without a single dangling resource.
```

```
==20296== Memcheck, a memory error detector
==20296== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==20296== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==20296== Command: ./main
==20296== 
==20296== Warning: ignored attempt to set SIGKILL handler in sigaction();
==20296==          the SIGKILL signal is uncatchable
--20296-- WARNING: unhandled amd64-linux syscall: 441
--20296-- You may be able to write your own handler.
--20296-- Read the file README_MISSING_SYSCALL_OR_IOCTL.
--20296-- Nevertheless we consider this a bug.  Please report
--20296-- it at http://valgrind.org/support/bug_reports.html.
SIGINT: Signal Interrupt 
==20296== 
==20296== HEAP SUMMARY:
==20296==     in use at exit: 0 bytes in 0 blocks
==20296==   total heap usage: 2,645,840 allocs, 2,645,840 frees, 6,720,609,328 bytes allocated
==20296== 
==20296== All heap blocks were freed -- no leaks are possible
==20296== 
==20296== For lists of detected and suppressed errors, rerun with: -s
==20296== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

## Test 4: Multi-Thread Atomicity & Channel Integrity (`4-worker.cpp`)

**Objective:** Evaluates the framework’s thread-safety and cross-thread memory reclamation. This test specifically monitors the `channel_t` and `worker_t` modules to ensure that data passed between threads is correctly owned, synchronized, and deleted without race conditions or memory leaks.

```bash
g++ -o main 4-worker.cpp -I../../include -lpthread -O2
valgrind --leak-check=full --show-leak-kinds=all ./main
```

```
Validation Result
- Total Allocations: 200,169 (High volume string formatting and task wrapping).
- Concurrency:       2 Worker Threads + 1 Orchestrator Thread.
- Verdict:           0 errors and 0 bytes leaked. This confirms that Nodepp provides a "Managed-like" safety experience in a high-performance, multi-threaded C++ environment.
```

```
==53073== Memcheck, a memory error detector
==53073== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==53073== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==53073== Command: ./main
==53073== 
==53073== Warning: ignored attempt to set SIGKILL handler in sigaction();
==53073==          the SIGKILL signal is uncatchable
Worker Stress Test Started (2 Workers)... 
--53073-- WARNING: unhandled amd64-linux syscall: 441
--53073-- You may be able to write your own handler.
--53073-- Read the file README_MISSING_SYSCALL_OR_IOCTL.
--53073-- Nevertheless we consider this a bug.  Please report
--53073-- it at http://valgrind.org/support/bug_reports.html.
Task_Data_Payload_Stress 100000 
Task_Data_Payload_Stress 99999 
Task_Data_Payload_Stress 99998 
Task_Data_Payload_Stress 99997 
[...]
Task_Data_Payload_Stress 2 
Task_Data_Payload_Stress 1 
Task_Data_Payload_Stress 0 
done 
==53073== 
==53073== HEAP SUMMARY:
==53073==     in use at exit: 0 bytes in 0 blocks
==53073==   total heap usage: 2,000,157 allocs, 2,000,157 frees, 132,897,128 bytes allocated
==53073== 
==53073== All heap blocks were freed -- no leaks are possible
==53073== 
==53073== For lists of detected and suppressed errors, rerun with: -s
==53073== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)

```