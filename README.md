# Sravz Backend CPP Codebase

## Overview

This repository contains the C++ backend for Sravz, designed for high-performance real-time data processing and WebSocket communication. The codebase leverages Boost libraries and modern C++ standards (C++20) for concurrency, networking, and logging.

## Features

- **WebSocket Server**: Multi-threaded WebSocket server for real-time chat and data streaming.
- **Redis Integration**: Subscriber and publisher modules for consuming and publishing messages to Redis topics.
- **Symbol Management**: Tracks and manages financial symbols and their sessions.
- **Configurable**: Uses configuration files for environment-specific settings (hostnames, ports, topics, etc.).
- **Modular Design**: Organized into components such as web server, publisher, subscriber, shared state, and utility functions.
- **Testing**: Integrated with CTest for unit and integration tests.

## Directory Structure

- `src/web/` - WebSocket server, session management, publisher, subscriber, shared state, and symbol logic.
- `src/redis_subscriber.cpp` - Redis subscriber implementation.
- `src/util.cpp` - Utility functions for environment variables and Redis connection options.
- `app.production.cfg` - Example configuration file for production.
- `CMakeLists.txt` - Build configuration using CMake.

## Getting Started

### Prerequisites

- C++20 compatible compiler
- CMake >= 3.24
- Boost libraries
- Redis server

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

### Configuration

Edit `app.production.cfg` to set hostnames, ports, topics, and other environment variables.

### Running

The main entry point is in `src/web/main.cpp`. You can run the server with:

```bash
./bin/sravz_backend_cpp <address> <port> <doc_root> <threads> <topics>
```

Example:

```bash
./bin/sravz_backend_cpp 0.0.0.0 5000 . 5 ETH-USD,BTC-USD
```

## License
Sravz LLC
