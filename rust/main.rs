use actix_cors::Cors;
use actix_web::{
    middleware::Logger,
    web, App, HttpResponse, HttpServer, Result as ActixResult,
};
use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

#[derive(Serialize, Deserialize)]
struct PingResponse {
    message: String,
    timestamp: DateTime<Utc>,
    success: bool,
}

#[derive(Serialize, Deserialize)]
struct HealthResponse {
    status: String,
    timestamp: DateTime<Utc>,
}

#[derive(Serialize, Deserialize)]
struct RootResponse {
    message: String,
    endpoints: HashMap<String, String>,
}

// Root endpoint
async fn root() -> ActixResult<HttpResponse> {
    log::info!("Handled root request");
    
    let mut endpoints = HashMap::new();
    endpoints.insert("ping".to_string(), "/ping".to_string());
    endpoints.insert("health".to_string(), "/health".to_string());
    
    let response = RootResponse {
        message: "Welcome to the Ping-Pong Server Rust Actix!".to_string(),
        endpoints,
    };
    
    Ok(HttpResponse::Ok().json(response))
}

// Ping endpoint - optimized for minimal allocations
async fn ping() -> ActixResult<HttpResponse> {
    log::info!("Handled ping request");
    
    let response = PingResponse {
        message: "pong".to_string(),
        timestamp: Utc::now(),
        success: true,
    };
    
    Ok(HttpResponse::Ok().json(response))
}

// Health endpoint
async fn health() -> ActixResult<HttpResponse> {
    log::info!("Handled health check request");
    
    let response = HealthResponse {
        status: "healthy".to_string(),
        timestamp: Utc::now(),
    };
    
    Ok(HttpResponse::Ok().json(response))
}

// 404 handler
async fn not_found() -> ActixResult<HttpResponse> {
    log::warn!("Handled 404 request");
    
    let response = PingResponse {
        message: "Not Found".to_string(),
        timestamp: Utc::now(),
        success: false,
    };
    
    Ok(HttpResponse::NotFound().json(response))
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    // Initialize logger
    env_logger::init_from_env(env_logger::Env::new().default_filter_or("info"));
    
    let port = 8000;
    let bind_address = format!("0.0.0.0:{}", port);
    
    println!("🚀 Ping-Pong server starting...");
    println!("📡 Server will run on http://localhost:{}", port);
    println!("🏓 Try: http://localhost:{}/ping", port);
    println!("🔧 Performance optimizations enabled!");
    
    HttpServer::new(|| {
        // Create CORS middleware
        let cors = Cors::default()
            .allow_any_origin()
            .allow_any_method()
            .allow_any_header()
            .max_age(3600);
        
        App::new()
            .wrap(cors)
            .wrap(Logger::default())
            // Register routes
            .route("/", web::get().to(root))
            .route("/ping", web::get().to(ping))
            .route("/health", web::get().to(health))
            // Default handler for 404
            .default_service(web::route().to(not_found))
    })
    // Performance optimizations
    .workers(4) // Use 4 worker threads
    .bind(&bind_address)?
    .run()
    .await
}