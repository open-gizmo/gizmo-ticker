<script setup>
import { computed, ref } from 'vue';
import { Cancel01Icon, CheckmarkCircle02Icon, InformationCircleIcon, Key01Icon, SaveIcon } from '@hugeicons/core-free-icons';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Alert, AlertDescription, AlertTitle } from '@brumaombra/ui-vintage/alert';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { Card, CardContent, CardFooter } from '@brumaombra/ui-vintage/card';
import { showConfirmDialog } from '@brumaombra/ui-vintage/confirm-dialog';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { Field, FieldGroup, FieldLabel } from '@brumaombra/ui-vintage/field';
import { Input } from '@brumaombra/ui-vintage/input';
import { saveApiKey } from '~/composables/useDeviceApi.js';
import { createIconComponent, handleBackendErrors } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import PageIntroCard from '~/components/PageIntroCard.vue';

const { t } = useI18n();
const globalStore = useGlobalStore();
const apiKey = ref('');

// Icon components
const successDialogIcon = createIconComponent(CheckmarkCircle02Icon);
const keyDialogIcon = createIconComponent(Key01Icon);
const saveDialogIcon = createIconComponent(SaveIcon);
const closeDialogIcon = createIconComponent(Cancel01Icon);

// Require a non-empty API key before submitting
const isFormValid = computed(() => {
    return apiKey.value.trim();
});

// Save the API key on the device
const saveCurrentApiKey = async () => {
    try {
        setBusy(true);
        await saveApiKey(apiKey.value);
        globalStore.value.config.apiKey = apiKey.value;
        showMessageDialog({
            type: 'Success',
            icon: successDialogIcon,
            title: t('dialogs.successTitle'),
            message: t('pages.apiKey.saveSuccess'),
            closeText: t('common.close'),
            closeButtonIcon: closeDialogIcon
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
    showConfirmDialog({
        icon: keyDialogIcon,
        title: t('pages.apiKey.confirmTitle'),
        message: t('pages.apiKey.confirmMessage'),
        confirmText: t('pages.apiKey.saveAction'),
        cancelText: t('common.keepEditing'),
        confirmButtonType: 'default',
        cancelButtonType: 'outline',
        confirmButtonIcon: saveDialogIcon,
        cancelButtonIcon: closeDialogIcon,
        onConfirm: saveCurrentApiKey
    });
};

// Page metadata
definePageMeta({
    layout: 'private'
});
</script>

<template>
    <div class="mx-auto flex w-full flex-col gap-6 lg:flex-row">
        <!-- Page intro -->
        <div class="w-full lg:max-w-sm">
            <PageIntroCard :eyebrow="t('pages.apiKey.eyebrow')"
                :title="t('pages.apiKey.title')"
                :description="t('pages.apiKey.description')"
                :icon="Key01Icon"
                :icon-label="t('pages.apiKey.label')" />
        </div>

        <!-- API key form -->
        <div class="min-w-0 flex-1">
            <Card>
                <!-- Card content -->
                <CardContent>
                    <form @submit.prevent="handleSavePress">
                        <FieldGroup>
                            <!-- API key input -->
                            <Field>
                                <FieldLabel for="apiKey">{{ t('pages.apiKey.label') }}</FieldLabel>
                                <Input id="apiKey" v-model="apiKey" type="text" :placeholder="t('pages.apiKey.placeholder')" />
                            </Field>

                            <!-- API key guidance -->
                            <Alert>
                                <HugeiconsIcon :icon="InformationCircleIcon" :stroke-width="1.8" />
                                <AlertTitle>{{ t('common.note') }}</AlertTitle>
                                <AlertDescription>
                                    {{ t('pages.apiKey.note') }}
                                </AlertDescription>
                            </Alert>
                        </FieldGroup>
                    </form>
                </CardContent>

                <!-- Card footer -->
                <CardFooter>
                    <!-- Save button -->
                    <Button variant="default" type="button" class="w-full" :disabled="!isFormValid" @click="handleSavePress">
                        <HugeiconsIcon :icon="SaveIcon" :stroke-width="1.8" class="size-4" />
                        {{ t('pages.apiKey.saveAction') }}
                    </Button>
                </CardFooter>
            </Card>
        </div>
    </div>
</template>