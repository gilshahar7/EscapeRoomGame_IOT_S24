# Escape Room Game: "Chase Across the Stars"

Welcome to "Chase Across the Stars," an exciting escape room game set in the vastness of space. Join the elite crew of the starship Galactic Guardian on a mission to recover a valuable artifact stolen by the notorious thief Zorax. Your skills and wit will be put to the test as you navigate the galaxy, solve puzzles, and outsmart your adversaries.

## Story

You are part of the crew aboard the starship Galactic Guardian, entrusted with the important task of retrieving a stolen artifact. Zorax, a cunning thief, has taken the artifact and hidden it on an abandoned space station. Your mission is to track down Zorax, gain access to the thief's safe, and retrieve the artifact to keep the galaxy safe.

To accomplish this mission, you must navigate the galaxy by aligning your path with specific constellations on a holographic map. These constellations will guide you through safe routes, avoiding dangerous regions of space. However, be careful not to waste fuel or risk overheating the engines by carrying too much.

## Puzzles

### 1. Aligning Constellations

In this puzzle, you will encounter seven different wheels that need to be aligned in the correct position. Each wheel has various symbols or numbers on it, and you must figure out the correct combination to proceed. Use your observation and deduction skills to solve this challenge.

### 2. Managing Fuel

In this puzzle, you will encounter three fuel tanks of different capacities: 3, 5, and 8 units. Your goal is to transfer fuel between the tanks to reach a specific amount. However, there are rules to follow: you can only transfer fuel by filling tanks completely or emptying them entirely. Can you find the right sequence of actions to achieve the desired fuel level?

### 3. Star Chase

In this puzzle, you will be presented with a 3D map of stars, accompanied by blinking lights. Your task is to decipher the secret code hidden within the map and the blinking stars. Pay close attention to the patterns and connections between the stars to crack the code and unlock the thief's safe.

# In Depth Details

## Hardware

| Component | Purpose | Description
| --- | --- | --- |
| 3D printed plastic wheels | Aligning Constellations | Main components of the "Aligning Constellations" puzzle |
| Small magnets | Aligning Constellations | Placed on each wheel. Used to detect the wheels position |
| Reed switches | Aligning Constellations | Fixed behind each wheel to detect if the wheel is positioned correctly |
| Metal Pins | Aligning Constellations | Used to fixate the wheels axis |
| Big arcade button for transferring | Managing Fuel | When pressed, fuel transfers between two connected tanks |
| Cable | Managing Fuel | Connects two fuel tanks |
| Reset button | Managing Fuel | Reset the puzzle
| Diode | Managing Fuel | For direction in cable |
| Aux sockets | Managing Fuel | Sockets for the cable |
| 5v usb powered fairy lights | Star Chase | For the stars that are not blinking |
| Keypad | Star Chase/Game state | Used to enter the passcode in "Star Chase" and for manual start/reset of the system |
| LED NeoPixel strips | All puzzles and connection feedback | <ul><li>Hint LED in "Aligning Constellations"</li><li>Fuel LEDs in "Managing Fuel"</li><li>Blinking stars LEDs in "Star Chase"</li><li>LEDs providing mqtt connection feedback</li></ul> |
| Countdown timer | General | Timer showing time left for player |
| Wooden box | General | Escape room game platform |
| Electronic latches | General | To open doors after each puzzle |
| Hinges | General | For the doors on the box |

## Node-RED Dashboard

The [Node-RED](https://nodered.org/) dashboard is used to monitor and control various aspects of the escape room game. It provides a user-friendly interface for administrators to manage the game state, monitor puzzle progress, and control game settings.
The dashbaord is using MQTT to exchange game state information with the ESP32.

### Features

- **Game State Monitoring**: Displays the current state of the game, including which puzzles have been completed and the remaining time.
- **Puzzle Control**: Allows administrators to manually start, reset, or skip puzzles.

### Accessing the Dashboard

To access the Node-RED dashboard:

1. Ensure that Node-RED is running on your server.
2. Open a web browser and navigate to the Node-RED dashboard URL (e.g., `http://localhost:1880/ui`).

## "Star Chase" Map
<img src="https://github.com/user-attachments/assets/98fc8be6-abc5-46ab-aaf4-b28feed4c9fe" width="500" height="300"/>

A webpage that features 3D model of the LEDs in the "Star Chaser" puzzle.\
The map is implemented using [React](https://react.dev/) and hosted using github pages at [https://gilshahar7.github.io/EscapeRoomGame_IOT_S24/](https://gilshahar7.github.io/EscapeRoomGame_IOT_S24/)

## Scoreboard
<img src="https://github.com/user-attachments/assets/f115bb78-d747-429d-af7d-4774a9a9af24" width="500" height="300"/>

A webpage featuring the players' scores.
Implemented using [React](https://react.dev/), with a [Firebase](https://firebase.google.com/) real-time database as backend and hosted using github pages at [https://gilshahar7.github.io/EscapeRoomGame_IOT_S24/#/scoreboard](https://gilshahar7.github.io/EscapeRoomGame_IOT_S24/#/scoreboard)

# Running the Admin Panel Using Docker #

To run the admin from any machine, you can use Docker.\
This will take care of installations of node-red and the MQTT broker:

1. Install [Docker](https://docs.docker.com/engine/install/)
2. In this directory, run:
```
docker-compose up -d
```
3. import `flows.json` to a node-red flow.
4. Make sure your node-red mqtt config tries to connect to mosquito:1883 instead of localhost:1883.
5. Create a Firebase configuration node using your database URL and API key.
6. Everything should be up and running!

# Repository Layout
* .github: Info related to hosting the stars map.
* docker: Configurations needed to run docker.
* escape_room_game: All the code related to the ESP side of the project, and configurations related to using PlatformIO.
* stars: The "Star Chase" map and scoreboard sub-project using [React](https://react.dev/) and [Vite](https://vitejs.dev/).
* flows.json: Holds the Node-RED admin panel components.
