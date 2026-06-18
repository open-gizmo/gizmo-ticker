<script setup>
import { onMounted } from 'vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { getConfig, getNetworks } from '~/composables/useDeviceApi.js';
import { handleBackendErrors, initializeCryptoCoin, initializeTheme, setCryptoCoin } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const globalStore = useGlobalStore();
const { t } = useI18n();

// Load the initial device data once for the app shell
const loadInitialData = async () => {
    try {
        setBusy(true, { label: t('common.syncingDevice') });

        // Load the networks if needed
        if (!globalStore.value.networksList?.length) {
            const networksData = await getNetworks();
            globalStore.value.networksList = networksData.networks;
        }

        // Load the config if needed
        if (!globalStore.value.configLoaded) {
            // Get the full device config
            const config = await getConfig();

            // Merge with the existing config
            globalStore.value.config = {
                ...globalStore.value.config,
                ...config
            };

            // Mark config as loaded
            globalStore.value.configLoaded = true;
        }

        // Set the crypto coin in the global store
        setCryptoCoin(globalStore.value.config?.cryptoCoin || 'bitcoin');
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('app.loadError'), showDialog: true });
    } finally {
        setBusy(false);
    }
};

// Initialize the theme and preload device data once the app mounts
onMounted(async () => {
    initializeTheme();
    initializeCryptoCoin();
    await loadInitialData();
});
</script>

<template>
    <div>
        <!-- Route content -->
        <NuxtLayout>
            <NuxtPage />
        </NuxtLayout>
    </div>
</template>