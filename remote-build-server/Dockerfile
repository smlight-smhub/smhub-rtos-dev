FROM ghcr.io/esphome/esphome:latest

# Install uv for fast python package installation
RUN pip install uv

# Install the device-builder package from the main branch
# (This provides the headless remote-build-only server capabilities)
RUN uv pip install --system git+https://github.com/smlight-smhub/device-builder.git@main

# Create a volume for persistent configuration (pairing keys)
VOLUME /config

# Expose the Noise-XX peer-link port
EXPOSE 6055

# Run in headless remote-build-only mode
ENTRYPOINT ["esphome-device-builder", "--remote-build-only", "/config"]
