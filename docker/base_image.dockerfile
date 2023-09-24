FROM ubuntu:22.04 as build

RUN apt update && apt install -y \
  autoconf \
  automake \
  build-essential \
  clang  \
  cmake \
  curl  \
  dnf \
  g++ \
  gdb \
  git \
  libatomic1  \
  libboost-all-dev \
  libcurl4-openssl-dev \
  libdouble-conversion-dev  \
  libdouble-conversion3  \
  libgoogle-glog-dev \
  librocksdb-dev \
  libssl-dev  \
  libtool \
  make \
  openssl \
  pip \
  pkg-config \
  protobuf-compiler \
  protobuf-compiler-grpc \
  python3.11 \
  unzip \
  && apt clean

RUN pip install --upgrade cmake

# Packages requiring manual build.
# GRPC
WORKDIR /tmp
RUN git clone --recurse-submodules -b v1.58.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc  \
    && cd /tmp/grpc  \
    && mkdir -p cmake/build \
    && cd /tmp/grpc/cmake/build \
    && cmake -DgRPC_INSTALL=ON \
          -DgRPC_BUILD_TESTS=OFF \
          -DCMAKE_INSTALL_PREFIX=/opt/grpc  ../..  \
    && make -j 32  \
    && make install \
    && rm -rf /tmp/*

# ROCKS_DB
RUN git clone -b v8.5.3 --depth 1 --shallow-submodules https://github.com/facebook/rocksdb.git \
    && cd /tmp/rocksdb/ && make static_lib -j 32 && rm -rf /tmp/*

RUN apt install -y -V ca-certificates lsb-release wget  \
    && wget https://apache.jfrog.io/artifactory/arrow/ubuntu/apache-arrow-apt-source-latest-jammy.deb  \
    && apt install -y -V ./apache-arrow-apt-source-latest-jammy.deb  \
    && apt update && apt install -y -V libarrow-dev libarrow-dataset-dev libarrow-acero-dev && apt clean

RUN apt install -y lsb-release wget software-properties-common gnupg && wget https://apt.llvm.org/llvm.sh \
    && chmod +x llvm.sh \
    && ./llvm.sh 16