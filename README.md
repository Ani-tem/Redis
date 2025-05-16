
# Redis Clone in C++

This project is a Redis-like in-memory key-value data store built from scratch in C++. It aims to mimic the core functionality of Redis using custom data structures and a minimal network protocol. It's designed for educational purposes to understand how a high-performance networked database like Redis works under the hood.

## 🚀 Features

- TCP server using raw socket API.
- Support for multiple clients using a thread pool.
- Custom protocol parser.
- In-memory key-value store using:
  - Hash Tables for generic key-value pairs
  - AVL Trees for sorted set operations
  - Linked Lists for list-type commands
  - Min-Heaps for priority queue behavior
- Efficient serialization/deserialization of messages.
- Simple command-line interface over TCP (like real Redis).

## 🧠 What You’ll Learn

This project is built from scratch without external libraries (like Boost or Protobuf) and helps you learn:

- Low-level socket programming (TCP, blocking I/O).
- Event-based concurrency with thread pools.
- How Redis-like commands are parsed and processed.
- Implementation of fundamental data structures (AVL tree, heap, hash table).
- Message framing: handling TCP as a byte stream by building a custom message protocol.
- Differences between binary and human-readable protocols.
- System programming challenges (e.g., partial reads, interrupted syscalls).

## 📁 Project Structure

```
├── server.cpp       # Main server handling TCP connections and command dispatch
├── avl.cpp / avl.h     # AVL Tree for sorted sets (ZSET)
├── hashtable.cpp / h   # Custom implementation of hash map
├── heap.cpp / h        # Min-heap used for priority queues or timeouts
├── list.h              # Doubly linked list
├── thread_pool.cpp / h # Thread pool to handle multiple clients concurrently
├── zset.cpp / zset.h   # Implementation of Redis-style sorted sets
```

## 📦 Setup

### 🔧 Prerequisites

- C++17 compatible compiler
- POSIX-compliant OS (Linux/macOS)
- `make` (optional)

### 🔨 Compilation

You can compile using g++:
```bash
g++ -std=c++17 -pthread 14_server.cpp -o redis_server
```

Or use a `Makefile` if provided.

### ▶️ Running the Server

```bash
./redis_server
```

This will start the TCP server on a default port (e.g., 6379). You can then connect using `telnet` or `netcat`:

```bash
telnet localhost 6379
# or
nc localhost 6379
```

## 🛠 Supported Commands

Example supported commands (depends on your implementation):

```
SET key value
GET key
DEL key
LPUSH key value
RPUSH key value
LPOP key
ZADD set score member
ZRANGE set start stop
```

## 📚 Protocol Explanation

TCP provides a byte stream, but applications expect **messages**. This project implements a simple message protocol:

- Starts with a **fixed-size header** indicating message type and length.
- Followed by **variable-length** payload.
- Uses **length-prefixed** messages to avoid issues with delimiters.

## 🧵 Threading Model

- The server uses a thread pool to handle multiple connections.
- The main thread accepts new connections and assigns them to worker threads.
- Each worker thread processes client requests independently.

## 🧪 Testing the Server

Once compiled and running, you can open multiple `telnet`/`nc` clients to simulate concurrent clients. Try issuing commands like:

```bash
SET name Boss
GET name
LPUSH queue task1
ZADD scores 10 Alice
ZRANGE scores 0 1
```

## 📌 Notes

- TCP socket I/O is **pull-based**: partial reads are possible, so we need `read_full()` and `write_all()` to ensure data is read/written completely.
- Syscalls like `read` and `write` can be interrupted by signals (`EINTR`), which is properly handled in this implementation.
- The code avoids third-party libraries to emphasize low-level understanding.

## 🧭 Roadmap

- [x] TCP socket server
- [x] Custom hash table
- [x] Sorted sets (AVL tree)
- [x] Lists (Linked list)
- [x] Thread pool
- [ ] Persistence (RDB/AOF)
- [ ] Pub/Sub
- [ ] Cluster support

## 👨‍💻 Author

Made with sweat and sockets by [Ani-tem](https://github.com/Ani-tem)

---

## 📃 License

This project is open-source and available under the [MIT License](LICENSE).
