package com.example.pingpong;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.web.embedded.tomcat.TomcatProtocolHandlerCustomizer;
import org.springframework.cache.annotation.Cacheable;
import org.springframework.cache.annotation.EnableCaching;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.CacheControl;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.time.Duration;
import java.time.Instant;
import java.time.format.DateTimeFormatter;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

@SpringBootApplication
@EnableCaching
public class PingPongServerApplication {
    public static void main(String[] args) {
        // Performance tuning for Spring Boot
        System.setProperty("server.tomcat.threads.max", "400");
        System.setProperty("server.tomcat.threads.min-spare", "50");
        System.setProperty("server.tomcat.accept-count", "1000");
        System.setProperty("server.tomcat.max-connections", "10000");
        System.setProperty("server.compression.enabled", "true");
        System.setProperty("server.compression.mime-types", "application/json,text/html,text/xml,text/plain");

        SpringApplication.run(PingPongServerApplication.class, args);
    }
}

@Configuration
class PerformanceConfiguration implements WebMvcConfigurer {

    @Bean
    public TomcatProtocolHandlerCustomizer<?> protocolHandlerCustomizer() {
        return protocolHandler -> {
            // Use virtual threads if available (Java 21+)
            try {
                protocolHandler.setExecutor(Executors.newVirtualThreadPerTaskExecutor());
            } catch (Exception e) {
                // Fallback to regular thread pool for older Java versions
                protocolHandler.setExecutor(Executors.newCachedThreadPool());
            }
        };
    }

    // ObjectMapper configuration is handled by Spring Boot auto-configuration
    // with application.properties settings
}

@RestController
class OptimizedPingPongController {

    private static final Logger logger = LoggerFactory.getLogger(OptimizedPingPongController.class);

    // Pre-built response objects to avoid object creation overhead
    private static final Map<String, Object> ROOT_RESPONSE;
    private static final Map<String, Object> ENDPOINTS_MAP;

    // Reusable maps for dynamic responses
    private final ThreadLocal<Map<String, Object>> pingResponseCache = ThreadLocal.withInitial(() -> new HashMap<>(4));
    private final ThreadLocal<Map<String, Object>> healthResponseCache = ThreadLocal
            .withInitial(() -> new HashMap<>(3));

    // Optimized timestamp formatter (thread-safe)
    private static final DateTimeFormatter TIMESTAMP_FORMATTER = DateTimeFormatter.ISO_INSTANT;

    static {
        // Pre-build static response to avoid repeated Map creation
        ENDPOINTS_MAP = Map.of(
                "ping", "/ping",
                "health", "/health");

        ROOT_RESPONSE = Map.of(
                "message", "Welcome to the Ping-Pong Server Java Spring Boot Optimized!",
                "endpoints", ENDPOINTS_MAP);
    }

    @GetMapping("/")
    @Cacheable(value = "rootResponse", unless = "#result == null")
    public ResponseEntity<Map<String, Object>> root() {
        if (logger.isDebugEnabled()) {
            logger.debug("Handled root request");
        }

        return ResponseEntity.ok()
                .cacheControl(CacheControl.maxAge(Duration.ofHours(1)))
                .body(ROOT_RESPONSE);
    }

    @GetMapping("/ping")
    public ResponseEntity<Map<String, Object>> ping() {
        if (logger.isDebugEnabled()) {
            logger.debug("Handled ping request");
        }

        // Reuse thread-local map to avoid allocation overhead
        Map<String, Object> response = pingResponseCache.get();
        response.clear();
        response.put("message", "pong");
        response.put("timestamp", getCurrentTimestamp());
        response.put("success", Boolean.TRUE);

        return ResponseEntity.ok()
                .cacheControl(CacheControl.noCache())
                .body(response);
    }

    @GetMapping("/health")
    public ResponseEntity<Map<String, Object>> health() {
        if (logger.isDebugEnabled()) {
            logger.debug("Handled health check request");
        }

        // Reuse thread-local map to avoid allocation overhead
        Map<String, Object> response = healthResponseCache.get();
        response.clear();
        response.put("status", "healthy");
        response.put("timestamp", getCurrentTimestamp());

        return ResponseEntity.ok()
                .cacheControl(CacheControl.noCache())
                .body(response);
    }

    // Optimized timestamp generation
    private String getCurrentTimestamp() {
        return TIMESTAMP_FORMATTER.format(Instant.now());
    }
}
