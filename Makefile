PWD=$(shell pwd)
TAG=$(shell git describe --tags --abbrev=0 | tr -d '\n')

compile:
	arduino-cli compile \
    --fqbn esp32:esp32:esp32 \
    --log-level=info \
    --build-path=$(PWD)/build \
    --build-property compiler.cpp.extra_flags=-DFIRMWARE_VERSION=\"$(TAG)\" \
    .

upload:
	arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --log-level=debug --input-dir=$(PWD)/build

monitor:
	arduino-cli monitor -p /dev/ttyUSB0 --config Baudrate=115200

clean:
	rm -rf ./build
