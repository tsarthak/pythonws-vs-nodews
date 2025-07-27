# Rust Actix Web Ping-Pong Server

A high-performance HTTP ping-pong server implemented using Actix Web framework in Rust. This server is optimized for maximum throughput and minimal latency.

## 🚀 Performance Features

- **Actix Web Framework**: One of the fastest web frameworks available
- **Multi-threaded**: Automatically uses all available CPU cores
- **Zero-copy JSON serialization**: Using serde for efficient data handling
- **Connection pooling**: Keep-alive connections for reduced overhead
- **Optimized build configuration**: LTO, panic=abort, and other release optimizations
- **Minimal memory allocations**: Careful string handling to reduce GC pressure

## 📡 API Endpoints

- **GET /** - Welcome message with available endpoints
- **GET /ping** - Returns "pong" with timestamp and success status
- **GET /health** - Health check endpoint
- Runs on port 8000
- JSON responses with UTC timestamps
- CORS enabled for all origins

## 🔧 Requirements

- Rust 1.75+ (2021 edition)
- Cargo package manager

## 🏃‍♂️ Running the Server

### Development Mode

```bash
# Navigate to the rust directory
cd rust

# Run with cargo (includes debug logging)
cargo run
```

### Production Mode (Optimized)

```bash
# Navigate to the rust directory
cd rust

# Build with release optimizations
cargo build --release

# Run the optimized binary
./target/release/pingpong-server
```

### Using Docker

```bash
# Navigate to the rust directory
cd rust

# Build the Docker image
docker build -t pingpong-rust .

# Run the container
docker run -p 8000:8000 pingpong-rust
```

## 🧪 Testing the Server

Once the server is running on port 8000:

```bash
# Test root endpoint
curl http://localhost:8000/

# Test ping endpoint
curl http://localhost:8000/ping

# Test health endpoint
curl http://localhost:8000/health

# Load test with curl (simple)
for i in {1..1000}; do curl -s http://localhost:8000/ping > /dev/null; done
```

## 📊 Performance Benchmarking

For serious benchmarking, use tools like:

```bash
# Install wrk (HTTP benchmarking tool)
# On macOS: brew install wrk
# On Ubuntu: sudo apt-get install wrk

# Benchmark ping endpoint
wrk -t12 -c400 -d30s http://localhost:8000/ping

# Benchmark with Apache Bench
ab -n 10000 -c 100 http://localhost:8000/ping
```

Expected performance (on modern hardware):
- **Requests/sec**: 100k+ RPS
- **Latency p50**: < 1ms
- **Latency p99**: < 10ms
- **Memory usage**: < 10MB

## 📋 Expected Responses

### Root (/)
```json
{
  "message": "Welcome to the Ping-Pong Server Rust Actix!",
  "endpoints": {
    "ping": "/ping",
    "health": "/health"
  }
}
```

### Ping (/ping)
```json
{
  "message": "pong",
  "timestamp": "2025-07-27T10:30:45.123456789Z",
  "success": true
}
```

### Health (/health)
```json
{
  "status": "healthy",
  "timestamp": "2025-07-27T10:30:45.123456789Z"
}
```

## ⚡ Performance Optimizations

### Compile-time Optimizations
- **LTO (Link Time Optimization)**: Enabled for better inlining
- **Single codegen unit**: Better optimization at link time
- **Panic=abort**: Reduces binary size and eliminates unwinding overhead
- **Strip symbols**: Smaller binary size

### Runtime Optimizations
- **Worker threads**: Automatically scales to available CPU cores
- **Connection keep-alive**: Reduces connection establishment overhead
- **Efficient JSON**: Serde with zero-copy deserialization where possible
- **Request timeouts**: Prevents resource exhaustion

### Memory Optimizations
- **Minimal allocations**: String reuse and efficient data structures
- **Stack allocation**: Prefer stack over heap where possible
- **Connection pooling**: Reuse connections to reduce allocation overhead

## 🏗️ Project Structure

```
rust/
├── Cargo.toml          # Dependencies and build configuration
├── Cargo.lock          # Locked dependency versions
├── main.rs             # Main server implementation
├── Dockerfile          # Multi-stage Docker build
└── README.md           # This file
```

## 🔍 Monitoring

The server includes structured logging. Set log level with:

```bash
# Debug level (very verbose)
RUST_LOG=debug cargo run

# Info level (default)
RUST_LOG=info cargo run

# Error level only
RUST_LOG=error cargo run
```

## 🚀 Deployment Tips

1. **Always use release builds** in production: `cargo build --release`
2. **Set appropriate worker count** based on your CPU cores
3. **Use a reverse proxy** (nginx/traefik) for SSL termination
4. **Monitor memory usage** and adjust as needed
5. **Use Docker** for consistent deployments across environments
