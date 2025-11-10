# Base image from the course
FROM --platform=linux/amd64 agodio/itba-so-multi-platform:3.0

# Avoid interactive prompts
ARG DEBIAN_FRONTEND=noninteractive

# 1) Install necessary dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        wget \
        gnupg \
        bear && \
    rm -rf /var/lib/apt/lists/*

# 2) Add official PVS-Studio repo and install
RUN wget -qO- https://files.pvs-studio.com/etc/pubkey.txt \
      | gpg --dearmor -o /etc/apt/trusted.gpg.d/viva64.gpg && \
    wget -O /etc/apt/sources.list.d/viva64.list \
      https://files.pvs-studio.com/etc/viva64.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends pvs-studio && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /root