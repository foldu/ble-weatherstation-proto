APPLICATION = ble-weatherstation

BOARD ?= arduino-nano-33-ble

RIOTBASE ?= $(CURDIR)/RIOT

# NimBLE bluetooth
USEPKG += nimble
USEMODULE += nimble_svc_gatt nimble_svc_gap bluetil_ad

# timers
USEMODULE += xtimer timex

# sensor
USEMODULE += bme280_i2c

# make this 0 to turn off -Werror for development
WERROR := 1

# Make OBJCOPY non-overridable
OBJCOPY := arm-none-eabi-objcopy

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

ifdef DEVICE_NAME
DEVICE_NAME := "$(DEVICE_NAME)"
else
DEVICE_NAME := "BLE Weatherstation"
endif
GIT_COMMIT := $(shell git rev-parse HEAD)
HW_REV ?= 1
MODEL_NUMBER ?= 1
MANUFACTURER_NAME ?= me
SERIAL_NUMBER ?= 1

CFLAGS += -DHW_REV=\"$(HW_REV)\" \
	  -DMODEL_NUMBER=\"$(MODEL_NUMBER)\" \
	  -DGIT_COMMIT=\"$(GIT_COMMIT)\" \
	  -DMANUFACTURER_NAME=\"$(MANUFACTURER_NAME)\" \
	  -DSERIAL_NUMBER=\"$(SERIAL_NUMBER)\" \
	  -DLOG_UART=1 \
	  -DDEVICE_NAME=\"$(DEVICE_NAME)\"

.PHONY: docs
docs:
	doxygen ./docs/Doxyfile
	sphinx-build ./docs/sphinx ./docs/build/sphinx

include $(RIOTBASE)/Makefile.include
