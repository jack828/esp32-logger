SHELL:=/bin/bash
PWD=$(shell pwd)
TAG=$(shell git describe --tags --abbrev=0 | tr -d '\n')
PORT=/dev/ttyUSB0
FQBN=esp32:esp32:esp32
FILENAME=esp32-logger
NODE_LIST_FILE="./.nodelist"

compile:
	arduino-cli compile \
		--fqbn $(FQBN) \
		--log-level=info \
		--build-path=$(PWD)/build \
		--build-property 'compiler.cpp.extra_flags="-D FIRMWARE_VERSION="$(TAG)"" "-D CHIP_ID=${node}" -lalgobsec' \
		--build-property 'compiler.warning_level=all' \
		--warnings all \
		.

upload:
	arduino-cli upload \
		-p $(PORT) \
		--fqbn $(FQBN) \
		--log-level=debug \
		--input-dir=$(PWD)/build

monitor:
	arduino-cli monitor \
		-p $(PORT) \
		--config Baudrate=115200

deploy-node:
	@set -e; \
	echo "[deploy] Building for ${node}"; \
		make compile node=${node}; \
	CHIP_ID_HEX=$$(printf "%x" ${node}); \
	FILE=$(PWD)/build/$(FILENAME).ino.bin; \
	MD5=$$(md5sum $$FILE | cut -d' ' -f1); \
	MODE=firmware; \
		curl --verbose --compressed -L -X POST -F "MD5=$$MD5" -F "name=$$MODE" -F "data=@$$FILE;filename=$$MODE" "http://esp$$CHIP_ID_HEX.local/update"; \
	echo "[deploy] Built and uploaded on ${node} - http://esp$$CHIP_ID_HEX.local/";

deploy:
	while read NODE; do \
		make deploy-node node=$$NODE; \
	done < ${NODE_LIST_FILE}

whereis:
	@echo "TODO this should loop over NODES and log ID + location from config"; \
		echo "will need to make api endpoint on the node itself"

# Finds (using avahi) locally available nodes and adds their IDs to .nodelist
discover:
	avahi-browse --resolve _http._tcp --parsable --terminate \
	 | grep wlp \
	 | grep ESP32-LOGGER \
	 | cut -d'=' -f3 \
	 | cut -d'"' -f1 \
	 > ${NODE_LIST_FILE}

clean:
	rm -rf ./build
