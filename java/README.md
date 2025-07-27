# Java Spring Boot Ping-Pong Server

A simple HTTP ping-pong server implemented using Spring Boot and Java 22.

## Features

- **GET /** - Welcome message with available endpoints
- **GET /ping** - Returns "pong" with timestamp and success status
- **GET /health** - Health check endpoint
- Runs on port 8000
- JSON responses with timestamps
- CORS enabled for all origins

## Requirements

- Java 22 or higher
- Maven 3.6+

## Running the Server

### Using Maven

```bash
# Navigate to the java directory
cd java

# Run the application
mvn spring-boot:run
```

### Using Java directly

```bash
# Navigate to the java directory
cd java

# Compile and package
mvn clean package

# Run the JAR file
java -jar target/pingpong-server-1.0.0.jar
```

## Testing the Server

Once the server is running on port 8000, you can test it using:

```bash
# Test root endpoint
curl http://localhost:8000/

# Test ping endpoint
curl http://localhost:8000/ping

# Test health endpoint
curl http://localhost:8000/health
```

## Expected Responses

### Root (/)
```json
{
  "message": "Welcome to the Ping-Pong Server Java Spring Boot!",
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
  "timestamp": "2025-07-26T10:30:45.123Z",
  "success": true
}
```

### Health (/health)
```json
{
  "status": "healthy",
  "timestamp": "2025-07-26T10:30:45.123Z"
}
```

## Project Structure

```
java/
├── pom.xml
├── src/
│   └── main/
│       ├── java/
│       │   └── com/
│       │       └── example/
│       │           └── pingpong/
│       │               └── PingPongServerApplication.java
│       └── resources/
│           └── application.properties
└── README.md
```
