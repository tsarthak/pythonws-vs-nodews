from starlette.applications import Starlette
from starlette.middleware import Middleware
from starlette.middleware.cors import CORSMiddleware
from starlette.responses import JSONResponse
from starlette.routing import Route
from datetime import datetime, timezone


async def ping(request):
    """Handle ping requests - return pong with timestamp"""
    print("Handled ping request")
    response_data = {
        "message": "pong",
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "success": True,
    }
    return JSONResponse(response_data)


async def health(request):
    """Handle health check requests"""
    print("Handled health check request")
    response_data = {
        "status": "healthy",
        "timestamp": datetime.now(timezone.utc).isoformat(),
    }
    return JSONResponse(response_data)


async def root(request):
    """Handle root requests - return welcome message and available endpoints"""
    print("Handled root request")
    response_data = {
        "message": "Welcome to the Ping-Pong Server Starlette!",
        "endpoints": {
            "ping": "/ping",
            "health": "/health",
        },
    }
    return JSONResponse(response_data)


async def not_found(request, exc):
    """Handle 404 requests"""
    print(f"Handled 404 request for path: {request.url.path}")
    response_data = {
        "message": "Not Found",
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "success": False,
    }
    return JSONResponse(response_data, status_code=404)


# Define routes
routes = [
    Route("/", root, methods=["GET"]),
    Route("/ping", ping, methods=["GET"]),
    Route("/health", health, methods=["GET"]),
]

# Define middleware for CORS and performance
middleware = [
    Middleware(
        CORSMiddleware,
        allow_origins=["*"],
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    ),
]

# Create the Starlette application
app = Starlette(
    debug=False,  # Set to False for production performance
    routes=routes,
    middleware=middleware,
    exception_handlers={404: not_found},
)
