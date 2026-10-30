// Limits of the firmware (gizmo-common and the ticker config), counted in bytes like the C strings on the device
export const MAX_REQUEST_BODY_SIZE = 2048;
export const MAX_SSID_LENGTH = 32;
export const MAX_PASSWORD_LENGTH = 64;
export const MAX_API_KEY_LENGTH = 34;
export const MOCK_HOSTNAME = 'ticker.local';
export const MOCK_IP = '192.168.1.42';

// Wi-Fi connection status codes of gizmo-common
export const wifiStatus = Object.freeze({
    DISCONNECTED: 0,
    CONNECTED: 1,
    CONNECTING: 2
});

// Settings of a device right after a reset (the firmware defaults)
const factoryConfig = () => ({
    currentPrice: true,
    priceChange: true,
    marketCap: true,
    dailyHighLow: true,
    yearHighLow: true,
    openPrice: true,
    volume: true,
    cryptoCoin: 'bitcoin',
    formatType: 'US',
    matrixIntensity: 1,
    scrollSpeed: 1,
    apiKey: ''
});

// Settings of the demo device the dev server starts with (already set up)
const demoConfig = () => ({
    ...factoryConfig(),
    marketCap: false,
    yearHighLow: false,
    openPrice: false,
    volume: false,
    matrixIntensity: 9,
    scrollSpeed: 7,
    apiKey: 'coingecko-demo-key-789'
});

// Networks found by the mock scan
const scannedNetworks = () => ([
    { ssid: 'BrumaLab WiFi', signal: -48, quality: 100, channel: 11, secured: true },
    { ssid: 'Office Fiber', signal: -63, quality: 74, channel: 6, secured: true },
    { ssid: 'Warehouse Guest', signal: -71, quality: 58, channel: 1, secured: false },
    { ssid: 'ESP-Test-Hotspot', signal: -83, quality: 34, channel: 3, secured: true }
]);

// State of a device: the demo one on start, a blank one after a reset (no saved network, hotspot open)
const createState = ({ isFactoryReset = false } = {}) => ({
    config: isFactoryReset ? factoryConfig() : demoConfig(),
    savedSsid: isFactoryReset ? '' : 'BrumaLab WiFi',
    hasScanned: false, // The first scan after boot comes back empty, like on the device
    pendingConnection: null,
    isFallbackPending: false, // A failed attempt goes back to the saved network on a later poll
    connectionStatus: isFactoryReset ? wifiStatus.DISCONNECTED : wifiStatus.CONNECTED
});

// Get the mock device state (kept across hot reloads)
export const getEspMockState = () => {
    if (!globalThis.__gizmoEspMockState) {
        globalThis.__gizmoEspMockState = createState();
    }

    return globalThis.__gizmoEspMockState;
};

// Reset the mock device like the firmware does: settings and saved network are cleared
export const resetEspMockState = () => {
    globalThis.__gizmoEspMockState = createState({ isFactoryReset: true });
    return globalThis.__gizmoEspMockState;
};

// Return the networks found by the scan (empty on the first call, while the device is still scanning)
export const scanNetworks = state => {
    if (!state.hasScanned) {
        state.hasScanned = true;
        return [];
    }

    return scannedNetworks();
};

// Mask the API key like the firmware: the first and last 3 characters stay visible, short keys are shown as is
export const maskApiKey = (apiKey, visibleEdgeChars = 3) => {
    if (apiKey.length <= visibleEdgeChars * 2) {
        return apiKey;
    }

    return `${apiKey.slice(0, visibleEdgeChars)}${'*'.repeat(apiKey.length - (visibleEdgeChars * 2))}${apiKey.slice(-visibleEdgeChars)}`;
};

// Length of a string in bytes, like strlen() on the device
export const byteLength = text => Buffer.byteLength(text, 'utf8');

// Success response of the firmware ({ errorId: null, errorMessage: null, data })
export const createSuccessResponse = (data = null) => ({
    errorId: null,
    errorMessage: null,
    data
});

// Error of the mock device, turned into the firmware error response by handleEspMockError
export class EspMockError extends Error {
    constructor(statusCode, errorId, errorMessage) {
        super(errorMessage);
        this.statusCode = statusCode;
        this.errorId = errorId;
    }
}

// Send the firmware error response ({ errorId, errorMessage, data: null }) with its status code
export const handleEspMockError = ({ event, error, errorId, errorMessage }) => {
    const mockError = error instanceof EspMockError ? error : new EspMockError(500, errorId, errorMessage);
    setResponseStatus(event, mockError.statusCode);
    return {
        errorId: mockError.errorId,
        errorMessage: mockError.message,
        data: null
    };
};

// Read the JSON object of a request body with the same checks as the firmware
export const readMockJsonBody = async event => {
    const rawBody = (await readRawBody(event, 'utf8')) || '';

    // The body must fit the firmware buffer
    if (byteLength(rawBody) > MAX_REQUEST_BODY_SIZE) {
        throw new EspMockError(413, 'request_body_too_large', 'Request body too large');
    }

    // The body must be valid JSON
    let body;
    try {
        body = JSON.parse(rawBody);
    } catch {
        throw new EspMockError(400, 'invalid_json', 'Invalid JSON');
    }

    // The body must be an object
    if (!body || typeof body !== 'object' || Array.isArray(body)) {
        throw new EspMockError(400, 'invalid_json', 'Invalid JSON object');
    }

    return body;
};
