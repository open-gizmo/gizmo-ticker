<script setup>
import { computed, ref } from 'vue';
import Cancel01Icon from '@hugeicons/core-free-icons/Cancel01Icon';
import CheckmarkCircle02Icon from '@hugeicons/core-free-icons/CheckmarkCircle02Icon';
import Key01Icon from '@hugeicons/core-free-icons/Key01Icon';
import SaveIcon from '@hugeicons/core-free-icons/SaveIcon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { showConfirmDialog } from '@brumaombra/ui-vintage/confirm-dialog';
import { Input } from '@brumaombra/ui-vintage/input';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { saveApiKey } from '~/composables/useDeviceApi.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const API_KEY_MAX_LENGTH = 34; // Longest key the device stores
const { t } = useI18n();
const globalStore = useGlobalStore();
const apiKey = ref('');

// Key without the spaces or line breaks a paste can bring (CoinGecko would refuse it)
const trimmedApiKey = computed(() => {
    return apiKey.value.trim();
});

// Require a non-empty API key before submitting
const isFormValid = computed(() => {
    return trimmedApiKey.value !== '';
});

// Key saved on the device (the device only ever returns it masked)
const savedApiKeyLabel = computed(() => {
    return globalStore.value.config.apiKey || t('pages.apiKey.notSet');
});

// Mask a key like the device does, keeping 3 characters at each end
const maskApiKey = key => {
    if (key.length <= 6) return key;
    return `${key.slice(0, 3)}${'*'.repeat(key.length - 6)}${key.slice(-3)}`;
};

// Save the API key on the device
const saveCurrentApiKey = async () => {
    try {
        setBusy(true);
        await saveApiKey(trimmedApiKey.value);
        globalStore.value.config.apiKey = maskApiKey(trimmedApiKey.value);
        apiKey.value = ''; // Clear the field once saved (after a failure, the key stays to try again)
        showMessageDialog({
            type: 'success',
            icon: CheckmarkCircle02Icon,
            title: t('dialogs.successTitle'),
            message: t('pages.apiKey.saveSuccess'),
            closeText: t('common.close'),
            closeButtonIcon: Cancel01Icon
        });
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('pages.apiKey.saveError'), showDialog: true });
    } finally {
        setBusy(false);
    }
};

// Ask for confirmation before overwriting the stored API key
const handleSavePress = () => {
    if (!isFormValid.value) return;
    showConfirmDialog({
        icon: Key01Icon,
        title: t('pages.apiKey.confirmTitle'),
        message: t('pages.apiKey.confirmMessage'),
        confirmText: t('pages.apiKey.saveAction'),
        cancelText: t('common.keepEditing'),
        confirmButtonType: 'primary',
        cancelButtonType: 'secondary',
        confirmButtonIcon: SaveIcon,
        cancelButtonIcon: Cancel01Icon,
        onConfirm: saveCurrentApiKey
    });
};

// Page metadata
definePageMeta({
    layout: 'dashboard'
});
</script>

<template>
    <GizmoPage :title="t('pages.apiKey.title')">
        <!-- Key saved on the device -->
        <ListGroup :title="t('pages.apiKey.currentKeyLabel')">
            <ListRow :label="savedApiKeyLabel" :icon="Key01Icon" />
        </ListGroup>

        <!-- New key, and the button to save it -->
        <form class="space-y-6" @submit.prevent="handleSavePress">
            <ListGroup :title="t('pages.apiKey.newKeyTitle')" :hint="t('pages.apiKey.note')">
                <ListRow :label="t('pages.apiKey.label')" :icon="Key01Icon" label-for="apiKey">
                    <Input id="apiKey" v-model="apiKey" autocomplete="off" :maxlength="API_KEY_MAX_LENGTH" :placeholder="t('pages.apiKey.placeholder')" />
                </ListRow>
            </ListGroup>

            <!-- Save button -->
            <Button variant="primary" type="submit" class="w-full" :disabled="!isFormValid">
                <HugeiconsIcon :icon="SaveIcon" class="size-4" />
                {{ t('pages.apiKey.saveAction') }}
            </Button>
        </form>
    </GizmoPage>
</template>