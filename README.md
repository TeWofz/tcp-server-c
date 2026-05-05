# TCP Server in C

A simple multi-client TCP echo server implemented in C using POSIX sockets.

## Features

- Accepts multiple client connections
- Uses `select()` to handle concurrent clients
- Implements TCP client-server communication
- Handles client disconnects safely
- Uses low-level Unix system calls such as `socket`, `bind`, `listen`, `accept`, `read`, and `write`

## Tech Stack

- C
- POSIX sockets
- TCP/IP
- Unix/Linux

## How to Build

```bash
make
