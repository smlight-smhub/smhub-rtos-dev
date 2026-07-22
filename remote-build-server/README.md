# SMHUB Remote Build Server

This folder contains a ready-to-use Docker environment that turns any idle computer into a headless C++ compile farm for SMHUB. 

By offloading heavy C++ builds from your low-power Home Assistant machine (like a Raspberry Pi) to a faster computer, you can speed up compile times dramatically and avoid bogging down your smart home!

## 🚀 How to Start the Server

1. Install [Docker](https://docs.docker.com/get-docker/).
2. Open a terminal in this directory and run:
   ```bash
   docker compose up -d
   ```
3. That's it! The server is now running headless in the background.

## 🔐 How to Pair

We are using the **native ESPHome remote build server architecture**, which relies on a highly secure, zero-trust cryptographic peer-link (Noise-XX). Because this server is headless, there is no UI. 

Instead, the server generates a **time-boxed pairing key** and prints it to its logs when it starts up.

1. View the pairing key by checking the Docker logs:
   ```bash
   docker compose logs -f
   ```
2. Look for a line that says: `Pairing key: ...`
3. Open the SMHUB Dashboard on your main development machine.
4. Go to **Settings > Build server > Pairing requests**.
5. Enter the IP address of the computer running this Docker container, port `6055`, and paste the pairing key you found in the logs.
6. The pairing is complete! The cryptographic identity is saved in the local `config/` folder, meaning you will **never have to pair it again** across restarts.

## 🐧 Note for Linux Server Users (mDNS)

If you are running this on a native Linux server (e.g., an Ubuntu NAS), you can optionally uncomment the `network_mode: host` line in the `docker-compose.yml` file and restart the container. 

This allows the container to broadcast its mDNS discovery packets over your LAN, which means it will **automatically pop up** on your SMHUB Dashboard without you needing to type its IP address!

*(Windows and Mac users using Docker Desktop should just leave it commented out and type the IP manually, as Docker Desktop virtualizes the network stack and blocks mDNS broadcasts).*
