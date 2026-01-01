# Concurrent Polynomial Processing System (C)

A high-performance parallel processing system that implements the **Producer-Consumer** design pattern using System V Shared Memory, POSIX Semaphores, and Multithreading. The system offloads complex polynomial mathematics to background workers, utilizing granular locking mechanisms for thread safety.

## 🚀 Key Features

- **Inter-Process Communication (IPC):**
  - Implements a zero-copy data pipeline using **System V Shared Memory** (`shmget`, `shmat`).
  - Custom serialization protocol to pack complex Polynomial structs into contiguous shared memory blocks.
- **Synchronization & Safety:**
  - Process-level synchronization using **POSIX Semaphores** to prevent race conditions during memory writes.
  - Thread-level synchronization using **Mutexes** to ensure atomic updates to result buffers.
- **Parallel Execution:**
  - **Multi-Process:** Decouples input parsing (Producer) from calculation (Consumer).
  - **Multi-Threaded:** The Consumer spins up worker threads (`pthread`) to calculate polynomial coefficients in parallel, maximizing CPU utilization for large data sets.

## 🛠 Technical Architecture

### The Pipeline

1.  **Producer (`ex3q3a`):**

    - Parses raw string input (e.g., `(3:1,2,1)ADD(2:1,1)`).
    - Acquires the named Semaphore.
    - Serializes the polynomial objects into the Shared Memory segment.
    - Releases the Semaphore to signal data availability.

2.  **Consumer (`ex3q3b`):**
    - Polls the Shared Memory for new operation headers.
    - Deserializes data back into local structs.
    - Spawns $N$ threads (where $N$ is the degree of the polynomial) to compute coefficients concurrently.
    - Aggregates results and clears the shared buffer.

### Custom Memory Protocol

To share dynamic data structures (linked lists/arrays) across process boundaries, a custom "flat" memory layout was engineered:

```text
[OpCode (1b)] [OffsetA (2b)] [OffsetB (2b)] ... [PolyA Data] ... [PolyB Data]
```
