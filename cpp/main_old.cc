#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <functional>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <array>

// Thread pool for handling requests
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
                        condition.wait(lock, [this] { return stop_flag || !tasks.empty(); });
                        if (stop_flag && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop_flag) return;
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
            if (worker.joinable()) worker.join();
        }
    }
};

class HttpServer {
private:
    int server_fd;
    int port;
    struct sockaddr_in address;
    std::unique_ptr<ThreadPool> thread_pool;
    std::atomic<bool> running{false};
    
    // Pre-allocated response templates for better performance
    static constexpr const char* PING_RESPONSE_TEMPLATE = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 74\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n"
        "{\"message\":\"pong\",\"timestamp\":\"%s\",\"success\":true}";
    
    static constexpr const char* HEALTH_RESPONSE_TEMPLATE =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 54\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n"
        "{\"status\":\"healthy\",\"timestamp\":\"%s\"}";
    
    static constexpr const char* ROOT_RESPONSE = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 130\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: max-age=3600\r\n"
        "\r\n"
        "{\"message\":\"Welcome to the Ping-Pong Server C++ Optimized!\",\"endpoints\":{\"ping\":\"/ping\",\"health\":\"/health\"}}";
    
    static constexpr const char* NOT_FOUND_RESPONSE_TEMPLATE =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 71\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "{\"message\":\"Not Found\",\"timestamp\":\"%s\",\"success\":false}";

    // Fast timestamp generation (reuse buffer)
    void getCurrentTimestamp(char* buffer, size_t buffer_size) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) % 1000;

        struct tm* utc_tm = gmtime(&time_t);
        snprintf(buffer, buffer_size, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                utc_tm->tm_year + 1900, utc_tm->tm_mon + 1, utc_tm->tm_mday,
                utc_tm->tm_hour, utc_tm->tm_min, utc_tm->tm_sec,
                static_cast<int>(ms.count()));
    }

    // Fast path parsing - only extract what we need
    const char* extractPath(const char* request) {
        // Skip "GET " or "POST " etc.
        const char* path_start = strchr(request, ' ');
        if (!path_start) return nullptr;
        path_start++; // Skip the space
        
        // Find end of path
        const char* path_end = strchr(path_start, ' ');
        if (!path_end) return nullptr;
        
        // For our simple cases, just check the path directly
        if (strncmp(path_start, "/ping", 5) == 0 && (path_start[5] == ' ' || path_start[5] == '?')) {
            return "/ping";
        } else if (strncmp(path_start, "/health", 7) == 0 && (path_start[7] == ' ' || path_start[7] == '?')) {
            return "/health";
        } else if (strncmp(path_start, "/ ", 2) == 0) {
            return "/";
        }
        return "/404";
    }

        char buffer[8192];
        ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_read <= 0) {
            close(client_socket);
            return;
        }

        buffer[bytes_read] = '\0';
        
        // Fast path parsing
        const char* path = extractPath(buffer);
        if (!path) {
            close(client_socket);
            return;
        }

        char response_buffer[1024];
        char timestamp[32];
        getCurrentTimestamp(timestamp, sizeof(timestamp));

        if (strcmp(path, "/ping") == 0) {
            snprintf(response_buffer, sizeof(response_buffer), PING_RESPONSE_TEMPLATE, timestamp);
            send(client_socket, response_buffer, strlen(response_buffer), MSG_NOSIGNAL);
        } else if (strcmp(path, "/health") == 0) {
            snprintf(response_buffer, sizeof(response_buffer), HEALTH_RESPONSE_TEMPLATE, timestamp);
            send(client_socket, response_buffer, strlen(response_buffer), MSG_NOSIGNAL);
        } else if (strcmp(path, "/") == 0) {
            send(client_socket, ROOT_RESPONSE, strlen(ROOT_RESPONSE), MSG_NOSIGNAL);
        } else {
            snprintf(response_buffer, sizeof(response_buffer), NOT_FOUND_RESPONSE_TEMPLATE, timestamp);
            send(client_socket, response_buffer, strlen(response_buffer), MSG_NOSIGNAL);
        }

        close(client_socket);
    }

public:
  HttpServer(int port) : port(port), server_fd(-1) {}

  bool start() {
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
      std::cerr << "❌ Failed to create socket\n";
      return false;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
      std::cerr << "❌ Failed to set socket options\n";
      return false;
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

    // Listen for connections
    if (listen(server_fd, 10) < 0) {
      std::cerr << "❌ Failed to listen on socket\n";
      return false;
    }

    std::cout << "🚀 Ping-Pong server started successfully!\n";
    std::cout << "📡 Server running on http://localhost:" << port << "\n";
    std::cout << "🏓 Try: http://localhost:" << port << "/ping\n";

    return true;
  }

  void run() {
    while (true) {
      int addrlen = sizeof(address);
      int client_socket =
          accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

      if (client_socket < 0) {
        std::cerr << "Failed to accept connection\n";
        continue;
      }

      // Handle request in a separate thread for better performance
      std::thread client_thread(&HttpServer::handleRequest, this,
                                client_socket);
      client_thread.detach();
    }
  }

  void stop() {
    if (server_fd != -1) {
      close(server_fd);
      std::cout << "✅ Server stopped gracefully\n";
    }
  }

  ~HttpServer() { stop(); }
};

int main() {
  const int PORT = 8000;

  HttpServer server(PORT);

  if (!server.start()) {
    std::cerr << "❌ Failed to start server\n";
    return 1;
  }

  // Set up signal handling for graceful shutdown
  std::cout << "Press Ctrl+C to stop the server...\n";

  try {
    server.run();
  } catch (const std::exception &e) {
    std::cerr << "❌ Server error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}