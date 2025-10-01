#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <queue>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

// Thread pool for handling requests efficiently
class ThreadPool {
private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable condition;
  std::atomic<bool> stop_flag{false};

public:
  ThreadPool(size_t threads) {
    for (size_t i = 0; i < threads; ++i) {
      workers.emplace_back([this] {
        for (;;) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock,
                           [this] { return stop_flag || !tasks.empty(); });
            if (stop_flag && tasks.empty())
              return;
            task = std::move(tasks.front());
            tasks.pop();
          }
          task();
        }
      });
    }
  }

  template <class F> void enqueue(F &&f) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      if (stop_flag)
        return;
      tasks.emplace(std::forward<F>(f));
    }
    condition.notify_one();
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop_flag = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
      if (worker.joinable())
        worker.join();
    }
  }
};

class OptimizedHttpServer {
private:
  int server_fd;
  int port;
  struct sockaddr_in address;
  std::unique_ptr<ThreadPool> thread_pool;
  std::atomic<bool> running{false};

  // Pre-compiled response templates for maximum performance
  static constexpr const char *PING_RESPONSE_TEMPLATE =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: 74\r\n"
      "Access-Control-Allow-Origin: *\r\n"
      "Connection: keep-alive\r\n"
      "Cache-Control: no-cache\r\n"
      "\r\n"
      "{\"message\":\"pong\",\"timestamp\":\"%s\",\"success\":true}";

  static constexpr const char *HEALTH_RESPONSE_TEMPLATE =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: 54\r\n"
      "Access-Control-Allow-Origin: *\r\n"
      "Connection: keep-alive\r\n"
      "Cache-Control: no-cache\r\n"
      "\r\n"
      "{\"status\":\"healthy\",\"timestamp\":\"%s\"}";

  static constexpr const char *ROOT_RESPONSE =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: 140\r\n"
      "Access-Control-Allow-Origin: *\r\n"
      "Connection: keep-alive\r\n"
      "Cache-Control: max-age=3600\r\n"
      "\r\n"
      "{\"message\":\"Welcome to the Ping-Pong Server C++ "
      "Optimized!\",\"endpoints\":{\"ping\":\"/ping\",\"health\":\"/health\"}}";

  static constexpr const char *NOT_FOUND_RESPONSE_TEMPLATE =
      "HTTP/1.1 404 Not Found\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: 71\r\n"
      "Access-Control-Allow-Origin: *\r\n"
      "Connection: close\r\n"
      "\r\n"
      "{\"message\":\"Not Found\",\"timestamp\":\"%s\",\"success\":false}";

  // Ultra-fast timestamp generation using stack buffer
  void getCurrentTimestamp(char *buffer, size_t buffer_size) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
              1000;

    struct tm *utc_tm = gmtime(&time_t);
    snprintf(buffer, buffer_size, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             utc_tm->tm_year + 1900, utc_tm->tm_mon + 1, utc_tm->tm_mday,
             utc_tm->tm_hour, utc_tm->tm_min, utc_tm->tm_sec,
             static_cast<int>(ms.count()));
  }

  // Zero-allocation path extraction
  const char *extractPath(const char *request) {
    // Skip method (GET/POST etc.)
    const char *path_start = strchr(request, ' ');
    if (!path_start)
      return nullptr;
    path_start++; // Skip the space

    // Fast path matching using string comparison
    if (strncmp(path_start, "/ping", 5) == 0 &&
        (path_start[5] == ' ' || path_start[5] == '?')) {
      return "/ping";
    } else if (strncmp(path_start, "/health", 7) == 0 &&
               (path_start[7] == ' ' || path_start[7] == '?')) {
      return "/health";
    } else if (strncmp(path_start, "/ ", 2) == 0) {
      return "/";
    }
    return "/404";
  }

  // Optimized request handler with minimal allocations
  void handleRequest(int client_socket) {
    char buffer[8192];
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0) {
      close(client_socket);
      return;
    }

    buffer[bytes_read] = '\0';

    // Fast path extraction
    const char *path = extractPath(buffer);
    if (!path) {
      close(client_socket);
      return;
    }

    char response_buffer[1024];
    char timestamp[32];

    if (strcmp(path, "/ping") == 0) {
      getCurrentTimestamp(timestamp, sizeof(timestamp));
      snprintf(response_buffer, sizeof(response_buffer), PING_RESPONSE_TEMPLATE,
               timestamp);
      send(client_socket, response_buffer, strlen(response_buffer), 0);
    } else if (strcmp(path, "/health") == 0) {
      getCurrentTimestamp(timestamp, sizeof(timestamp));
      snprintf(response_buffer, sizeof(response_buffer),
               HEALTH_RESPONSE_TEMPLATE, timestamp);
      send(client_socket, response_buffer, strlen(response_buffer), 0);
    } else if (strcmp(path, "/") == 0) {
      // Root response is static - no timestamp needed
      send(client_socket, ROOT_RESPONSE, strlen(ROOT_RESPONSE), 0);
    } else {
      getCurrentTimestamp(timestamp, sizeof(timestamp));
      snprintf(response_buffer, sizeof(response_buffer),
               NOT_FOUND_RESPONSE_TEMPLATE, timestamp);
      send(client_socket, response_buffer, strlen(response_buffer), 0);
    }

    close(client_socket);
  }

public:
  OptimizedHttpServer(int port)
      : port(port), server_fd(-1), thread_pool(nullptr) {}

  bool start() {
    // Create optimized thread pool
    size_t worker_count = std::max(4u, std::thread::hardware_concurrency() * 2);
    thread_pool = std::make_unique<ThreadPool>(worker_count);

    std::cout << "🔧 Using " << worker_count
              << " worker threads for optimal performance\n";

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
      std::cerr << "❌ Failed to create socket\n";
      return false;
    }

    // Performance optimizations
    int opt = 1;

    // SO_REUSEADDR - reuse address immediately
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
      std::cerr << "❌ Failed to set SO_REUSEADDR\n";
      return false;
    }

    // SO_REUSEPORT - multiple processes can bind to same port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
      // Not fatal, some systems don't support it
      std::cout << "⚠️  SO_REUSEPORT not supported, continuing...\n";
    }

    // TCP_NODELAY - disable Nagle's algorithm for lower latency
    if (setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt))) {
      std::cout << "⚠️  TCP_NODELAY not set, may have slightly higher latency\n";
    }

    // Configure address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
      std::cerr << "❌ Failed to bind socket to port " << port << "\n";
      return false;
    }

    // Listen with larger backlog for high concurrency
    if (listen(server_fd, 1024) < 0) {
      std::cerr << "❌ Failed to listen on socket\n";
      return false;
    }

    running = true;

    std::cout << "🚀 Optimized Ping-Pong server started successfully!\n";
    std::cout << "📡 Server running on http://localhost:" << port << "\n";
    std::cout << "🏓 Try: http://localhost:" << port << "/ping\n";
    std::cout << "⚡ Performance optimizations enabled:\n";
    std::cout << "   - Thread pool with " << worker_count << " workers\n";
    std::cout << "   - Zero-copy string operations\n";
    std::cout << "   - Pre-compiled response templates\n";
    std::cout << "   - TCP_NODELAY for low latency\n";
    std::cout << "   - Large connection backlog (1024)\n";

    return true;
  }

  void run() {
    while (running) {
      int addrlen = sizeof(address);
      int client_socket =
          accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

      if (client_socket < 0) {
        if (running) {
          std::cerr << "Failed to accept connection\n";
        }
        continue;
      }

      // Enqueue request to thread pool instead of creating new thread
      thread_pool->enqueue(
          [this, client_socket]() { this->handleRequest(client_socket); });
    }
  }

  void stop() {
    running = false;
    if (server_fd != -1) {
      close(server_fd);
      std::cout << "✅ Server stopped gracefully\n";
    }
  }

  ~OptimizedHttpServer() { stop(); }
};

int main() {
  const int PORT = 8000;

  OptimizedHttpServer server(PORT);

  if (!server.start()) {
    std::cerr << "❌ Failed to start server\n";
    return 1;
  }

  std::cout << "Press Ctrl+C to stop the server...\n";

  try {
    server.run();
  } catch (const std::exception &e) {
    std::cerr << "❌ Server error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}