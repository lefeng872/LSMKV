# 使用官方 Ubuntu 作为基础镜像
FROM ubuntu:22.04
CMD bash

# 更新并安装所需的编译工具和库
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    g++ \
    cmake \
    make \
    git \
    clang \
    libc++-dev

COPY mypasswd /tmp

RUN useradd --no-log-init -r -m -g sudo stu

RUN cat /tmp/mypasswd | chpasswd

USER stu

WORKDIR /home/stu/
