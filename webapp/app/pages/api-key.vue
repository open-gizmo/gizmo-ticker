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
import { Field, FieldContent, FieldGroup, FieldLabel } from '@brumaombra/ui-vintage/field';
import { Input } from '@brumaombra/ui-vintage/input';
import { saveApiKey } from '~/composables/useDeviceApi.js';
import { handleBackendErrors } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import PageIntroCard from '~/components/PageIntroCard.vue';

const { t } = useI18n();
const globalStore = useGlobalStore();
const apiKey = ref('');

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
    layout: 'private'
});
</script>

<template>
    <div class="mx-auto flex w-full flex-col gap-4 md:gap-6 lg:flex-row">
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
                                <FieldContent>
                                    <Input id="apiKey" v-model="apiKey" type="text" :placeholder="t('pages.apiKey.placeholder')" />
                                </FieldContent>
                            </Field>

                            <!-- API key guidance -->
                            <Alert>
                                <HugeiconsIcon :icon="InformationCircleIcon" />
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
                    <Button variant="primary" type="button" class="w-full" :disabled="!isFormValid" @click="handleSavePress">
                        <HugeiconsIcon :icon="SaveIcon" class="size-4" />
                        {{ t('pages.apiKey.saveAction') }}
                    </Button>
                </CardFooter>
            </Card>
        </div>
    </div>
</template>