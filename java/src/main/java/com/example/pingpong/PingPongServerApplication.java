package com.example.pingpong;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.time.Instant;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.util.Map;

@SpringBootApplication
public class PingPongServerApplication {
    public static void main(String[] args) {
        SpringApplication.run(PingPongServerApplication.class, args);
    }
}

@RestController
class PingPongController {
    
    private String getCurrentTimestamp() {
        return Instant.now()
                .atOffset(ZoneOffset.UTC)
                .format(DateTimeFormatter.ISO_INSTANT);
    }
    
    @GetMapping("/")
    public Map<String, Object> root() {
        System.out.println("Handled root request");
        return Map.of(
            "message", "Welcome to the Ping-Pong Server Java Spring Boot!",
            "endpoints", Map.of(
                "ping", "/ping",
                "health", "/health"
            )
        );
    }
    
    @GetMapping("/ping")
    public Map<String, Object> ping() {
        System.out.println("Handled ping request");
        return Map.of(
            "message", "pong",
            "timestamp", getCurrentTimestamp(),
            "success", true
        );
    }
    
    @GetMapping("/health")
    public Map<String, Object> health() {
        System.out.println("Handled health check request");
        return Map.of(
            "status", "healthy",
            "timestamp", getCurrentTimestamp()
        );
    }
}
