Development environment and building
====================================

Make environment variables
##########################
All of these are optional.

- DEVICE_NAME: Device name, defaults to "BLE Weatherstation"
- HW_REV: Hardware revision number
- MODEL_NUMBER: Hardware model number
- MANUFACTURER_NAME: Hardware manufacturer name
- SERIAL_NUMBER: Hardware serial number

Developing with nix
###################
Requires a nix 3.0 prerelease with flake support.
Set environment variables as needed.

All the below commands require that you're in a `nix develop` shell.

Get the source code
-------------------
.. code-block:: shell

    git clone https://github.com/foldu/ble-weatherstation-proto --recursive

Building
--------
.. code-block:: sh

    make

Building documentation
----------------------
.. code-block:: sh

   make docs

The documentation will be placed into `./docs/build`.

Flashing
--------
.. code-block:: shell

    # this may fail on the first attempt, just run it again
    make flash
