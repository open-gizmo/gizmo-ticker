<script setup>
import { computed, ref } from 'vue';
import Cancel01Icon from '@hugeicons/core-free-icons/Cancel01Icon';
import CheckmarkCircle02Icon from '@hugeicons/core-free-icons/CheckmarkCircle02Icon';
import LockIcon from '@hugeicons/core-free-icons/LockIcon';
import RefreshIcon from '@hugeicons/core-free-icons/RefreshIcon';
import WifiFullSignalIcon from '@hugeicons/core-free-icons/WifiFullSignalIcon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { showConfirmDialog } from '@brumaombra/ui-vintage/confirm-dialog';
import { Dialog, DialogContent, DialogDescription, DialogFooter, DialogHeader, DialogTitle } from '@brumaombra/ui-vintage/dialog';
import { EmptyStateCard } from '@brumaombra/ui-vintage/empty-state-card';
import { Field, FieldContent, FieldLabel } from '@brumaombra/ui-vintage/field';
import { Input } from '@brumaombra/ui-vintage/input';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { connectToWiFi, getNetworks } from '~/composables/useDeviceApi.js';
import { handleBackendErrors } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Props: the copy of the confirmation (the setup hotspot page says the hotspot closes)
const props = defineProps({
    confirmTitle: { type: String, required: true },
    confirmMessageKey: { type: String, required: true }
});

const { t } = useI18n();
const globalStore = useGlobalStore();
const selectedNetwork = ref(null);
const password = ref('');
const isPasswordDialogVisible = ref(false);
const isLoading = ref(false);

// Networks found by the device, strongest first
const networks = computed(() => {
    const scannedNetworks = Array.isArray(globalStore.value.networksList) ? globalStore.value.networksList : [];
    return [...scannedNetworks].sort((first, second) => second.quality - first.quality);
});

// Refresh the list of networks found by the device
const handleRefreshPress = async () => {
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

// Send the credentials to the device and wait for the result
const connectDeviceToWiFi = async () => {
    const ssid = selectedNetwork.value.ssid;

    try {
        setBusy(true);
        const result = await connectToWiFi(ssid, password.value);
        globalStore.value.config.ssid = result.ssid || ssid;
        showMessageDialog({
            type: 'success',
            icon: CheckmarkCircle02Icon,
            title: t('pages.wifi.connectedTitle'),
            message: t('pages.wifi.connectedMessage', { ssid: result.ssid || ssid, hostname: result.hostname, ip: result.ip }),
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

// Ask for confirmation before switching the device to the selected network
const confirmConnection = () => {
    showConfirmDialog({
        icon: WifiFullSignalIcon,
        title: props.confirmTitle,
        message: t(props.confirmMessageKey, { ssid: selectedNetwork.value.ssid }),
        confirmText: t('pages.wifi.connectAction'),
        cancelText: t('common.cancel'),
        confirmButtonType: 'primary',
        cancelButtonType: 'secondary',
        confirmButtonIcon: WifiFullSignalIcon,
        cancelButtonIcon: Cancel01Icon,
        onConfirm: connectDeviceToWiFi
    });
};

// Pick a network: secured networks ask for the password first, open ones go straight to the confirmation
const handleNetworkPress = network => {
    selectedNetwork.value = network;
    password.value = '';

    // Ask for the password of a secured network
    if (network.secured) {
        isPasswordDialogVisible.value = true;
        return;
    }

    // Open networks have no password
    confirmConnection();
};

// Continue with the password typed in the dialog
const handlePasswordSubmit = () => {
    if (!password.value) return;
    isPasswordDialogVisible.value = false;
    confirmConnection();
};
</script>

<template>
    <div class="space-y-6">
        <!-- Networks found by the device -->
        <ListGroup v-if="networks.length" :title="t('pages.wifi.availableNetworks')" :hint="t('pages.wifi.note')">
            <ListRow v-for="network in networks"
                :key="network.ssid"
                :label="network.ssid"
                :icon="network.secured ? LockIcon : WifiFullSignalIcon"
                :value="`${network.quality}%`"
                action
                @click="handleNetworkPress(network)" />
        </ListGroup>

        <!-- No networks yet (the first scan can come back empty) -->
        <EmptyStateCard v-else :icon="WifiFullSignalIcon" :title="t('pages.wifi.noNetworksTitle')" :description="t('pages.wifi.noNetworksDescription')" />

        <!-- Scan again -->
        <Button variant="secondary" class="w-full" :loading="isLoading" @click="handleRefreshPress">
            <HugeiconsIcon :icon="RefreshIcon" class="size-4" />
            {{ isLoading ? t('common.refreshing') : t('pages.wifi.scanAgain') }}
        </Button>

        <!-- Password of a secured network -->
        <Dialog :open="isPasswordDialogVisible" @update:open="value => { isPasswordDialogVisible = value; }">
            <DialogContent>
                <form @submit.prevent="handlePasswordSubmit">
                    <!-- Header -->
                    <DialogHeader>
                        <DialogTitle>{{ t('pages.wifi.passwordDialogTitle', { ssid: selectedNetwork?.ssid }) }}</DialogTitle>
                        <DialogDescription>{{ t('pages.wifi.passwordDialogDescription') }}</DialogDescription>
                    </DialogHeader>

                    <!-- Password -->
                    <Field class="my-4">
                        <FieldLabel for="wifi-password">{{ t('pages.wifi.password') }}</FieldLabel>
                        <FieldContent>
                            <Input id="wifi-password" v-model="password" type="password" autocomplete="current-password" :placeholder="t('pages.wifi.passwordPlaceholder')" />
                        </FieldContent>
                    </Field>

                    <!-- Actions -->
                    <DialogFooter>
                        <Button type="button" variant="secondary" @click="isPasswordDialogVisible = false">
                            <HugeiconsIcon :icon="Cancel01Icon" class="size-4" />
                            {{ t('common.cancel') }}
                        </Button>
                        <Button type="submit" variant="primary" :disabled="!password">
                            <HugeiconsIcon :icon="WifiFullSignalIcon" class="size-4" />
                            {{ t('pages.wifi.connectAction') }}
                        </Button>
                    </DialogFooter>
                </form>
            </DialogContent>
        </Dialog>
    </div>
</template>