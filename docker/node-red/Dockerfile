# Use the official Node-RED image as a base
FROM nodered/node-red:latest

# Install additional Node-RED nodes
# Replace `node-red-node-example` with the names of the nodes you want to install
RUN npm install node-red-dashboard \
    && npm install @gogovega/node-red-contrib-firebase-realtime-database \
    && npm install @gogovega/firebase-config-node

