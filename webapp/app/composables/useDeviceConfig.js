import { getConfig } from '~/composables/useDeviceApi.js';
import { setCryptoCoin } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Load the device config into the store, and remember a failure (so the settings page never saves the store defaults)
export const loadDeviceConfig = async () => {
    const globalStore = useGlobalStore();

    try {
        // Get the full device config
        const config = await getConfig();

        // Merge it with the store and mark it as loaded
        globalStore.value.config = {
            ...globalStore.value.config,
            ...config
        };
        globalStore.value.configLoaded = true;
        globalStore.value.configLoadFailed = false;

        // Apply the accent of the coin set on the device
        setCryptoCoin(globalStore.value.config.cryptoCoin || 'bitcoin');
    } catch (error) {
        // Remember the failure, so the settings page offers to try again
        globalStore.value.configLoadFailed = true;
        throw error;
    }
};