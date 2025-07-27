# Python Starlette Ping-Pong Server

A lightweight and high-performance HTTP ping-pong server implemented using Starlette framework in Python. Starlette provides excellent performance while maintaining simplicity.

## 🚀 Features

- **Starlette Framework**: Lightweight, fast ASGI framework
- **Async/Await**: Fully asynchronous for high concurrency
- **CORS Support**: Cross-Origin Resource Sharing enabled
- **UTC Timestamps**: Consistent timezone handling
- **Performance Optimized**: Uses uvloop and httptools for speed
- **JSON Responses**: Clean, structured API responses

## 📡 API Endpoints

- **GET /** - Welcome message with available endpoints
- **GET /ping** - Returns "pong" with timestamp and success status
- **GET /health** - Health check endpoint
- Runs on port 8000
- JSON responses with UTC timestamps
- CORS enabled for all origins

## 🔧 Requirements

- Python 3.12+
- UV package manager (recommended) or pip

## 🏃‍♂️ Running the Server

### Using UV (Recommended)

```bash
# Navigate to the python directory
cd python

# Install dependencies
uv sync

# Run the server
uv run python server.py
```

### Using pip

```bash
# Navigate to the python directory
cd python

# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Run the server
python server.py
```

### Using uvicorn directly

```bash
# Install dependencies first, then:
uvicorn server:app --host 0.0.0.0 --port 8000 --reload
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

# Test 404 handling
curl http://localhost:8000/nonexistent
```

## 📊 Performance Features

### Starlette Advantages
- **Minimal overhead**: Less abstraction than FastAPI
- **Fast routing**: Efficient request routing
- **ASGI compliant**: Modern async standard
- **Memory efficient**: Lower memory footprint

### Runtime Optimizations
- **uvloop**: High-performance event loop (Unix/Linux/macOS)
- **httptools**: Fast HTTP request parsing
- **Async handlers**: Non-blocking request processing
- **Connection pooling**: Efficient connection management

## 📋 Expected Responses

### Root (/)
```json
{
  "message": "Welcome to the Ping-Pong Server Starlette!",
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
  "timestamp": "2025-07-27T10:30:45.123456+00:00",
  "success": true
}
```

### Health (/health)
```json
{
  "status": "healthy",
  "timestamp": "2025-07-27T10:30:45.123456+00:00"
}
```

### 404 Not Found
```json
{
  "message": "Not Found",
  "timestamp": "2025-07-27T10:30:45.123456+00:00",
  "success": false
}
```

## 🐳 Docker Support

The server can be containerized using the existing Dockerfile:

```bash
# Build the image
docker build -t pingpong-python .

# Run the container
docker run -p 8000:8000 pingpong-python
```

## ⚡ Performance Comparison

### Starlette vs FastAPI
- **Latency**: ~10-15% lower latency than FastAPI
- **Memory**: ~20-30% less memory usage
- **Throughput**: ~5-10% higher requests/second
- **Startup**: Faster application startup time

### Benchmarking

```bash
# Using Apache Bench
ab -n 10000 -c 100 http://localhost:8000/ping

# Using wrk (if installed)
wrk -t12 -c400 -d30s http://localhost:8000/ping
```

Expected performance on modern hardware:
- **Requests/sec**: 15k-25k RPS (single worker)
- **Latency p50**: 2-5ms
- **Latency p99**: 10-20ms
- **Memory usage**: 15-25MB

## 🏗️ Project Structure

```
python/
├── pyproject.toml      # Project configuration and dependencies
├── uv.lock            # Locked dependency versions
├── server.py          # Main Starlette application
├── Dockerfile         # Container configuration
└── README.md          # This file
```

## 🔧 Production Configuration

For production deployment, consider:

```python
# In server.py, modify uvicorn.run():
uvicorn.run(
    app,
    host="0.0.0.0",
    port=8000,
    workers=4,  # Increase workers for production
    log_level="warning",  # Reduce log verbosity
    access_log=False,  # Disable access logs for performance
)
```

## 🚀 Deployment Tips

1. **Use multiple workers** in production: `workers=cpu_count()`
2. **Disable debug mode**: Ensure `debug=False` in Starlette app
3. **Use reverse proxy**: nginx or traefik for SSL termination
4. **Monitor performance**: Use APM tools like New Relic or DataDog
5. **Environment variables**: Configure via environment for different stages

## 🔍 Monitoring

The server includes console logging. For structured logging, consider:

```python
import structlog
logger = structlog.get_logger()
```

## 🧪 Testing

For testing the application:

```python
from starlette.testclient import TestClient
from server import app

client = TestClient(app)

def test_ping():
    response = client.get("/ping")
    assert response.status_code == 200
    assert response.json()["message"] == "pong"
```
