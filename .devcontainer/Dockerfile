FROM mcr.microsoft.com/devcontainers/base:ubuntu

# Install build tools for C
RUN apt-get update && \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -y --no-install-recommends \
        build-essential \
        gdb \
        clang \
        make \
        cmake \
        git && \
    rm -rf /var/lib/apt/lists/*

USER vscode

WORKDIR /workspaces/detective-quest-OPaidoArthur
