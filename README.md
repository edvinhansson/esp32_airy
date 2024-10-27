# ESP32 AIRY
An example for how secure HTTPS data can be sent from a sensor, in this case an ESP32 with a DHT11.

## Requirements
- esp-idf
- OpenSSL

## Setup
Since the connection is made using HTTPS, you'll need certificates and keys. `setup_certs.py` can be run to create a CA along with keys and certificates for each component in the chain (ESP32, Node-RED, and Grafana).

You'll need to install Node-RED and Grafana yourself. See below for then setting up HTTPS for each of them:<br>
[Setup HTTPS for Node-RED](https://nodered.org/docs/user-guide/runtime/securing-node-red)<br>
[Setup HTTPS for Grafana](https://grafana.com/docs/grafana/latest/setup-grafana/set-up-https/)

## Build
The program that is ran on the ESP32 is built using esp-idf. Before building you may want to configure where the ESP32 should POST its data to. This can be configured by using `menuconfig` and looking under the `AIRY` tab.

## Configure Wi-Fi
If you're running the program for the first time you'll need to setup the Wi-Fi connection. This can be done using the Espressif SoftAP phone app, and you'll be provided the proof-of-possession string and service name via the serial monitor.