PWD=$(shell pwd)

compile:
	arduino-cli compile --fqbn esp32:esp32:esp32 . --log-level=debug --build-path=$(PWD)/build

upload:
	arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --log-level=debug --input-dir=$(PWD)/build

monitor:
	arduino-cli monitor -p /dev/ttyUSB0 --config Baudrate=115200

clean:
	rm -rf ./build
