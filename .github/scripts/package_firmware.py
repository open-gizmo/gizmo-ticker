# Package a firmware build for the browser flasher: copy every flash part into a versioned folder and write the manifest that lists them
import argparse
import json
import shutil
import subprocess
from pathlib import Path


# Read the flash parts of a PlatformIO environment (bootloader, partition table, boot_app0 and the app, with their addresses)
def read_flash_parts(project_dir, environment):
    output = subprocess.run(['pio', 'project', 'metadata', '-e', environment, '--json-output'], cwd=project_dir, check=True, capture_output=True, text=True).stdout
    metadata = json.loads(output)[environment]

    # The extra images come with their addresses, the app goes at the application offset
    parts = [{'path': Path(image['path']), 'offset': int(image['offset'], 16)} for image in metadata['extra']['flash_images']]
    application_path = Path(metadata['prog_path']).with_suffix('.bin')
    parts.append({'path': application_path, 'offset': int(metadata['extra']['application_offset'], 16)})

    # Check every part was built
    for part in parts:
        if not part['path'].is_file():
            raise SystemExit(f'Missing flash part: {part["path"]} (build the firmware first)')

    # Return the parts in flash order
    return sorted(parts, key=lambda part: part['offset'])


# Copy the parts into the versioned folder and write the manifest (in the ESP Web Tools format, with paths relative to it)
def package_firmware(project_dir, environment, name, version, chip_family, output_dir):
    parts = read_flash_parts(project_dir, environment)
    version_dir = output_dir / version
    version_dir.mkdir(parents=True, exist_ok=True)

    # Copy each part under its own file name
    manifest_parts = []
    for part in parts:
        shutil.copy2(part['path'], version_dir / part['path'].name)
        manifest_parts.append({'path': f'{version}/{part["path"].name}', 'offset': part['offset']})

    # Write the manifest of this version (updates keep the saved settings, so nothing is erased)
    manifest = {
        'name': name,
        'version': version,
        'new_install_prompt_erase': False,
        'builds': [{'chipFamily': chip_family, 'parts': manifest_parts}]
    }
    (output_dir / 'manifest.json').write_text(json.dumps(manifest, indent='\t') + '\n')

    # Show what was packaged
    for part in manifest_parts:
        print(f'0x{part["offset"]:06x}  {part["path"]}')


# Read the arguments and package the build
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Package a firmware build for the OpenGizmo browser flasher')
    parser.add_argument('--project-dir', type=Path, required=True, help='Folder of the PlatformIO project')
    parser.add_argument('--environment', default='esp32dev', help='PlatformIO environment that was built')
    parser.add_argument('--name', required=True, help='Project name shown by the flasher')
    parser.add_argument('--version', required=True, help='Release version, also the name of the parts folder')
    parser.add_argument('--chip-family', default='ESP32', help='Chip the firmware is built for (ESP32, ESP32-S3...)')
    parser.add_argument('--output-dir', type=Path, required=True, help='Folder to write the manifest and the parts folder to')
    arguments = parser.parse_args()
    package_firmware(arguments.project_dir, arguments.environment, arguments.name, arguments.version, arguments.chip_family, arguments.output_dir)
