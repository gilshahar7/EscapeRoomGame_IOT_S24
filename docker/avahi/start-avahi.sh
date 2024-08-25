#!/bin/bash

# Start dbus service
service dbus start

# Start avahi-daemon service
service avahi-daemon start

# Keep the container running
tail -f /dev/null
