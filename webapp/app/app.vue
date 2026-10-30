<script setup>
import { onMounted } from 'vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { useTheme } from '@brumaombra/ui-vintage/theme-selector';
import { loadDeviceConfig } from '~/composables/useDeviceConfig.js';
import { handleBackendErrors, initializeCryptoCoin } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const globalStore = useGlobalStore();
const { t } = useI18n();
const { startTheme } = useTheme();

// Load the device config once for the app shell (the Wi-Fi pages scan the networks themselves, so a failed scan can't block the config)
const loadInitialData = async () => {
    // Nothing to load if the config is already there
    if (globalStore.value.configLoaded) return;

    try {
        setBusy(true, { label: t('common.syncingDevice') });
        await loadDeviceConfig();
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('app.loadError'), showDialog: true });
    } finally {
        setBusy(false);
    }
};

// Apply the saved theme (shared with the theme selector) and preload device data once the app mounts
onMounted(async () => {
    startTheme();
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