import Alert02Icon from '@hugeicons/core-free-icons/Alert02Icon';
import Cancel01Icon from '@hugeicons/core-free-icons/Cancel01Icon';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Available crypto coins for the ticker and matching webapp accent styles
const availableCryptoCoins = [
    { id: 'bitcoin', name: 'Bitcoin', shortLabel: 'BTC', image: '/crypto/bitcoin.svg', accentLight: '#f7931a', accentDark: '#f7a23c' },
    { id: 'kaspa', name: 'Kaspa', shortLabel: 'KAS', image: '/crypto/kaspa.svg', accentLight: '#6fc7ba', accentDark: '#7dd3c7' }
];

// Translate a key through the active i18n instance when available
export const translate = (key, params = {}) => {
    const nuxtApp = useNuxtApp();

    // If the function is available, return the translated string
    if (typeof nuxtApp?.$i18n?.t === 'function') {
        return nuxtApp.$i18n.t(key, params);
    }

    // Otherwise, return the key as a fallback
    return key;
};

// Apply the active crypto coin accent style to the document root
const applyCryptoCoinToDocument = cryptoCoin => {
    document.documentElement.dataset.cryptoCoin = cryptoCoin;
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

// Pause execution for a short delay
export const delay = timeout => {
    return new Promise(resolve => setTimeout(resolve, timeout));
};

// Handle backend errors with a shared message strategy
export const handleBackendErrors = ({ error, defaultMessage = '', showDialog = false }) => {
    const backendErrorPayload = error?.response?._data || error?.data || error?.response?.data || null;
    const backendErrorId = backendErrorPayload?.errorId || '';
    const backendErrorMessage = backendErrorPayload?.errorMessage || '';
    const translationKey = backendErrorId ? `api.backendErrors.${backendErrorId}` : '';
    const backendErrorTranslated = translationKey ? translate(translationKey) : '';
    const runtimeErrorMessage = error instanceof Error ? error.message : '';
    const resolvedMessage = backendErrorTranslated || backendErrorMessage || runtimeErrorMessage || defaultMessage || translate('common.genericError');
    const resolvedLogMessage = backendErrorMessage || runtimeErrorMessage || defaultMessage || translate('common.genericError');

    // Show a user-friendly error dialog if requested
    if (showDialog) {
        showMessageDialog({
            type: 'error',
            icon: Alert02Icon,
            title: translate('dialogs.errorTitle'),
            message: resolvedMessage,
            closeText: translate('common.close'),
            closeButtonIcon: Cancel01Icon
        });
    }

    // If the error contains a status code, rethrow it for upstream handling
    if (error?.statusCode || error?.response?.status) {
        throw error;
    }

    // For other types of errors, throw a generic error with the message
    console.error(resolvedLogMessage, error);
    throw new Error(resolvedMessage);
};