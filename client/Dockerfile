FROM python:3.12-slim

RUN mkdir /app
WORKDIR /app
COPY . .
RUN python -m pip install --no-cache-dir --upgrade pip setuptools wheel uv
RUN uv venv \
    && uv sync --all-groups --refresh \
    && uv run main.py
