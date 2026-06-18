import { mkdirSync, readdirSync, readFileSync, statSync, writeFileSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { gzipSync } from 'node:zlib';

// Resolve project paths
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const webappRoot = path.resolve(__dirname, '..');
const generatedPublicDir = path.join(webappRoot, '.output', 'public');
const firmwareWebUiDir = path.resolve(webappRoot, '..', 'microcontroller', 'src', 'webui');
const headerPath = path.join(firmwareWebUiDir, 'generated_assets.h');
const sourcePath = path.join(firmwareWebUiDir, 'generated_assets.cpp');

// Match asset extensions to response content types
const contentTypes = new Map([
    ['.html', 'text/html; charset=utf-8'],
    ['.js', 'application/javascript; charset=utf-8'],
    ['.mjs', 'application/javascript; charset=utf-8'],
    ['.css', 'text/css; charset=utf-8'],
    ['.json', 'application/json; charset=utf-8'],
    ['.svg', 'image/svg+xml'],
    ['.png', 'image/png'],
    ['.jpg', 'image/jpeg'],
    ['.jpeg', 'image/jpeg'],
    ['.webp', 'image/webp'],
    ['.ico', 'image/x-icon'],
    ['.txt', 'text/plain; charset=utf-8'],
    ['.woff', 'font/woff'],
    ['.woff2', 'font/woff2']
]);

// Collect every generated file recursively
const walkFiles = directoryPath => {
    const entries = readdirSync(directoryPath, { withFileTypes: true });

    // Recursively flatten the file tree into a list of file paths
    return entries.flatMap(entry => {
        const fullPath = path.join(directoryPath, entry.name);
        if (entry.isDirectory()) return walkFiles(fullPath);
        if (entry.isFile()) return [fullPath];
        return [];
    });
};

// Map generated files back to the routes the ESP should serve
const getRoutePath = relativePath => {
    const normalizedPath = relativePath.split(path.sep).join('/');
    if (normalizedPath === 'index.html') return '/';
    if (normalizedPath.endsWith('/index.html')) return `/${normalizedPath.slice(0, -'/index.html'.length)}`;
    return `/${normalizedPath}`;
};

// Pick the correct content type for each asset
const getContentType = filePath => {
    const extension = path.extname(filePath).toLowerCase();
    return contentTypes.get(extension) ?? 'application/octet-stream';
};

// Convert binary data into a C++ byte array
const toHexLiteralList = buffer => {
    return Array.from(buffer, byte => `0x${byte.toString(16).padStart(2, '0')}`).join(', ');
};

// Escape strings before writing them into C++ source
const escapeCppString = value => {
    return value.replaceAll('\\', '\\\\').replaceAll('"', '\\"');
};

// Export the generated Nuxt site into PROGMEM-ready assets
const generate = () => {
    // Ensure the Nuxt static output exists before proceeding
    if (!statSync(generatedPublicDir, { throwIfNoEntry: false })?.isDirectory()) {
        throw new Error(`Nuxt static output not found at ${generatedPublicDir}. Run \"npm run build\" first.`);
    }

    // Collect all generated files and sort them for consistent symbol generation
    const files = walkFiles(generatedPublicDir).sort((left, right) => left.localeCompare(right));
    mkdirSync(firmwareWebUiDir, { recursive: true });

    // Build the embedded asset manifest
    const assets = files.map((filePath, index) => {
        const relativePath = path.relative(generatedPublicDir, filePath);
        const routePath = getRoutePath(relativePath);
        const contentType = getContentType(filePath);
        const fileContents = readFileSync(filePath);
        const gzippedContents = gzipSync(fileContents, { level: 9 });

        // Return the asset metadata
        return {
            symbolName: `embedded_asset_${index}`,
            sourcePath: relativePath.split(path.sep).join('/'),
            routePath,
            contentType,
            gzippedContents
        };
    });

    // Generate the shared header
    const headerContents = [
        '#pragma once',
        '',
        '#include <Arduino.h>',
        '#include <pgmspace.h>',
        '',
        'struct EmbeddedAsset {',
        '    const char* routePath;',
        '    const char* sourcePath;',
        '    const char* contentType;',
        '    const uint8_t* data;',
        '    size_t size;',
        '    bool gzip;',
        '};',
        '',
        'extern const EmbeddedAsset EMBEDDED_ASSETS[];',
        'extern const size_t EMBEDDED_ASSET_COUNT;'
    ].join('\n');

    // Generate the C++ source with compressed assets
    const sourceSections = [
        '#include "generated_assets.h"',
        '',
        ...assets.flatMap(asset => [
            `static const uint8_t ${asset.symbolName}[] PROGMEM = { ${toHexLiteralList(asset.gzippedContents)} };`,
            ''
        ]),
        'const EmbeddedAsset EMBEDDED_ASSETS[] = {',
        ...assets.map(asset => `    { "${escapeCppString(asset.routePath)}", "${escapeCppString(asset.sourcePath)}", "${escapeCppString(asset.contentType)}", ${asset.symbolName}, sizeof(${asset.symbolName}), true },`),
        '};',
        '',
        'const size_t EMBEDDED_ASSET_COUNT = sizeof(EMBEDDED_ASSETS) / sizeof(EMBEDDED_ASSETS[0]);'
    ].join('\n');

    // Write the generated assets to the firmware source directory
    writeFileSync(headerPath, headerContents, 'utf8');
    writeFileSync(sourcePath, sourceSections, 'utf8');

    // Print a readable summary for the export step
    const manifestSummary = assets.map(asset => `${asset.routePath} <= ${asset.sourcePath}`).join('\n');
    process.stdout.write(`Generated ${assets.length} embedded assets.\n${manifestSummary}\n`);
};

// Run the export process
generate();