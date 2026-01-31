# Nodepp Stability & Memory Benchmarks

This repository contains the official memory validation and stress-test suite for the Nodepp framework. These tests utilize Valgrind Memcheck to prove the architectural integrity of our asynchronous kernel, ensuring zero memory leaks under extreme concurrency.

## Performance Summary

| Test Case | Objective | Iterations / Load | Memory Leaks | Result |
| --- | --- | --- | --- | --- |
| Atomic Longevity | High-concurrency HTTP | 100k requests | 0 bytes | PASSED |
| Rapid Lifecycle | Smart Pointer stress | 1M object cycles | 0 bytes | PASSED |
| Broken Pipe | Resilience to I/O failure | 100k interruptions | 0 bytes | PASSED |

## Benchmark Methodology

### Test 1: The "Atomic" Longevity Test (`1-server.cpp`)

**Objective:** Validates that the `epoll` event-loop cleans up all file descriptors, buffers, and coroutine states after a sustained high-concurrency burst.

```bash
g++ -o main 1-server.cpp -I../../include
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
g++ -o main 2-event.cpp -I../../include ; valgrind --tool=memcheck ./main
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
g++ -o main 3-pipe.cpp -I../../include
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