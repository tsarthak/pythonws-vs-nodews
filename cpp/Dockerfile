FROM gcc:latest
RUN mkdir -p /app
WORKDIR /app
COPY . .
RUN g++ -std=c++17 -pthread -o server main.cc
