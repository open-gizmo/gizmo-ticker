import { defineComponent, h } from 'vue';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Alert02Icon, Cancel01Icon } from '@hugeicons/core-free-icons';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';

// Create the icon component (to pass as a prop)
export const createIconComponent = icon => defineComponent({
    name: icon?.name || icon?.iconName || 'DialogIcon',
    inheritAttrs: false,
    setup(_, { attrs }) {
        return () => h(HugeiconsIcon, {
            icon,
            color: 'currentColor',
            strokeWidth: attrs.strokeWidth ?? attrs['stroke-width'] ?? 1.8,
            ...attrs
        });
    }
});

// Icon components
const circleAlertIconComponent = createIconComponent(Alert02Icon);
const closeIconComponent = createIconComponent(Cancel01Icon);

// Available theme modes for the webapp
const availableThemes = [
    { id: 'light' },
    { id: 'dark' },
    { id: 'system' }
];

// Available crypto coins for the ticker and matching webapp accent styles
const availableCryptoCoins = [
    { id: 'bitcoin', shortLabel: 'BTC', accentLight: '#f7931a', accentDark: '#f7a23c' },
    { id: 'kaspa', shortLabel: 'KAS', accentLight: '#6fc7ba', accentDark: '#7dd3c7' }
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

// Apply the resolved theme class to the document root
const applyThemeToDocument = theme => {
    const isDarkSystem = window.matchMedia('(prefers-color-scheme: dark)').matches;
    const resolvedTheme = theme === 'system' ? (isDarkSystem ? 'dark' : 'light') : theme;
    document.documentElement.classList.remove('light', 'dark');
    document.documentElement.classList.add(resolvedTheme);
};

// Apply the active crypto coin accent style to the document root
const applyCryptoCoinToDocument = cryptoCoin => {
    document.documentElement.dataset.cryptoCoin = cryptoCoin;
};

// Return the list of supported theme options
export const getThemes = () => {
    return availableThemes;
};

// Return the list of supported crypto coins
export const getCryptoCoins = () => {
    return availableCryptoCoins;
};

// Persist and apply a selected theme
export const setTheme = theme => {
    const globalStore = useGlobalStore();
    globalStore.value.themeMode = theme;

    // Save the preference in localStorage for persistence across sessions
    localStorage.setItem('theme', theme);

    // Apply the theme to the document root to update the UI
    applyThemeToDocument(theme);
};

// Apply a selected crypto coin to the shared store and document
export const setCryptoCoin = cryptoCoin => {
    const globalStore = useGlobalStore();
    const isSupportedCryptoCoin = availableCryptoCoins.some(coin => coin.id === cryptoCoin);
    globalStore.value.config.cryptoCoin = isSupportedCryptoCoin ? cryptoCoin : 'bitcoin';
    applyCryptoCoinToDocument(globalStore.value.config.cryptoCoin);
};

// Initialize the theme from storage and system preference
export const initializeTheme = () => {
    const globalStore = useGlobalStore();

    // Check localStorage for a saved theme preference
    const storedTheme = localStorage.getItem('theme') || 'system';
    globalStore.value.themeMode = storedTheme;
    applyThemeToDocument(storedTheme);

    // Listen for changes in the system theme preference
    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', () => {
        if (globalStore.value.themeMode === 'system') {
            applyThemeToDocument('system');
        }
    });
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
            type: 'Error',
            icon: circleAlertIconComponent,
            title: translate('dialogs.errorTitle'),
            message: resolvedMessage,
            closeText: translate('common.close'),
            closeButtonIcon: closeIconComponent
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