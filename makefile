srv1_config = configs/srv1_config.textproto
srv2_config = configs/srv2_config.textproto
srv3_config = configs/srv3_config.textproto
srv4_config = configs/srv4_config.textproto
srv5_config = configs/srv5_config.textproto

.PHONY: start_metadata
start_metadata:
	./cmake-build-debug/MetadataServer

.PHONY: start_srv1
start_srv1:
	./cmake-build-debug/CounterAppServer $(srv1_config)

.PHONY: start_srv2
start_srv2:
	cmake-build-debug/CounterAppServer $(srv2_config)

.PHONY: start_srv3
start_srv3:
	cmake-build-debug/CounterAppServer $(srv3_config)

.PHONY: start_srv4
start_srv4:
	cmake-build-debug/CounterAppServer $(srv4_config)

.PHONY: start_srv5
start_srv5:
	cmake-build-debug/CounterAppServer $(srv5_config)