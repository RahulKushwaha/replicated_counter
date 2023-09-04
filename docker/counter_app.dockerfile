FROM base_os_image as build

RUN mkdir /opt/counter_app
WORKDIR /opt/counter_app
COPY cmake-build-docker .
COPY configs/ configs/

ENTRYPOINT ["./CounterAppServer" , "configs/srv1_config.textproto", "configs/applications/counter/app1_config.textproto"]