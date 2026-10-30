<script setup>
import { computed, ref } from 'vue';
import Cancel01Icon from '@hugeicons/core-free-icons/Cancel01Icon';
import CheckmarkCircle02Icon from '@hugeicons/core-free-icons/CheckmarkCircle02Icon';
import Key01Icon from '@hugeicons/core-free-icons/Key01Icon';
import SaveIcon from '@hugeicons/core-free-icons/SaveIcon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { Card, CardContent } from '@brumaombra/ui-vintage/card';
import { showConfirmDialog } from '@brumaombra/ui-vintage/confirm-dialog';
import { Field, FieldContent, FieldDescription, FieldLabel } from '@brumaombra/ui-vintage/field';
import { Input } from '@brumaombra/ui-vintage/input';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { PageHeader } from '@brumaombra/ui-vintage/page-header';
import { saveApiKey } from '~/composables/useDeviceApi.js';
import { handleBackendErrors } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const { t } = useI18n();
const globalStore = useGlobalStore();
const apiKey = ref('');

// Require a non-empty API key before submitting
const isFormValid = computed(() => {
    return apiKey.value.trim();
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
        await saveApiKey(apiKey.value);
        globalStore.value.config.apiKey = maskApiKey(apiKey.value);
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
        apiKey.value = '';
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
    <div class="mx-auto w-full max-w-2xl space-y-6">
        <!-- Title -->
        <PageHeader :title="t('pages.apiKey.title')" size="medium" />

        <!-- Key saved on the device -->
        <ListGroup :title="t('pages.apiKey.currentKeyLabel')">
            <ListRow :label="savedApiKeyLabel" :icon="Key01Icon" icon-tile />
        </ListGroup>

        <!-- New key -->
        <form class="space-y-6" @submit.prevent="handleSavePress">
            <Card>
                <CardContent>
                    <Field>
                        <FieldLabel for="apiKey">{{ t('pages.apiKey.label') }}</FieldLabel>
                        <FieldContent>
                            <Input id="apiKey" v-model="apiKey" type="text" autocomplete="off" :placeholder="t('pages.apiKey.placeholder')" />
                            <FieldDescription>{{ t('pages.apiKey.note') }}</FieldDescription>
                        </FieldContent>
                    </Field>
                </CardContent>
            </Card>

            <!-- Save button -->
            <Button variant="primary" type="submit" class="w-full" :disabled="!isFormValid">
                <HugeiconsIcon :icon="SaveIcon" class="size-4" />
                {{ t('pages.apiKey.saveAction') }}
            </Button>
        </form>
    </div>
</template>