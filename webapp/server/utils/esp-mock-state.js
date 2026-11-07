// The Wi-Fi and device mock (its state, the responses and errors of the firmware) comes from the gizmo-common layer: this file adds the settings and the API key of the ticker
import { getGizmoMockState, restartMockDevice } from 'gizmo-common/web/server/utils/gizmo-mock.js';

// Limit of the firmware (ticker config), counted in bytes like the C strings on the device
export const MAX_API_KEY_LENGTH = 34;

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
    matrixModules: 4,
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

// Get the mock device state: the one of the layer (Wi-Fi, uptime, kept across hot reloads) plus the settings of the ticker
export const getEspMockState = () => {
    const state = getGizmoMockState();
    state.config ??= demoConfig();
    return state;
};

// Reset the mock device like the firmware does: settings and saved network are cleared, and it restarts into its setup hotspot
export const resetEspMockState = () => {
    const state = getEspMockState();
    state.config = factoryConfig();
    state.savedSsid = '';
    restartMockDevice(state);
    return state;
};

// Mask the API key like the firmware: the first and last 3 characters stay visible, short keys are shown as is
export const maskApiKey = (apiKey, visibleEdgeChars = 3) => {
    if (apiKey.length <= visibleEdgeChars * 2) {
        return apiKey;
    }

    return `${apiKey.slice(0, visibleEdgeChars)}${'*'.repeat(apiKey.length - (visibleEdgeChars * 2))}${apiKey.slice(-visibleEdgeChars)}`;
};
