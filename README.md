# redis2

A tiny Redis-like server/client built from scratch in C++, mostly to learn
non-blocking socket IO and a couple of core data structures along the way.

## What's here

- `server.cpp` — a single-threaded event loop server using `poll()` over
  non-blocking sockets. Each connection has its own read/write byte buffer;
  requests are parsed off `incoming` and responses appended to `outgoing`.
- `client.cpp` — a one-shot CLI client: connects, sends one command from
  argv, prints the response, exits.
- `hashtable.cpp` / `hashtable.h` — the key/value store. Two hash tables
  (`newer`/`older`) so rehashing can happen incrementally across inserts
  instead of stalling everything on one big rehash.
- `utils.cpp` / `utils.h` — small logging/error helpers shared by both
  binaries.

## Wire protocol

A request is: `[u32 total_len][u32 nstrings][u32 len][bytes] ...`
A response is: `[u32 total_len][u8 tag][tag-specific payload]`, where tag is
one of nil / err / str / int / dbl / arr.

## Commands

`get <key>`, `set <key> <val>`, `del <key>`, `keys`

## Building

```
make            # build server + client
make debug      # same, with -g -O0 for a debugger
make clean
```

## Running

```
./server &
./client set foo bar
./client get foo
./client del foo
```

## Known gaps

- No expiry/TTL.
- No persistence — everything's in memory, gone on restart.
- `poll()` is O(n) in connection count; fine at this scale, wouldn't scale
  as-is to thousands of connections (epoll/kqueue would).
