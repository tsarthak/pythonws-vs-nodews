import requests

from concurrent.futures import ProcessPoolExecutor as PoolExecutor
from datetime import datetime
import os
import time


def fetch_data(iterations):
    print("------------------------------------------------------------------")
    print(f"{os.getpid()} Starting fetch_data with {iterations} iterations")
    start = datetime.now()
    sleep_time = 0  # Simulate a small delay to mimic real-world request handling

    for _ in range(iterations):
        response = requests.get("http://localhost:8000/ping")
        if response.status_code != 200:
            print(f"Error: Received status code {response.status_code}")
            raise RuntimeError("Failed to fetch data from the server")
        # time.sleep(sleep_time)  # Simulate a small delay to mimic real-world request handling

    end = datetime.now()
    elapsed_time = (end - start).total_seconds() - (iterations * sleep_time)
    print(
        f"{os.getpid()} Finished fetch_data with {iterations} iterations. Average requests per second: {iterations / elapsed_time:.2f}"
    )
    print("------------------------------------------------------------------")
    return elapsed_time


def main():
    workers = 2
    iterations = 2000
    print(f"Using {workers} workers with {iterations} iterations each")
    with PoolExecutor(max_workers=workers) as executor:
        total = 0.0
        futures = []

        for _ in range(workers):
            futures.append(executor.submit(fetch_data, iterations=iterations))

        for future in futures:
            result = future.result()
            total += result
        print(
            f"Average requests per second across all workers: {(iterations) / (total / workers):.2f}"
        )


if __name__ == "__main__":
    main()
