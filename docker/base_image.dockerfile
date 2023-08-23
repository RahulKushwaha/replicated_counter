FROM ubuntu:23.10 as build

RUN apt update && apt install -y \
  autoconf \
  automake \
  build-essential \
  cmake \
  curl \
  g++ \
  git \
  libtool \
  make \
  pkg-config \
  unzip \
  protobuf-compiler \
  gdb \
  && apt clean

RUN apt install -y curl libssl-dev libcurl4-openssl-dev libboost-all-dev
RUN apt install -y libdouble-conversion3 libdouble-conversion-dev dnf && gcc --version
RUN apt install -y  libatomic1
RUN apt install -y clang
RUN apt install -y pip
RUN apt install -y python3.11
RUN apt install -y protobuf-compiler-grpc protobuf-compiler

RUN pip install --upgrade cmake --break-system-packages