<script setup>
import { computed, ref } from 'vue';
import { Cancel01Icon, CheckmarkCircle02Icon, InformationCircleIcon, RefreshIcon, WifiFullSignalIcon } from '@hugeicons/core-free-icons';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Alert, AlertDescription, AlertTitle } from '@brumaombra/ui-vintage/alert';
import BackgroundGrid from '@brumaombra/ui-vintage/background-grid';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { Card, CardContent, CardHeader, CardFooter } from '@brumaombra/ui-vintage/card';
import { showConfirmDialog } from '@brumaombra/ui-vintage/confirm-dialog';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { Field, FieldGroup, FieldLabel } from '@brumaombra/ui-vintage/field';
import { Input } from '@brumaombra/ui-vintage/input';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@brumaombra/ui-vintage/select';
import { connectToWiFi, getNetworks } from '~/composables/useDeviceApi.js';
import { createIconComponent, handleBackendErrors } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import BrandLogo from '~/components/BrandLogo.vue';
import CardHeaderWithIcon from '~/components/CardHeaderWithIcon.vue';
import LanguageSelector from '~/components/LanguageSelector.vue';
import ThemeSelector from '~/components/ThemeSelector.vue';

const globalStore = useGlobalStore();
const ssid = ref('');
const password = ref('');
const isLoading = ref(false);
const { t } = useI18n();

// Icon components
const successDialogIcon = createIconComponent(CheckmarkCircle02Icon);
const wifiDialogIcon = createIconComponent(WifiFullSignalIcon);
const closeDialogIcon = createIconComponent(Cancel01Icon);

// Available network options
const networkOptions = computed(() => {
    const networks = Array.isArray(globalStore.value?.networksList) ? globalStore.value.networksList : [];
    return networks.map(network => ({
        value: network.ssid,
        label: network.ssid,
        meta: `${network.signal} dBm · Ch ${network.channel} · ${network.quality}% · ${network.secured ? t('pages.wifi.secured') : t('pages.wifi.open')}`
    }));
});

// Submit WiFi credentials to the device
const connectDeviceToWiFi = async () => {
    try {
        setBusy(true);
        const result = await connectToWiFi(ssid.value, password.value);
        globalStore.value.config.ssid = result.ssid || ssid.value;
        showMessageDialog({
            type: 'Info',
            icon: successDialogIcon,
            title: t('pages.wifi.connectedTitle'),
            message: t('pages.wifi.connectedMessage', { ssid: result.ssid || ssid.value, hostname: result.hostname, ip: result.ip }),
            closeText: t('common.close'),
            closeButtonIcon: closeDialogIcon
        });
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('pages.wifi.connectError'), showDialog: true });
    } finally {
        setBusy(false);
        password.value = '';
    }
};

// Ask for confirmation before disabling AP mode and joining the selected WiFi network
const handleConnectPress = () => {
    showConfirmDialog({
        icon: wifiDialogIcon,
        title: t('pages.apSetup.connectConfirmTitle'),
        message: t('pages.apSetup.connectConfirmMessage', { ssid: ssid.value }),
        confirmText: t('pages.wifi.connectAction'),
        cancelText: t('common.cancel'),
        confirmButtonType: 'default',
        cancelButtonType: 'outline',
        confirmButtonIcon: wifiDialogIcon,
        cancelButtonIcon: closeDialogIcon,
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
</script>

<template>
    <div class="relative min-h-screen bg-[var(--bg-main-light)] text-[var(--text-primary-light)] dark:bg-[var(--bg-main-dark)] dark:text-[var(--text-primary-dark)]">
        <BackgroundGrid />

        <!-- Main content -->
        <main class="relative z-10 mx-auto flex min-h-screen w-full max-w-2xl items-center justify-center px-4 py-10 sm:px-6 lg:px-8">
            <div class="w-full space-y-6">
                <!-- Toolbar -->
                <div class="flex justify-end gap-3">
                    <LanguageSelector />
                    <ThemeSelector />
                </div>

                <!-- Brand -->
                <BrandLogo :brand-name="t('app.title')" />

                <!-- Setup card -->
                <Card>
                    <!-- Header with icon -->
                    <CardHeader>
                        <CardHeaderWithIcon :eyebrow="t('pages.apSetup.eyebrow')"
                            :title="t('pages.apSetup.title')"
                            :icon="WifiFullSignalIcon"
                            :icon-label="t('nav.wifi.label')" />
                    </CardHeader>

                    <!-- Card content -->
                    <CardContent>
                        <form @submit.prevent="handleConnectPress">
                            <FieldGroup>
                                <!-- Network selector -->
                                <Field>
                                    <!-- Network header -->
                                    <div class="flex items-end justify-between gap-3">
                                        <!-- Label -->
                                        <FieldLabel for="ssid">{{ t('pages.wifi.availableNetworks') }}</FieldLabel>

                                        <!-- Refresh button -->
                                        <Button type="button" variant="outline" :disabled="isLoading" @click="refreshSSIDList">
                                            <HugeiconsIcon :icon="RefreshIcon" :stroke-width="1.8" class="size-4" :class="isLoading ? 'animate-spin' : ''" />
                                            <span v-if="isLoading">{{ t('common.refreshing') }}</span>
                                            <span v-else>{{ t('common.refresh') }}</span>
                                        </Button>
                                    </div>

                                    <!-- Network select -->
                                    <Select v-model="ssid">
                                        <SelectTrigger id="ssid" class="w-full">
                                            <SelectValue :placeholder="t('pages.wifi.networkPlaceholder')" />
                                        </SelectTrigger>
                                        <SelectContent>
                                            <SelectItem v-for="option in networkOptions" :key="option.value" :value="option.value" :text-value="option.label">
                                                <div class="flex flex-col">
                                                    <span>{{ option.label }}</span>
                                                    <span class="text-xs text-muted-foreground">{{ option.meta }}</span>
                                                </div>
                                            </SelectItem>
                                        </SelectContent>
                                    </Select>
                                </Field>

                                <!-- Password input -->
                                <Field>
                                    <FieldLabel for="password">{{ t('pages.wifi.password') }}</FieldLabel>
                                    <Input id="password" v-model="password" type="password" :placeholder="t('pages.wifi.passwordPlaceholder')" />
                                </Field>

                                <!-- Connection note -->
                                <Alert>
                                    <HugeiconsIcon :icon="InformationCircleIcon" :stroke-width="1.8" />
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
                        <Button variant="default" type="button" class="w-full" :disabled="!ssid || !password" @click="handleConnectPress">
                            <HugeiconsIcon :icon="WifiFullSignalIcon" :stroke-width="1.8" class="size-4" />
                            {{ t('pages.wifi.connectAction') }}
                        </Button>
                    </CardFooter>
                </Card>
            </div>
        </main>
    </div>
</template>