const defaultConfig = () => ({
    currentPrice: true,
    priceChange: true,
    marketCap: false,
    dailyHighLow: true,
    yearHighLow: false,
    openPrice: false,
    volume: false,
    cryptoCoin: 'bitcoin',
    formatType: 'US',
    matrixIntensity: 9,
    scrollSpeed: 7,
    ssid: 'BrumaLab WiFi',
    apiKey: 'coi******************789'
});

const defaultNetworks = () => ([
    { ssid: 'BrumaLab WiFi', signal: -48, quality: 100, channel: 11, secured: true },
    { ssid: 'Office Fiber', signal: -63, quality: 74, channel: 6, secured: true },
    { ssid: 'Warehouse Guest', signal: -71, quality: 58, channel: 1, secured: false },
    { ssid: 'ESP-Test-Hotspot', signal: -83, quality: 34, channel: 3, secured: true }
]);

const createDefaultState = () => ({
    config: defaultConfig(),
    networks: defaultNetworks(),
    pendingConnection: null,
    connectionStatus: 1
});

export const getEspMockState = () => {
    if (!globalThis.__gizmoEspMockState) {
        globalThis.__gizmoEspMockState = createDefaultState();
    }

    return globalThis.__gizmoEspMockState;
};

export const resetEspMockState = () => {
    globalThis.__gizmoEspMockState = createDefaultState();
    return globalThis.__gizmoEspMockState;
};

export const maskApiKey = apiKey => {
    if (!apiKey) {
        return '';
    }

    if (apiKey.length <= 4) {
        return '*'.repeat(apiKey.length);
    }

    const visibleChars = Math.min(3, apiKey.length);
    const maskedLength = Math.max(0, apiKey.length - (visibleChars * 2));
    return `${apiKey.slice(0, visibleChars)}${'*'.repeat(maskedLength)}${apiKey.slice(-visibleChars)}`;
};

export const createSuccessResponse = (data = {}) => ({
    success: true,
    data
});

export const createMockError = (statusCode, errorId, errorMessage) => {
    return createError({
        statusCode,
        statusMessage: errorMessage,
        data: {
            errorId,
            errorMessage
        }
    });
};

export const handleEspMockError = ({ error, errorId, errorMessage }) => {
    if (error?.statusCode) {
        throw error;
    }

    throw createMockError(500, errorId, errorMessage);
};