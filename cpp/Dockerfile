FROM gcc:13
RUN mkdir -p /app
WORKDIR /app
COPY . .

# Compile with aggressive optimizations for maximum performance
RUN g++ -std=c++17 -pthread -O3 -march=native -mtune=native \
    -ffast-math -funroll-loops -finline-functions \
    -flto -fno-stack-protector -DNDEBUG \
    -o server main.cc

# Strip symbols to reduce binary size
RUN strip server
