FROM ubuntu as build

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
  && apt clean

RUN apt install -y pip
RUN pip install conan
RUN mkdir -p /opt/LogStorage/replicated_counter/build

COPY docker/conanfile.txt /opt/conan/conanfile.txt

WORKDIR "/opt/conan"
RUN conan profile detect && conan install . --build=missing

RUN apt install -y curl libssl-dev libcurl4-openssl-dev libboost-all-dev
RUN apt install -y libdouble-conversion3 libdouble-conversion-dev dnf && gcc --version
RUN apt install -y  libatomic1

RUN mkdir -p /opt/LogStorage
WORKDIR "/opt/LogStorage"
#RUN git clone --single-branch --branch main https://github.com/RahulKushwaha/replicated_counter.git
COPY / replicated_counter/
WORKDIR "/opt/LogStorage/replicated_counter/build"
RUN apt install -y clang
RUN cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug ..