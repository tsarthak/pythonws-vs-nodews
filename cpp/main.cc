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

class HttpServer {
private:
  int server_fd;
  int port;
  struct sockaddr_in address;

  std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
              1000;

    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return ss.str();
  }

  std::string createJsonResponse(const std::string &message,
                                 const std::string &timestamp,
                                 bool success = true) {
    std::stringstream json;
    json << "{\n";
    json << "  \"message\": \"" << message << "\",\n";
    json << "  \"timestamp\": \"" << timestamp << "\",\n";
    json << "  \"success\": " << (success ? "true" : "false") << "\n";
    json << "}";
    return json.str();
  }

  std::string
  createHttpResponse(const std::string &body,
                     const std::string &contentType = "application/json",
                     int statusCode = 200) {
    std::string statusText = (statusCode == 200) ? "OK" : "Not Found";

    std::stringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    return response.str();
  }

  std::string parseHttpRequest(const std::string &request, std::string &method,
                               std::string &path) {
    std::istringstream stream(request);
    std::string line;

    if (std::getline(stream, line)) {
      std::istringstream lineStream(line);
      lineStream >> method >> path;
    }

    return path;
  }

  void handleRequest(int client_socket) {
    char buffer[4096] = {0};
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);

    if (bytes_read <= 0) {
      close(client_socket);
      return;
    }

    std::string request(buffer);
    std::string method, path;
    parseHttpRequest(request, method, path);

    std::string response;
    std::string timestamp = getCurrentTimestamp();

    if (path == "/ping") {
      std::string json = createJsonResponse("pong", timestamp);
      response = createHttpResponse(json);
      std::cout << "Handled ping request\n";
    } else if (path == "/health") {
      std::stringstream json;
      json << "{\n";
      json << "  \"status\": \"healthy\",\n";
      json << "  \"timestamp\": \"" << timestamp << "\"\n";
      json << "}";
      response = createHttpResponse(json.str());
      std::cout << "Handled health check request\n";
    } else if (path == "/") {
      std::stringstream json;
      json << "{\n";
      json << "  \"message\": \"Welcome to the Ping-Pong Server C++!\",\n";
      json << "  \"endpoints\": {\n";
      json << "    \"ping\": \"/ping\",\n";
      json << "    \"health\": \"/health\"\n";
      json << "  }\n";
      json << "}";
      response = createHttpResponse(json.str());
      std::cout << "Handled root request" << std::endl;
    } else {
      std::string json = createJsonResponse("Not Found", timestamp, false);
      response = createHttpResponse(json, "application/json", 404);
      std::cout << "Handled 404 request for path: " << path << "\n";
    }

    send(client_socket, response.c_str(), response.length(), 0);
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