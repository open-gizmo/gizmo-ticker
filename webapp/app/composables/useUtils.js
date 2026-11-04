import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Available crypto coins for the ticker and matching webapp accent styles
const availableCryptoCoins = [
    { id: 'bitcoin', name: 'Bitcoin', shortLabel: 'BTC', image: '/crypto/bitcoin.svg', accentLight: '#f7931a', accentDark: '#f7a23c' },
    { id: 'kaspa', name: 'Kaspa', shortLabel: 'KAS', image: '/crypto/kaspa.svg', accentLight: '#6fc7ba', accentDark: '#7dd3c7' }
];

// Apply the active crypto coin accent style to the document root, and its logo to the app config (the layer shows it in the sidebar, the setup page and the error page)
const applyCryptoCoinToDocument = cryptoCoin => {
    document.documentElement.dataset.cryptoCoin = cryptoCoin;
    updateAppConfig({ gizmo: { logo: getCryptoCoinById(cryptoCoin).image } });
};

// Return the list of supported crypto coins
export const getCryptoCoins = () => {
    return availableCryptoCoins;
};

// Return a supported crypto coin by id, with bitcoin as fallback
export const getCryptoCoinById = cryptoCoinId => {
    return availableCryptoCoins.find(coin => coin.id === cryptoCoinId) || availableCryptoCoins[0];
};

// Apply a selected crypto coin to the shared store and document
export const setCryptoCoin = cryptoCoin => {
    const globalStore = useGlobalStore();
    const isSupportedCryptoCoin = availableCryptoCoins.some(coin => coin.id === cryptoCoin);
    globalStore.value.config.cryptoCoin = isSupportedCryptoCoin ? cryptoCoin : 'bitcoin';
    applyCryptoCoinToDocument(globalStore.value.config.cryptoCoin);
};

// Initialize the crypto coin accent state from the current store value
export const initializeCryptoCoin = () => {
    const globalStore = useGlobalStore();
    applyCryptoCoinToDocument(globalStore.value.config.cryptoCoin || 'bitcoin');
};