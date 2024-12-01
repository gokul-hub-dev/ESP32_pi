#!/bin/bash

#############################################
# ./esp32_setup.sh SKETCH_NAME LED SKETCH_FILE LED/main.cpp COMPILE UPLOAD_BIN SERIAL_MONITOR
#############################################

# Default values for folder and file names
BIN_DIR="$HOME/ESP32_pi/bin"
ESP32_URL="https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json"
PORT="/dev/ttyUSB0"

# Ensure PATH includes the Arduino CLI directory
export PATH=$PATH:$BIN_DIR

# Check if Arduino CLI is installed
if ! command -v arduino-cli &> /dev/null; then
    echo "Arduino CLI not found. Installing..."
    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
    if [[ $? -ne 0 ]]; then
        echo "Error installing Arduino CLI. Exiting."
        exit 1
    fi
    
fi

ASYNC_TCP="n"
ESP_ASYNC_WEB_SERVER="n"
ASYNC_ELEGANT_OTA="n"

if [[ "$ASYNC_TCP" == "y" ]]; then
  echo "Searching for AsyncTCP library..."
  if arduino-cli lib search AsyncTCP; then
    echo "Library found. Proceeding to installation..."
  else
    echo "Failed to search for AsyncTCP library. Check your internet connection or Arduino CLI installation."
    exit 1
  fi
  echo "Installing AsyncTCP library..."
  if arduino-cli lib install "AsyncTCP"; then
      echo "AsyncTCP library installed successfully."
  else
      echo "Failed to install AsyncTCP library."
      exit 1
  fi
  # List all installed libraries
  echo "Listing installed libraries..."
  arduino-cli lib list  
fi
if [[ "$ESP_ASYNC_WEB_SERVER" == "y" ]]; then
  echo "Searching for ESPAsyncWebServer library..."
  if arduino-cli lib search ESPAsyncWebServer; then
    echo "Library found. Proceeding to installation..."
  else
    echo "Failed to search for ESPAsyncWebServer library."
    exit 1
  fi
  echo "Installing ESPAsyncWebServer library..."
  if arduino-cli lib install "ESPAsyncWebServer"; then
      echo "ESPAsyncWebServer library installed successfully."
  else
      echo "Failed to install ESPAsyncWebServer library."
      exit 1
  fi
  # List all installed libraries
  echo "Listing installed libraries..."
  arduino-cli lib list  
fi
if [[ "$ASYNC_ELEGANT_OTA" == "y" ]]; then
  echo "Searching for AsyncElegantOTA library..."
  if arduino-cli lib search AsyncElegantOTA; then
    echo "Library found. Proceeding to installation..."
  else
    echo "Failed to search for AsyncElegantOTA library."
    exit 1
  fi
  echo "Installing AsyncElegantOTA library..."
  if arduino-cli lib install "AsyncElegantOTA"; then
      echo "AsyncElegantOTA library installed successfully."
  else
      echo "Failed to install AsyncElegantOTA library."
      exit 1
  fi
  # List all installed libraries
  echo "Listing installed libraries..."
  arduino-cli lib list  
fi

# Check if ESP32 core is installed
if ! arduino-cli core list | grep -q "esp32:esp32"; then
    echo "Configuring ESP32 support..."
    arduino-cli config init
    arduino-cli config add board_manager.additional_urls $ESP32_URL
    arduino-cli core update-index
    if ! arduino-cli core install esp32:esp32; then
        echo "Error installing ESP32 core. Exiting."
        exit 1
    fi
else
    echo "ESP32 core is already installed."
fi

# Argument Parsing for sketch folder and file name
COMPILE="n"
UPLOAD_BIN="n"
SERIAL_MONITOR="n"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    -f|SKETCH_NAME)
      SKETCH_NAME="$2"
      shift 2
      ;;
    -s|SKETCH_FILE)
      SKETCH_FILE="$2"
      shift 2
      ;;
    COMPILE)
      COMPILE="y"
      shift
      ;;
    UPLOAD_BIN)
      UPLOAD_BIN="y"
      shift
      ;;
    SERIAL_MONITOR)
      SERIAL_MONITOR="y"
      shift
      ;;
    *)
      echo "Unknown argument: $1"
      exit 1
      ;;
  esac
done

# If folder is not specified, use "LED" by default
#SKETCH_NAME=${SKETCH_NAME:-"LED"}  # Default to "LED" folder if not provided
#SKETCH_FILE=${SKETCH_FILE:-"$SKETCH_NAME/main.cpp"}  # Default to main.cpp inside the provided folder

# Create an empty .ino file if not already present (needed by Arduino CLI)
SKETCH_INO="$SKETCH_NAME/$SKETCH_NAME.ino"

echo "=============$SKETCH_NAME"

if [[ ! -f "$SKETCH_INO" ]]; then
  echo "Creating empty .ino file as wrapper..."
  echo "// Arduino Sketch" > "$SKETCH_INO"
  echo "// This file is required for Arduino CLI to compile the project." >> "$SKETCH_INO"
fi

# Check if the provided sketch file exists
if [[ ! -f "$SKETCH_FILE" ]]; then
    echo "Error: Sketch file $SKETCH_FILE not found."
    exit 1
fi

# Compile the sketch (only if COMPILE is passed)
if [[ "$COMPILE1" == "y" ]]; then
  echo "Compiling the sketch from $SKETCH_FILE..."
  if ! arduino-cli compile --fqbn esp32:esp32:esp32 "$SKETCH_NAME"; then
      echo "Error compiling the sketch. Exiting."
      exit 1
  fi
fi

# Ensure output directory exists and has correct permissions
#if [ ! -d "$OUTPUT_DIR" ]; then
#  echo "Creating output directory: $OUTPUT_DIR"
#  mkdir -p "$OUTPUT_DIR"
#  chown pi:pi "$OUTPUT_DIR"
#  chmod 755 "$OUTPUT_DIR"
#else
#  echo "Output directory already exists: $OUTPUT_DIR"
#fi

if [[ "$COMPILE" == "y" ]]; then
    echo "Compiling the sketch from $SKETCH_FILE..."
    # Create an output directory for compiled files    
    # Use the --output-dir flag to specify the output directory
    if ! arduino-cli compile --fqbn esp32:esp32:esp32 "$SKETCH_NAME"; then
        echo "Error compiling the sketch. Exiting."
        exit 1
    fi
fi

# Check if the ESP32 is connected and upload the sketch (only if UPLOAD_BIN is passed)
if [[ "$UPLOAD_BIN" == "y" ]]; then
  if arduino-cli board list | grep -q "$PORT"; then
      echo "Uploading the sketch to ESP32 on port $PORT..."
      if ! arduino-cli upload --fqbn esp32:esp32:esp32 -p $PORT "$SKETCH_NAME"; then
          echo "Error uploading the sketch. Exiting."
          exit 1
      fi
  else
      echo "ESP32 not detected on port $PORT. Please check the connection."
      exit 1
  fi
fi

# Optionally start serial monitor (only if SERIAL_MONITOR is passed)
if [[ "$SERIAL_MONITOR" == "y" ]]; then
  echo "Opening serial monitor on port $PORT..."
  if ! arduino-cli monitor -p $PORT --config baudrate=115200; then
      echo "Error opening serial monitor. Exiting."
      exit 1
  fi
fi

echo "ESP32 Finished..........."
