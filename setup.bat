@echo off
REM Run this script as administrator
net start mosquitto
dns-sd -R "mqtt" _mqtt._tcp . 1883