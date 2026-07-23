ARG BUILD_BASE_VERSION=2026.06.1

FROM ghcr.io/esphome/docker-base:debian-${BUILD_BASE_VERSION} AS base

RUN git config --system --add safe.directory "*"

RUN apt update \
    && apt install -y curl \
    && curl -fsSL https://deb.nodesource.com/setup_24.x | bash - \
    && apt install -y \
      protobuf-compiler \
      nodejs \
      vim \
      less \
      git-delta

RUN pip install uv

RUN useradd esphome -m || true

USER esphome
WORKDIR /home/esphome

ENV VIRTUAL_ENV=/home/esphome/.local/esphome-venv
ENV UV_SYSTEM_PYTHON=false
RUN uv venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# Pre-cache Python dependencies from upstream to make bootstrapping instant
RUN curl -fsSL https://raw.githubusercontent.com/smlight-smhub/esphome/dev/requirements.txt -o req1.txt \
    && curl -fsSL https://raw.githubusercontent.com/smlight-smhub/esphome/dev/requirements_dev.txt -o req2.txt \
    && curl -fsSL https://raw.githubusercontent.com/smlight-smhub/esphome/dev/requirements_test.txt -o req3.txt \
    && uv pip install -r req1.txt -r req2.txt -r req3.txt \
    && rm req*.txt

WORKDIR /workspaces
