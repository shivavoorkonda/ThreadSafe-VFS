# ThreadSafe Virtual File System (VFS) 🖥️

**Developed by Shiva Voorkonda**

A highly concurrent, user-space **Virtual File System** written in **C++17** that simulates core Operating System internals — custom memory management, virtual memory paging with LRU eviction, reader-writer synchronization, and real-time deadlock detection — all running in a multithreaded environment.

---

## 📌 Table of Contents

- [Overview](#overview)
- [OS Concepts Demonstrated](#os-concepts-demonstrated)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Core Features](#core-features)
- [Prerequisites](#prerequisites)
- [How to Build and Run](#how-to-build-and-run)
- [System Proof of Work](#system-proof-of-work)
- [Tech Stack](#tech-stack)
- [License](#license)
- [Author](#author)

---

## Overview

This project implements a **user-space Virtual File System** that manages files entirely in a custom memory pool (50 MB), simulating how real operating systems handle memory allocation, page faults, and concurrent access. When RAM is exhausted, the system automatically **pages out** the least-recently-used files to disk and **pages them back in** on demand — exactly like OS-level virtual memory.

Multiple threads can read and write files simultaneously with proper synchronization, while a background **Deadlock Detection Daemon** continuously monitors for circular wait conditions using graph-based cycle detection.

---

## OS Concepts Demonstrated

| Concept | Implementation |
|---|---|
| **Custom Memory Allocation** | 50 MB pre-allocated pool divided into 4 KB pages, replacing `malloc`/`new` |
| **Virtual Memory & Paging** | LRU-based page-out to `swap.sys` on disk when RAM is full |
| **Page Faults** | Automatic detection and page-in when accessing swapped-out files |
| **Reader-Writer Locks** | `std::shared_mutex` allows concurrent reads, exclusive writes |
| **Deadlock Detection** | Background daemon builds a Wait-For Graph and runs DFS for cycle detection |
| **Multithreading** | Concurrent file operations using `std::thread` with proper synchronization |

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                      main.cpp                           │
│            (Orchestrator & Stress Test)                  │
├────────────┬────────────┬──────────────┬────────────────┤
│            │            │              │                │
│  VFS.h     │ Memory     │  Swapper.h   │  Deadlock      │
│ (Virtual   │ Allocator.h│ (LRU Paging) │  Detector.h    │
│  File      │ (50MB Pool │              │ (Wait-For      │
│  Struct)   │  4KB Pages)│              │  Graph + DFS)  │
│            │            │              │                │
├────────────┴────────────┴──────┬───────┴────────────────┤
│         RAM (Custom Pool)      │      Disk (swap.sys)   │
│      ┌───┬───┬───┬───┬───┐    │    ┌──────────────┐    │
│      │4KB│4KB│4KB│...│4KB│◄───┼───►│  swap.sys     │    │
│      └───┴───┴───┴───┴───┘    │    │ (paged-out    │    │
│        12,800 pages total     │    │  file data)   │    │
└───────────────────────────────┴────┴──────────────┘────┘
```

---

## Project Structure

```
ThreadSafe-VFS/
├── main.cpp              # Entry point — stress test & concurrent R/W demo
├── MemoryAllocator.h     # Custom 50MB memory pool with 4KB page allocation
├── VFS.h                 # VirtualFile struct with shared_mutex & access tracking
├── Swapper.h             # LRU-based page-out/page-in (virtual memory simulation)
├── DeadlockDetector.h    # Background daemon — Wait-For Graph with DFS cycle detection
├── .gitignore            # Build artifacts & swap file exclusions
├── LICENSE               # MIT License
└── README.md             # This file
```

---

## Core Features ⚙️

### 🧠 Custom Memory Allocator (`MemoryAllocator.h`)
- Pre-allocates a **50 MB** contiguous memory pool at startup
- Divides it into **12,800 pages** of **4 KB** each
- Thread-safe allocation and deallocation using `std::mutex`
- Completely bypasses standard OS `malloc` — simulates OS-level memory management

### 💾 Virtual Memory Swapper (`Swapper.h`)
- When all RAM pages are exhausted, the **Least Recently Used (LRU)** file is evicted to disk (`swap.sys`)
- On access to a swapped-out file, a **Page Fault** is triggered and the file is paged back into RAM
- Simulates real OS paging behavior with transparent page-in/page-out

### 🔒 Reader-Writer Synchronization (`VFS.h`)
- Each file has its own `std::shared_mutex` (Reader-Writer lock)
- **Multiple threads** can read the same file simultaneously (shared lock)
- **Write access** is exclusive — only one writer at a time, blocking all readers
- Prevents data races and ensures consistency

### 🕵️ Deadlock Detection Daemon (`DeadlockDetector.h`)
- Runs as a **background thread**, scanning every 500ms
- Maintains a **Wait-For Graph** mapping thread dependencies
- Uses **Depth-First Search (DFS)** to detect cycles (deadlocks)
- Alerts and recovers automatically when a deadlock is found

---

## Prerequisites

| Requirement | Minimum Version |
|---|---|
| **C++ Compiler** | C++17 support (GCC 7+, Clang 5+, or MSVC 2017+) |
| **POSIX Threads** | Required for `std::thread` and `std::shared_mutex` |
| **Git** | For cloning the repository |
| **OS** | Linux, macOS, or Windows (with MinGW/WSL) |

---

## How to Build and Run 🛠️

### 1. Clone the repository

```bash
git clone https://github.com/shivavoorkonda/ThreadSafe-VFS.git
cd ThreadSafe-VFS
```

### 2. Compile the code

**Linux / macOS:**
```bash
g++ -std=c++17 -pthread main.cpp -o vfs_simulator
```

**Windows (MinGW):**
```bash
g++ -std=c++17 -pthread main.cpp -o vfs_simulator.exe
```

**Windows (MSVC):**
```bash
cl /std:c++17 /EHsc main.cpp /Fe:vfs_simulator.exe
```

### 3. Run the simulation

```bash
./vfs_simulator
```

---

## System Proof of Work 🚀

Terminal output from a **15,000 file stress test** demonstrating the Swapper, Page Faults, and Reader-Writer Locks working in tandem:

```text
[SYSTEM] Initialized 50MB Custom Memory Pool.
Starting ThreadSafe-VFS...

--- Starting 15,000 File Stress Test (Will take a moment) ---
Successfully created 15,000 files. RAM was exhausted and files were paged to Disk!

--- Spawning Concurrent Readers and Writers ---
[Thread 134197917828672] Reading: ImportantData.txt
[Thread 134197909435968] Reading: ImportantData.txt
[Thread 134197875865152] Reading: ImportantData.txt
[Thread 134197892650560] Reading: ImportantData.txt
[PAGE FAULT] File 'StressFile_5.txt' not in RAM. Fetching from disk...
[Thread 134197528229440] Reading: StressFile_5.txt
[Thread 134197901043264] *** WRITING *** to: ImportantData.txt
[Thread 134197884257856] *** WRITING *** to: ImportantData.txt

System Shutting Down cleanly.
```

**Key observations:**
- ✅ Multiple readers access `ImportantData.txt` concurrently (shared lock)
- ✅ Writers get exclusive access (unique lock)
- ✅ Page Fault triggered for `StressFile_5.txt` — file was swapped to disk and fetched back
- ✅ 15,000 files created — RAM exhausted → LRU eviction kicked in
- ✅ Clean shutdown with all threads joined

---

## Tech Stack

| Technology | Purpose |
|---|---|
| **C++17** | Core language — `shared_mutex`, structured bindings, `chrono` |
| **POSIX Threads** | Multithreading via `std::thread`, `std::mutex`, `std::shared_mutex` |
| **STL Containers** | `unordered_map`, `unordered_set`, `vector` for data structures |
| **File I/O** | `fstream` for swap file operations |

---

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## Author ✍️

**Shiva Voorkonda**

- 🔗 GitHub: [github.com/shivavoorkonda](https://github.com/shivavoorkonda)
- 📧 Email: shivavoorkonda@gmail.com

---

> *Built to demonstrate practical understanding of Operating System internals through hands-on implementation.*
