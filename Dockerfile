FROM ubuntu:latest AS builder

RUN apt update && apt install -y mingw-w64 git build-essential cmake

FROM builder AS qt

RUN apt install -y qt6-base-dev

CMD bash