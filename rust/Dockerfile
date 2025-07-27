# Use the official Rust image for building
FROM rust:latest AS builder

# Install dependencies for building
RUN apt-get update && apt-get install -y \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy Cargo files first for better caching
COPY . .

# Build the application with optimizations
RUN cargo build --release

# Use a minimal runtime image
FROM debian:bookworm-slim

# Set working directory
WORKDIR /app

# Copy the binary from builder stage
COPY --from=builder /app/target/release/pingpong-server /app/pingpong-server

# Add health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8000/health || exit 1

# Run the application
CMD ["./pingpong-server"]
