# Use a base image with a compatible OS
FROM ubuntu:20.04

# Install Avahi and other necessary packages
RUN apt-get update && apt-get install -y \
    avahi-daemon \
    avahi-utils \
    dbus \
    && rm -rf /var/lib/apt/lists/*

# Copy Avahi service configuration
COPY mqtt.service /etc/avahi/services/mqtt.service

# Copy the startup script
COPY start-avahi.sh /usr/local/bin/start-avahi.sh

# Make the script executable
RUN chmod +x /usr/local/bin/start-avahi.sh

# Expose the necessary ports
EXPOSE 5353/udp

# Use the script as the entry point
CMD ["/usr/local/bin/start-avahi.sh"]
