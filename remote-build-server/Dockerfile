FROM ghcr.io/esphome/esphome:dev

RUN pip install uv

# Overwrite the upstream ESPHome package with the custom SMHUB fork (which contains the sg2000 platform)
RUN uv pip install --system --force-reinstall git+https://github.com/smlight-smhub/esphome.git@dev

RUN uv pip install --system git+https://github.com/smlight-smhub/device-builder.git@main

# Create a volume for persistent configuration (pairing keys)
VOLUME /config

# Expose the Noise-XX peer-link port
EXPOSE 6055

# Run in headless remote-build-only mode
ENTRYPOINT ["esphome-device-builder", "--remote-build-only", "/config"]
