FROM debian:trixie

RUN apt update && apt install -y curl iputils-ping qt6-base-dev ssh