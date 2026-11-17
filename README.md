# Sider: a Redis-style in-memory data store in C++

Sider is an in-memory key-value server written in C++ with no third-party
libraries. It speaks a small length-prefixed binary protocol over TCP and
supports strings, sorted sets and key expiry.

The point of the project is to understand how a server like Redis works
underneath: one thread, an event loop, non-blocking sockets, and data
structures chosen so that no single operation stalls every other client.

## How it works

### One thread, one event loop

All client connections are handled by a **single thread** running a `poll()`
event loop. Every socket is non-blocking. On each turn of the loop the server:

1. asks `poll()` which sockets are ready, with a timeout set to the nearest timer;
2. accepts new connections;
3. reads from readable sockets, parses any complete requests, runs them and
   queues the responses;
4. writes queued responses to writable sockets;
5. fires timers: closes idle connections and deletes expired keys.

Because one thread runs every command, the data structures need no locks, and
commands are naturally atomic. This is the same model Redis uses.

Each connection has its own input and output buffer, so a client can send
several requests at once (pipelining) and a slow reader cannot block the others.

### The thread pool does one job

There is a small pool of 4 worker threads, and it does **not** handle clients.
Deleting a sorted set with more than 1000 members means freeing a lot of
memory, which would stall the event loop. Those large deletions are handed to
the pool; small ones are freed inline to avoid the cost of a hand-off.

### Data structures

| Structure | File | Used for |
|---|---|---|
| Hash table with progressive rehashing | `hashtable.cpp` | The keyspace, and member lookup inside sorted sets. Resizing moves a fixed number of entries per operation, so a large table never causes one long pause. |
| AVL tree with subtree counts | `avl.cpp` | Ordering inside sorted sets. The counts make "skip N entries" an O(log N) operation. |
| Sorted set | `zset.cpp` | A hash table plus an AVL tree over the same nodes: O(1) lookup by member, O(log N) range queries by score. |
| Min-heap | `heap.cpp` | Key expiry. The key that expires soonest is always at the top. |
| Intrusive doubly linked list | `list.h` | Idle-connection timers, ordered by last activity. |

### Timers

- **Key expiry:** `pexpire` puts a key in the min-heap. The loop deletes
  expired keys from the top of the heap, at most 2000 per turn so a burst of
  expiries cannot starve clients.
- **Idle connections:** a connection with no activity for 5 seconds is closed.

## Commands

Command names are lower-case.

| Command | Effect | Reply |
|---|---|---|
| `set key value` | Store a string | nil |
| `get key` | Read a string | string, or nil if missing |
| `del key` | Delete a key of any type | 1 if deleted, 0 if missing |
| `keys` | List every key | array of strings |
| `pexpire key ms` | Expire the key after `ms` milliseconds; a negative value removes the expiry | 1 if the key exists, else 0 |
| `pttl key` | Milliseconds until expiry | integer; -1 if no expiry, -2 if the key is missing |
| `zadd zset score member` | Add a member or update its score | 1 if added, 0 if updated |
| `zrem zset member` | Remove a member | 1 if removed, 0 if missing |
| `zscore zset member` | Read a member's score | double, or nil if missing |
| `zquery zset score member offset limit` | Starting from the first entry at or after `(score, member)`, skip `offset` entries and return up to `limit` values | array of alternating member, score |

## Protocol

TCP is a byte stream, so every message carries its own length. All integers
are 4-byte little-endian.

**Request:** a list of strings.

```
| total length | number of strings | len 1 | string 1 | len 2 | string 2 | ...
```

**Response:** a length, then one tagged value.

```
| total length | tag (1 byte) | payload |
```

| Tag | Type | Payload |
|---|---|---|
| 0 | nil | none |
| 1 | error | 4-byte code, 4-byte length, message |
| 2 | string | 4-byte length, bytes |
| 3 | integer | 8-byte signed |
| 4 | double | 8-byte IEEE 754 |
| 5 | array | 4-byte count, then that many tagged values |

Messages are limited to 32 MB. Because the protocol is binary, `telnet` and
`nc` cannot talk to the server; use the client below.

## Build and run

Requires a C++17 compiler on Linux or macOS. The code uses the GNU `typeof`
extension, hence `gnu++17`.

```bash
g++ -std=gnu++17 -O2 -pthread \
    server.cpp avl.cpp hashtable.cpp heap.cpp zset.cpp thread_pool.cpp \
    -o sider
./sider
```

The server listens on port **1234**.

## Try it

`client.py` is a small Python client (standard library only) that sends one
command and prints the decoded reply:

```bash
$ python3 client.py set name anirudh
None
$ python3 client.py get name
anirudh
$ python3 client.py pexpire name 60000
1
$ python3 client.py pttl name
59963
$ python3 client.py zadd board 10 alice
1
$ python3 client.py zadd board 20 bob
1
$ python3 client.py zquery board 0 "" 0 10
['alice', 10.0, 'bob', 20.0]
$ python3 client.py keys
['board', 'name']
```

## Limitations

- No persistence: all data is lost when the server stops.
- Strings and sorted sets only; no lists, hashes or pub/sub.
- No replication or clustering.
- `keys` walks the whole keyspace in one go, so it blocks the loop on a large dataset.

## License

[MIT](LICENSE)
