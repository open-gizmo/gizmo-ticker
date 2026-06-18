// Shared device and UI state for the webapp
export const useGlobalStore = () => useState('global', () => ({
    // Shared UI theme mode
    themeMode: 'system',

    // Cached WiFi scan results
    networksList: [],

    // Cached full device config payload
    config: {
        currentPrice: false,
        priceChange: false,
        marketCap: false,
        dailyHighLow: false,
        yearHighLow: false,
        openPrice: false,
        volume: false,
        cryptoCoin: 'bitcoin',
        formatType: 'US',
        matrixIntensity: 0,
        scrollSpeed: 0,
        ssid: '',
        apiKey: ''
    },

    // Flag to track whether the config payload has been fetched at least once
    configLoaded: false
}));