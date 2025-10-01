import asyncio
import aiohttp
import time
from datetime import datetime
from typing import List
import argparse
import sys


class PerformanceStats:
    """Track and calculate performance statistics"""
    
    def __init__(self):
        self.reset()
    
    def reset(self):
        self.total_requests = 0
        self.successful_requests = 0
        self.failed_requests = 0
        self.total_time = 0.0
        self.start_time = None
        self.end_time = None
        self.response_times = []
    
    def start_timing(self):
        self.start_time = time.perf_counter()
    
    def end_timing(self):
        self.end_time = time.perf_counter()
        self.total_time = self.end_time - self.start_time
    
    def add_request(self, response_time: float, success: bool = True):
        self.total_requests += 1
        self.response_times.append(response_time)
        if success:
            self.successful_requests += 1
        else:
            self.failed_requests += 1
    
    def get_stats(self):
        if not self.response_times:
            return {}
        
        sorted_times = sorted(self.response_times)
        count = len(sorted_times)
        
        return {
            'total_requests': self.total_requests,
            'successful_requests': self.successful_requests,
            'failed_requests': self.failed_requests,
            'total_time': self.total_time,
            'requests_per_second': self.successful_requests / self.total_time if self.total_time > 0 else 0,
            'avg_response_time': sum(sorted_times) / count,
            'min_response_time': sorted_times[0],
            'max_response_time': sorted_times[-1],
            'p50_response_time': sorted_times[count // 2],
            'p95_response_time': sorted_times[int(count * 0.95)],
            'p99_response_time': sorted_times[int(count * 0.99)],
        }


async def fetch_single_request(session: aiohttp.ClientSession, url: str, semaphore: asyncio.Semaphore) -> tuple[float, bool]:
    """Make a single HTTP request with connection reuse and proper error handling"""
    async with semaphore:  # Limit concurrent connections
        start_time = time.perf_counter()
        try:
            async with session.get(url, timeout=aiohttp.ClientTimeout(total=5)) as response:
                await response.read()  # Ensure we read the full response
                end_time = time.perf_counter()
                success = response.status == 200
                if not success:
                    print(f"Error: Received status code {response.status}")
                return end_time - start_time, success
        except Exception as e:
            end_time = time.perf_counter()
            print(f"Request failed: {e}")
            return end_time - start_time, False


async def run_benchmark(
    url: str = "http://localhost:8000/ping",
    total_requests: int = 50000,
    concurrent_requests: int = 500,
    connection_pool_size: int = 250,
    show_progress: bool = True
) -> PerformanceStats:
    """Run the main benchmark with optimized async requests"""
    
    print("🚀 Starting optimized HTTP benchmark")
    print(f"📊 Configuration:")
    print(f"   - Target URL: {url}")
    print(f"   - Total requests: {total_requests:,}")
    print(f"   - Concurrent requests: {concurrent_requests}")
    print(f"   - Connection pool size: {connection_pool_size}")
    print("=" * 70)
    
    # Create semaphore to limit concurrent connections
    semaphore = asyncio.Semaphore(concurrent_requests)
    
    # Configure connection pool for optimal performance
    connector = aiohttp.TCPConnector(
        limit=connection_pool_size,  # Total connection pool size
        limit_per_host=connection_pool_size,  # Connections per host
        keepalive_timeout=60,  # Keep connections alive for 60 seconds
        enable_cleanup_closed=True,  # Clean up closed connections
        use_dns_cache=True,  # Cache DNS lookups
    )
    
    # Configure session with optimizations
    timeout = aiohttp.ClientTimeout(total=10, connect=5)
    
    stats = PerformanceStats()
    
    async with aiohttp.ClientSession(
        connector=connector,
        timeout=timeout,
        headers={'Connection': 'keep-alive'},  # Reuse connections
    ) as session:
        
        # Create all request tasks
        tasks = []
        stats.start_timing()
        
        for i in range(total_requests):
            task = fetch_single_request(session, url, semaphore)
            tasks.append(task)
        
        # Execute all requests concurrently with progress tracking
        completed = 0
        batch_size = min(1000, concurrent_requests * 2)  # Process in batches for memory efficiency
        
        for i in range(0, len(tasks), batch_size):
            batch = tasks[i:i + batch_size]
            results = await asyncio.gather(*batch, return_exceptions=True)
            
            for result in results:
                if isinstance(result, Exception):
                    stats.add_request(0.0, False)
                else:
                    response_time, success = result
                    stats.add_request(response_time, success)
                
                completed += 1
                
                if show_progress and completed % max(1, total_requests // 20) == 0:
                    progress = (completed / total_requests) * 100
                    print(f"Progress: {progress:.1f}% ({completed:,}/{total_requests:,})")
        
        stats.end_timing()
    
    return stats


def print_results(stats: PerformanceStats):
    """Print detailed benchmark results"""
    results = stats.get_stats()
    
    print("\n" + "=" * 70)
    print("📈 BENCHMARK RESULTS")
    print("=" * 70)
    
    print(f"🎯 Total Requests:     {results['total_requests']:,}")
    print(f"✅ Successful:         {results['successful_requests']:,}")
    print(f"❌ Failed:             {results['failed_requests']:,}")
    print(f"⏱️  Total Time:         {results['total_time']:.2f} seconds")
    print(f"🚀 Requests/Second:    {results['requests_per_second']:.2f}")
    
    print(f"\n📊 Response Times (milliseconds):")
    print(f"   Average:     {results['avg_response_time'] * 1000:.2f} ms")
    print(f"   Minimum:     {results['min_response_time'] * 1000:.2f} ms")
    print(f"   Maximum:     {results['max_response_time'] * 1000:.2f} ms")
    print(f"   50th %ile:   {results['p50_response_time'] * 1000:.2f} ms")
    print(f"   95th %ile:   {results['p95_response_time'] * 1000:.2f} ms")
    print(f"   99th %ile:   {results['p99_response_time'] * 1000:.2f} ms")
    
    # Performance rating
    rps = results['requests_per_second']
    if rps > 10000:
        rating = "🔥 EXCELLENT"
    elif rps > 5000:
        rating = "⚡ VERY GOOD"
    elif rps > 1000:
        rating = "✨ GOOD"
    elif rps > 500:
        rating = "👍 FAIR"
    else:
        rating = "⚠️  NEEDS IMPROVEMENT"
    
    print(f"\n🏆 Performance Rating: {rating}")
    print("=" * 70)


async def warmup_server(url: str, warmup_requests: int = 100):
    """Warm up the server with a few requests"""
    print(f"🔥 Warming up server with {warmup_requests} requests...")
    
    connector = aiohttp.TCPConnector(limit=20)
    async with aiohttp.ClientSession(connector=connector) as session:
        tasks = []
        semaphore = asyncio.Semaphore(10)  # Lower concurrency for warmup
        
        for _ in range(warmup_requests):
            tasks.append(fetch_single_request(session, url, semaphore))
        
        await asyncio.gather(*tasks, return_exceptions=True)
    
    print("✅ Warmup complete!")


async def main():
    """Main function with command line argument support"""
    parser = argparse.ArgumentParser(description='High-performance HTTP benchmark client')
    parser.add_argument('--url', default='http://localhost:8000/ping', help='Target URL')
    parser.add_argument('--requests', type=int, default=10000, help='Total number of requests')
    parser.add_argument('--concurrency', type=int, default=100, help='Number of concurrent requests')
    parser.add_argument('--pool-size', type=int, default=100, help='Connection pool size')
    parser.add_argument('--no-warmup', action='store_true', help='Skip server warmup')
    parser.add_argument('--no-progress', action='store_true', help='Hide progress output')
    
    args = parser.parse_args()
    
    try:
        # Test server connectivity first
        connector = aiohttp.TCPConnector()
        async with aiohttp.ClientSession(connector=connector) as session:
            async with session.get(args.url, timeout=aiohttp.ClientTimeout(total=5)) as response:
                if response.status != 200:
                    print(f"❌ Server returned status {response.status}")
                    sys.exit(1)
        
        print(f"✅ Server is responding at {args.url}")
        
        # Optional warmup
        if not args.no_warmup:
            await warmup_server(args.url, min(100, args.requests // 10))
        
        # Run the main benchmark
        stats = await run_benchmark(
            url=args.url,
            total_requests=args.requests,
            concurrent_requests=args.concurrency,
            connection_pool_size=args.pool_size,
            show_progress=not args.no_progress
        )
        
        # Print results
        print_results(stats)
        
    except aiohttp.ClientConnectorError:
        print(f"❌ Could not connect to {args.url}")
        print("   Make sure the server is running!")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n⚠️ Benchmark interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    asyncio.run(main())
