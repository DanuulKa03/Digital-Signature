FROM debian:trixie

COPY ./docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

RUN apt update && apt install -y curl iputils-ping qt6-base-dev ssh build-essential libgl1-mesa-dev cmake