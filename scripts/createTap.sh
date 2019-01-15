#!/bin/bash

if [[ $(id -u) = 0 ]]; then
  echo "Started as root. Please run as the user who wants to use this."
  exit
fi

while getopts ":e:t:b:i:h" opt; do
  case $opt in
    e) eth_name="$OPTARG"
    ;;
    t) tap_name="$OPTARG"
    ;;
    b) bridge_name="$OPTARG"
    ;;
    i) ip_addr="$OPTARG"
    ;;
    h) echo "Sample usage: ./createTap.sh -e eth0 -t tap0 -b br0 -i 42"
    exit
    ;;
    \?) echo "Invalid option $OPTARG"
  esac
done
username=$(whoami)

printf "Username: $username\n"
printf "Ethernet interface: $eth_name\n"
printf "Tap name: $tap_name\n"
printf "Bridge name: $bridge_name\n"
printf "IpAddr: 192.168.0.$ip_addr/24\n"

echo
read -p "Do you want to continue? [Y/y] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
  echo
  echo
  # Print commands
  (set -o xtrace;
      # Create and configure tap
      sudo ip tuntap add dev "$tap_name" mode tap user $username
      sudo ip link set "$tap_name" up
      sudo ip addr add 192.168.0."$ip_addr"/24 dev "$tap_name"

      # Create bridge
      sudo ip link add name "$bridge_name" type bridge
      sudo ip link set "$bridge_name" up

      # Allow unicast to pass
      sudo brctl setageing "$bridge_name" 0

      # Make sure interface is up to add it to the bridge
      sudo ip link set "$eth_name" up

      # Adding both to bridge
      sudo ip link set "$eth_name" master "$bridge_name"
      sudo ip link set "$tap_name" master "$bridge_name"
  )

  # Enable forwarding
  sudo sysctl -w net.ipv4.conf.all.forwarding=1

  # Check if it worked
  echo
  bridge link
  echo
  echo "$tap_name will be down until something is connected. Then it's turned on automatically."
else
  echo "abort"
fi
