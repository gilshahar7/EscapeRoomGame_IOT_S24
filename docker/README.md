# Docker #

To run the admin from any machine, you can use Docker.\
This will take care of installations of node-red and the MQTT broker:

1. Install [Docker](https://docs.docker.com/engine/install/)
2. In this directory, run:
```
docker-compose up -d
```
3. Make sure your node-red mqtt config tries to connect to mosquito:1883 instead of localhost:1883.
4. Create a Firebase configuration node using your database URL and API key.
5. Everything should be up and running!
