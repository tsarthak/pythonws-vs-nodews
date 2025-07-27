FROM python:3.12
RUN mkdir /app
WORKDIR /app
COPY . .
RUN python -m pip install --no-cache-dir --upgrade pip uv
RUN uv pip install -n -r pyproject.toml
RUN uv run main.py
