from fastapi import FastAPI
from datetime import datetime

app = FastAPI()


@app.get("/ping")
async def ping():
    return {
        "message": "pong",
        "timestamp": datetime.now().isoformat(),
        "success": True,
    }


@app.get("/health")
async def health():
    print("Received health check request")
    response = {
        "status": "healthy",
        "timestamp": datetime.now().isoformat() + "Z",
    }
    print("Sent health check response")
    return response


@app.get("/")
async def root():
    return {
        "message": "Welcome to the Ping-Pong Server!",
        "endpoints": {
            "ping": "/ping",
            "health": "/health",
        },
    }
