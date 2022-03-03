PWD=$(shell pwd)
TAG=$(shell git describe --tags --abbrev=0 | tr -d '\n')
PORT=/dev/ttyUSB0
FQBN=esp32:esp32:esp32

compile:
	arduino-cli compile \
    --fqbn $(FQBN) \
    --log-level=info \
    --build-path=$(PWD)/build \
    --build-property compiler.cpp.extra_flags=-DFIRMWARE_VERSION=\"$(TAG)\" \
    --build-property compiler.warning_level=all \
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

clean:
	rm -rf ./build
