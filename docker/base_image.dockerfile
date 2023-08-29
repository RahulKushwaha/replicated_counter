FROM ubuntu:23.10 as build

RUN apt update && apt install -y \
  autoconf \
  automake \
  build-essential \
  clang  \
  cmake \
  curl  \
  curl \
  dnf \
  g++ \
  gdb \
  git \
  libatomic1  \
  libboost-all-dev \
  libcurl4-openssl-dev \
  libdouble-conversion-dev  \
  libdouble-conversion3  \
  libssl-dev  \
  libtool \
  make \
  pip \
  pkg-config \
  protobuf-compiler \
  protobuf-compiler \
  protobuf-compiler-grpc \
  python3.11 \
  unzip \
  && apt clean


RUN pip install --upgrade cmake --break-system-packages

# Packages requiring manual build.
# GRPC
WORKDIR /tmp
RUN git clone --recurse-submodules -b v1.57.0 https://github.com/grpc/grpc

WORKDIR /tmp/grpc
RUN mkdir -p cmake/build
WORKDIR /tmp/grpc/cmake/build
RUN cmake -DgRPC_INSTALL=ON \
          -DgRPC_BUILD_TESTS=OFF \
          -DCMAKE_INSTALL_PREFIX=/opt/grpc  ../..
RUN make -j 8
RUN make install
RUN apt-get install -y librocksdb-dev

# Arrow
WORKDIR /tmp/
RUN git clone --depth 1 https://github.com/apache/arrow.git

WORKDIR /tmp/arrow/cpp
RUN mkdir build
WORKDIR /tmp/arrow/cpp/build
RUN cmake -DCMAKE_INSTALL_PREFIX=/opt/arrow ..
RUN make -j 8
RUN make install

RUN apt install -y libgoogle-glog-dev openssl

# ABSEIL
WORKDIR /tmp
RUN git clone --depth 1 https://github.com/abseil/abseil-cpp.git
RUN mkdir -p /tmp/abseil-cpp/build
WORKDIR /tmp/abseil-cpp/build
RUN cmake -DCMAKE_INSTALL_PREFIX=/opt/abseil ..
RUN make -j 8 && make install