# Multi-stage Dockerfile for AudioSystemBridge
# Builder stage: compile the C++ HTTP server
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential g++ pkg-config git cmake \
    libjack-jackd2-dev liblilv-dev libglib2.0-dev libsndfile1-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy repository into the build image
COPY . /build

# Build the bridge server; uses Makefile at src/bridge/Makefile
WORKDIR /build/AudioSystemBridge/src/bridge
RUN if [ -f Makefile ]; then make; else echo "Makefile not found" && exit 1; fi


# Runtime stage: minimal image with runtime deps
FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libjack-jackd2-0 liblilv-0-0 libglib2.0-0 libsndfile1 ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /build/AudioSystemBridge/src/bridge/audiosystembridge-http /usr/local/bin/audiosystembridge-http

EXPOSE 8080

ENTRYPOINT ["/usr/local/bin/audiosystembridge-http", "8080"]
