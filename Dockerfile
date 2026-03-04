FROM catthehacker/ubuntu:act-20.04

# 避免交互式提示
ENV DEBIAN_FRONTEND=noninteractive

# 安装依赖
RUN add-apt-repository -y ppa:deadsnakes/ppa && \
    apt-get install -y \
        libzmq3-dev \
        libsqlite3-dev \
        gcc-multilib \
        g++-multilib \
        python3.14-dev \
        python3-pip \
        git && \
    # 清理缓存以减小镜像大小
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    rm -rf /tmp/* /var/tmp/*

