<script setup>
import { computed, ref } from 'vue';
import { Cancel01Icon, CheckmarkCircle02Icon, InformationCircleIcon, RefreshIcon, WifiFullSignalIcon } from '@hugeicons/core-free-icons';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Alert, AlertDescription, AlertTitle } from '@brumaombra/ui-vintage/alert';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { Card, CardContent, CardFooter } from '@brumaombra/ui-vintage/card';
import { showConfirmDialog } from '@brumaombra/ui-vintage/confirm-dialog';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { Field, FieldContent, FieldGroup, FieldLabel } from '@brumaombra/ui-vintage/field';
import { Input } from '@brumaombra/ui-vintage/input';
import { Select, SelectContent, SelectItem, SelectItemContent, SelectTrigger, SelectValueContent } from '@brumaombra/ui-vintage/select';
import { connectToWiFi, getNetworks } from '~/composables/useDeviceApi.js';
import { handleBackendErrors } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import PageIntroCard from '~/components/PageIntroCard.vue';
import WifiStatusPanel from '~/components/WifiStatusPanel.vue';

const globalStore = useGlobalStore();
const ssid = ref('');
const password = ref('');
const isLoading = ref(false);
const { t } = useI18n();

// Device WiFi status summary for the card header
const currentNetworkLabel = computed(() => {
    return globalStore.value.config.ssid || t('pages.wifi.notConnected');
});

// Number of scanned networks reported by the ESP
const scannedNetworksCount = computed(() => {
    return Array.isArray(globalStore.value.networksList) ? globalStore.value.networksList.length : 0;
});

// Available network options
const networkOptions = computed(() => {
    const networks = Array.isArray(globalStore.value?.networksList) ? globalStore.value.networksList : [];
    return networks.map(network => ({
        value: network.ssid,
        label: network.ssid,
        meta: `${network.signal} dBm · Ch ${network.channel} · ${network.quality}% · ${network.secured ? t('pages.wifi.secured') : t('pages.wifi.open')}`
    }));
});

// Currently selected network metadata for the select trigger
const selectedNetworkOption = computed(() => {
    return networkOptions.value.find(option => option.value === ssid.value) || null;
});

// Submit WiFi credentials to the device
const connectDeviceToWiFi = async () => {
    try {
        setBusy(true);
        const result = await connectToWiFi(ssid.value, password.value);
        globalStore.value.config.ssid = result.ssid || ssid.value;
        showMessageDialog({
            type: 'Info',
            icon: CheckmarkCircle02Icon,
            title: t('pages.wifi.connectedTitle'),
            message: t('pages.wifi.connectedMessage', { ssid: result.ssid || ssid.value, hostname: result.hostname, ip: result.ip }),
            closeText: t('common.close'),
            closeButtonIcon: Cancel01Icon
        });
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('pages.wifi.connectError'), showDialog: true });
    } finally {
        setBusy(false);
        password.value = '';
    }
};

// Ask for confirmation before replacing the device WiFi connection
const handleConnectPress = () => {
    showConfirmDialog({
        icon: WifiFullSignalIcon,
        title: t('pages.wifi.connectConfirmTitle'),
        message: t('pages.wifi.connectConfirmMessage', { ssid: ssid.value }),
        confirmText: t('pages.wifi.connectAction'),
        cancelText: t('common.cancel'),
        confirmButtonType: 'primary',
        cancelButtonType: 'secondary',
        confirmButtonIcon: WifiFullSignalIcon,
        cancelButtonIcon: Cancel01Icon,
        onConfirm: connectDeviceToWiFi
    });
};

// Refresh the detected SSID list
const refreshSSIDList = async () => {
    isLoading.value = true;

    try {
        const networksData = await getNetworks();
        globalStore.value.networksList = networksData.networks;
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('pages.wifi.refreshError'), showDialog: true });
    } finally {
        isLoading.value = false;
    }
};

// Page metadata
definePageMeta({
    layout: 'dashboard'
});
</script>

<template>
    <div class="mx-auto flex w-full flex-col gap-4 md:gap-6 lg:flex-row">
        <!-- Page intro -->
        <div class="w-full lg:max-w-sm">
            <PageIntroCard :eyebrow="t('pages.wifi.eyebrow')" :title="t('pages.wifi.title')" :description="t('pages.wifi.description')" :icon="WifiFullSignalIcon" :icon-label="t('nav.wifi.label')" />
        </div>

        <!-- Connection form -->
        <div class="min-w-0 flex-1">
            <Card>
                <!-- Card content -->
                <CardContent>
                    <form @submit.prevent="handleConnectPress">
                        <FieldGroup>
                            <!-- Current network summary -->
                            <WifiStatusPanel :current-network-label="currentNetworkLabel"
                                :scanned-networks-count="scannedNetworksCount" />

                            <!-- Network selector -->
                            <Field>
                                <!-- Label -->
                                <FieldLabel for="ssid">{{ t('pages.wifi.availableNetworks') }}</FieldLabel>

                                <!-- Control -->
                                <FieldContent>
                                    <div class="flex items-start gap-2">
                                        <!-- Network select -->
                                        <div class="min-w-0 flex-1">
                                            <Select v-model="ssid">
                                                <SelectTrigger id="ssid" class="w-full">
                                                    <SelectValueContent :placeholder="t('pages.wifi.networkPlaceholder')" :label="selectedNetworkOption?.label" />
                                                </SelectTrigger>
                                                <SelectContent>
                                                    <SelectItem v-for="option in networkOptions" :key="option.value" :value="option.value" :text-value="option.label">
                                                        <SelectItemContent :label="option.label" :description="option.meta" />
                                                    </SelectItem>
                                                </SelectContent>
                                            </Select>
                                        </div>

                                        <!-- Refresh button -->
                                        <Button type="button" variant="secondary" size="icon-lg" :disabled="isLoading" :aria-label="t('common.refresh')" class="shrink-0" @click="refreshSSIDList">
                                            <HugeiconsIcon :icon="RefreshIcon" class="size-4" :class="isLoading ? 'animate-spin' : ''" />
                                        </Button>
                                    </div>
                                </FieldContent>
                            </Field>

                            <!-- Password input -->
                            <Field>
                                <FieldLabel for="password">{{ t('pages.wifi.password') }}</FieldLabel>
                                <FieldContent>
                                    <Input id="password" v-model="password" type="password" :placeholder="t('pages.wifi.passwordPlaceholder')" />
                                </FieldContent>
                            </Field>

                            <!-- Connection note -->
                            <Alert>
                                <HugeiconsIcon :icon="InformationCircleIcon" />
                                <AlertTitle>{{ t('common.note') }}</AlertTitle>
                                <AlertDescription>
                                    {{ t('pages.wifi.note') }}
                                </AlertDescription>
                            </Alert>
                        </FieldGroup>
                    </form>
                </CardContent>

                <!-- Card footer -->
                <CardFooter>
                    <!-- Connect button -->
                    <Button variant="primary" type="button" class="w-full" :disabled="!ssid || !password" @click="handleConnectPress">
                        <HugeiconsIcon :icon="WifiFullSignalIcon" class="size-4" />
                        {{ t('pages.wifi.connectAction') }}
                    </Button>
                </CardFooter>
            </Card>
        </div>
    </div>
</template>