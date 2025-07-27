import uvicorn

if __name__ == "__main__":
    print("🚀 Ping-Pong server starting...")
    print("📡 Server will run on http://localhost:8000")
    print("🏓 Try: http://localhost:8000/ping")
    print("⚡ Using Starlette for optimal performance!")
    
    uvicorn.run(
        "server:app",
        host="0.0.0.0",
        port=8000,
        workers=6,
        loop="uvloop",  # Use uvloop for better performance (if available)
        http="httptools",  # Use httptools for faster HTTP parsing
    )
