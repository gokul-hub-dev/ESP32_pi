#!/bin/bash

# Script to completely remove Arduino and its related files from Raspberry Pi

echo "Starting Arduino removal process..."

# Remove arduino-cli if installed
echo "Removing arduino-cli..."
rm -f ~/bin/arduino-cli
rm -rf ~/.arduino15

# Remove Arduino IDE (if installed via package manager)
echo "Removing Arduino IDE..."
sudo apt remove --purge -y arduino
sudo apt autoremove -y

# Remove Arduino IDE if installed manually (optional)
echo "Removing Arduino from /opt (if installed manually)..."
sudo rm -rf /opt/arduino

# Remove Arduino-related tools from /usr/local/bin
echo "Removing Arduino tools from /usr/local/bin..."
sudo rm -f /usr/local/bin/arduino
sudo rm -f /usr/local/bin/arduino-add-groups
sudo rm -f /usr/local/bin/arduino-builder
sudo rm -f /usr/local/bin/arduino-ctags

# Optionally remove Arduino sketches and libraries
echo "Removing Arduino sketches and libraries..."
rm -rf ~/Arduino
rm -rf ~/Documents/Arduino

# Clean up any remaining configuration directories
echo "Cleaning up configuration files..."
rm -rf ~/.arduino15

# Clean up any remaining dependencies (if installed via apt)
echo "Cleaning up unused dependencies..."
sudo apt autoremove -y

# Final message
echo "Arduino removal complete!"

# Reboot system to finalize changes
echo "Rebooting the system..."
sudo reboot
