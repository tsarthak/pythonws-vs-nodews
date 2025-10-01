FROM python:3.12-slim
RUN mkdir -p /app
WORKDIR /app
COPY . .
RUN python -m pip install --upgrade pip uv
RUN uv venv && uv pip install -n -r pyproject.toml
