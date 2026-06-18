## Embedded Web UI Pipeline

The Nuxt app can now be exported into `PROGMEM`-ready C++ assets for the ESP8266 firmware.

### Commands

From [webapp/package.json](../webapp/package.json):

```bash
npm run build:embedded
```

This does two things:

1. Runs `nuxt generate` with the current root routes.
2. Converts the generated static output into C++ asset files under `microcontroller/src/webui/`.

### Generated Files

The export script writes:

- `microcontroller/src/webui/generated_assets.h`
- `microcontroller/src/webui/generated_assets.cpp`

Each generated asset contains:

- the public route path
- the original source path from the Nuxt static output
- the content type
- gzipped file data stored in `PROGMEM`

### Firmware Integration

- the firmware looks up incoming requests inside `EMBEDDED_ASSETS`
- embedded files are sent from `PROGMEM`
- gzipped assets are returned with `Content-Encoding: gzip`
- JSON endpoints live under `/api`, so page routes and API routes do not overlap anymore

### Deployment Flow

When the web UI changes:

1. run `npm run build:embedded` from `webapp/`
2. rebuild the firmware from `microcontroller/`
3. upload the firmware to the ESP8266

No LittleFS web asset upload step is required for the embedded UI anymore.