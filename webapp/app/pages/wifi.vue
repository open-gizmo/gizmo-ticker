<script setup>
import { computed } from 'vue';
import WifiFullSignalIcon from '@hugeicons/core-free-icons/WifiFullSignalIcon';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { PageHeader } from '@brumaombra/ui-vintage/page-header';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import WifiNetworkPicker from '~/components/wifi/WifiNetworkPicker.vue';

const { t } = useI18n();
const globalStore = useGlobalStore();

// Network the device is set up for
const currentNetworkLabel = computed(() => {
    return globalStore.value.config.ssid || t('pages.wifi.notConnected');
});

// Page metadata
definePageMeta({
    layout: 'dashboard'
});
</script>

<template>
    <div class="mx-auto w-full max-w-2xl space-y-6">
        <!-- Title -->
        <PageHeader :title="t('pages.wifi.title')" size="medium" />

        <!-- Current network -->
        <ListGroup :title="t('pages.wifi.currentNetworkLabel')">
            <ListRow :label="currentNetworkLabel" :icon="WifiFullSignalIcon" icon-tile />
        </ListGroup>

        <!-- Networks found by the device, and the connection -->
        <WifiNetworkPicker :confirm-title="t('pages.wifi.connectConfirmTitle')" confirm-message-key="pages.wifi.connectConfirmMessage" />
    </div>
</template>