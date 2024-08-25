# Docker #

To run the admin from any machine, you can use Docker.\
This will take care of installations of node-red and the MQTT broker:

1. Install [Docker](https://docs.docker.com/engine/install/)
2. Pull the required images:
```
docker pull yubki/custom-node-red
docker pull yubki/custom-avahi
docker pull yubki/eclipse-mosquitto
```
3. In this directory, run:
```
docker-compose up -d
```
4. Everything should be up and running!