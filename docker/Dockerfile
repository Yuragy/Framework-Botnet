FROM ubuntu:20.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libcurl4-openssl-dev

WORKDIR /app
COPY . .
RUN mkdir build && cd build && cmake .. && make

FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    libssl-dev \
    libcurl4-openssl-dev

WORKDIR /app
COPY --from=builder /app/build/main /app/main
COPY --from=builder /app/build/malware_exe /app/malware.exe

CMD ["./main"]


