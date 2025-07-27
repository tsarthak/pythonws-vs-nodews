import HyperExpress from "hyper-express";

// Create a new HyperExpress server instance
const server = new HyperExpress.Server();

// Define the port
const PORT = process.env.PORT || 8000;

// Ping endpoint - responds with "pong"
server.get("/ping", async (request, response) => {
    try {
        response.json({
            message: "pong",
            timestamp: new Date().toISOString(),
            success: true,
        });
    } catch (error) {
        response.status(500).json({
            error: "Internal server error",
            message: error.message,
        });
    }
});

// Health check endpoint
server.get("/health", async (request, response) => {
    response.json({
        status: "healthy",
        timestamp: new Date().toISOString(),
    });
});

// Root endpoint
server.get("/", async (request, response) => {
    response.json({
        message: "Welcome to the Ping-Pong Server!",
        endpoints: {
            ping: "/ping",
            health: "/health",
        },
    });
});

// Start the server
server
    .listen(PORT)
    .then(() => {
        console.log(`🚀 Ping-Pong server started successfully!`);
        console.log(`📡 Server running on http://localhost:${PORT}`);
        console.log(`🏓 Try: http://localhost:${PORT}/ping`);
    })
    .catch((error) => {
        console.error("❌ Failed to start server:", error);
        process.exit(1);
    });

// Graceful shutdown
const gracefulShutdown = () => {
    console.log("\n🛑 Received shutdown signal, closing server...");
    server
        .close()
        .then(() => {
            console.log("✅ Server closed gracefully");
            process.exit(0);
        })
        .catch((error) => {
            console.error("❌ Error during shutdown:", error);
            process.exit(1);
        });
};

// Handle shutdown signals
process.on("SIGTERM", gracefulShutdown);
process.on("SIGINT", gracefulShutdown);
