FROM ubuntu as build

RUN mkdir /opt/counter_app
WORKDIR /opt/counter_app
COPY cmake-build-debug/CounterAppServer CounterAppServer
COPY configs/ configs/

RUN chmod a+x CounterAppServer
RUN /opt/counter_app/CounterAppServer