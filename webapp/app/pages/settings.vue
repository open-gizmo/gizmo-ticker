<script setup>
import { computed } from 'vue';
import { Cancel01Icon, CheckmarkCircle02Icon, InformationCircleIcon, RefreshIcon, SaveIcon, Settings01Icon } from '@hugeicons/core-free-icons';
import { HugeiconsIcon } from '@hugeicons/vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@brumaombra/ui-vintage/card';
import { showConfirmDialog } from '@brumaombra/ui-vintage/confirm-dialog';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { Field, FieldDescription, FieldGroup, FieldLabel } from '@brumaombra/ui-vintage/field';
import { SliderFormComponent } from '@brumaombra/ui-vintage/slider-form-component';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@brumaombra/ui-vintage/select';
import { SwitchFormComponent } from '@brumaombra/ui-vintage/switch-form-component';
import { resetSettings, saveSettings } from '~/composables/useDeviceApi.js';
import { createIconComponent, getCryptoCoins, handleBackendErrors, setCryptoCoin } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import PageIntroCard from '~/components/PageIntroCard.vue';
import BitcoinLogo from '~/components/crypto/BitcoinLogo.vue';
import KaspaLogo from '~/components/crypto/KaspaLogo.vue';

const globalStore = useGlobalStore();
const cryptoCoins = getCryptoCoins();
const { t } = useI18n();

// Icon components
const successDialogIcon = createIconComponent(CheckmarkCircle02Icon);
const infoDialogIcon = createIconComponent(InformationCircleIcon);
const resetDialogIcon = createIconComponent(RefreshIcon);
const saveDialogIcon = createIconComponent(SaveIcon);
const closeDialogIcon = createIconComponent(Cancel01Icon);

// Reactive config payload
const config = computed(() => globalStore.value.config);

// Slider percentage for matrix intensity
const matrixIntensityPercentage = computed(() => {
    return Math.round((Number(config.value?.matrixIntensity ?? 0) / 15) * 100);
});

// Slider percentage for scroll speed
const scrollSpeedPercentage = computed(() => {
    return Math.round((Number(config.value?.scrollSpeed ?? 0) / 15) * 100);
});

// Available thousands separator formats
const separatorOptions = computed(() => ([
    { value: 'US', label: '21,000.00', meta: t('pages.settings.formatType.usMeta') },
    { value: 'EU', label: '21.000,00', meta: t('pages.settings.formatType.euMeta') }
]));

// Resolve the icon for each crypto coin option
const cryptoCoinLogos = {
    bitcoin: BitcoinLogo,
    kaspa: KaspaLogo
};

// Available crypto coin choices
const cryptoCoinOptions = computed(() => {
    return cryptoCoins.map(coin => ({
        value: coin.id,
        label: t(`pages.settings.cryptoCoin.${coin.id}Label`),
        icon: cryptoCoinLogos[coin.id] || BitcoinLogo
    }));
});

// Save the current settings payload
const saveCurrentSettings = async () => {
    // If no config, there is nothing to save
    if (!config.value) {
        return;
    }

    try {
        setBusy(true);
        await saveSettings(config.value);
        showMessageDialog({
            type: 'Success',
            icon: successDialogIcon,
            title: t('dialogs.successTitle'),
            message: t('pages.settings.saveSuccess'),
            closeText: t('common.close'),
            closeButtonIcon: closeDialogIcon
        });
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('pages.settings.saveError'), showDialog: true });
    } finally {
        setBusy(false);
    }
};

// Reset the saved device settings and restart the firmware
const resetSavedSettings = async () => {
    try {
        setBusy(true);
        const result = await resetSettings();
        showMessageDialog({
            type: 'Info',
            icon: infoDialogIcon,
            title: t('pages.settings.resetInfoTitle'),
            message: result.message || t('pages.settings.resetInfoMessage'),
            closeText: t('common.close'),
            closeButtonIcon: closeDialogIcon
        });
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('pages.settings.resetError'), showDialog: true });
    } finally {
        setBusy(false);
    }
};

// Ask for confirmation before saving the current settings
const handleSavePress = () => {
    showConfirmDialog({
        icon: saveDialogIcon,
        title: t('pages.settings.saveConfirmTitle'),
        message: t('pages.settings.saveConfirmMessage'),
        confirmText: t('pages.settings.saveAction'),
        cancelText: t('common.keepEditing'),
        confirmButtonType: 'default',
        cancelButtonType: 'outline',
        confirmButtonIcon: saveDialogIcon,
        cancelButtonIcon: closeDialogIcon,
        onConfirm: saveCurrentSettings
    });
};

// Ask for confirmation before resetting the saved settings
const handleResetPress = () => {
    showConfirmDialog({
        icon: resetDialogIcon,
        title: t('pages.settings.resetConfirmTitle'),
        message: t('pages.settings.resetConfirmMessage'),
        confirmText: t('pages.settings.resetAction'),
        cancelText: t('common.cancel'),
        confirmButtonType: 'default',
        cancelButtonType: 'outline',
        confirmButtonIcon: resetDialogIcon,
        cancelButtonIcon: closeDialogIcon,
        onConfirm: resetSavedSettings
    });
};

// Page metadata
definePageMeta({
    layout: 'private'
});
</script>

<template>
    <div class="mx-auto flex w-full min-w-0 flex-col gap-6">
        <div class="grid min-w-0 gap-6 xl:grid-cols-[minmax(0,320px),minmax(0,1fr)]">
            <!-- Page intro -->
            <PageIntroCard class="min-w-0"
                :eyebrow="t('pages.settings.eyebrow')"
                :title="t('pages.settings.title')"
                :description="t('pages.settings.description')"
                :icon="Settings01Icon"
                :icon-label="t('nav.settings.label')" />

            <!-- Settings form -->
            <form class="min-w-0 space-y-6" @submit.prevent="handleSavePress">
                <!-- Visibility card -->
                <Card>
                    <!-- Visibility section -->
                    <CardHeader>
                        <CardTitle>{{ t('pages.settings.visibilityTitle') }}</CardTitle>
                        <CardDescription>{{ t('pages.settings.visibilityDescription') }}</CardDescription>
                    </CardHeader>

                    <!-- Visibility toggles -->
                    <CardContent class="space-y-3">
                        <!-- Current price -->
                        <SwitchFormComponent id="currentPrice"
                            v-model="config.currentPrice"
                            :label="t('pages.settings.visibility.currentPrice.label')"
                            :description="t('pages.settings.visibility.currentPrice.description')" />

                        <!-- Price change -->
                        <SwitchFormComponent id="priceChange"
                            v-model="config.priceChange"
                            :label="t('pages.settings.visibility.priceChange.label')"
                            :description="t('pages.settings.visibility.priceChange.description')" />

                        <!-- Market cap -->
                        <SwitchFormComponent id="marketCap"
                            v-model="config.marketCap"
                            :label="t('pages.settings.visibility.marketCap.label')"
                            :description="t('pages.settings.visibility.marketCap.description')" />

                        <!-- 24h volume -->
                        <SwitchFormComponent id="dailyHighLow"
                            v-model="config.dailyHighLow"
                            :label="t('pages.settings.visibility.dailyHighLow.label')"
                            :description="t('pages.settings.visibility.dailyHighLow.description')" />

                        <!-- 24h high / low -->
                        <SwitchFormComponent id="yearHighLow"
                            v-model="config.yearHighLow"
                            :label="t('pages.settings.visibility.yearHighLow.label')"
                            :description="t('pages.settings.visibility.yearHighLow.description')" />

                        <!-- Open price -->
                        <SwitchFormComponent id="openPrice"
                            v-model="config.openPrice"
                            :label="t('pages.settings.visibility.openPrice.label')"
                            :description="t('pages.settings.visibility.openPrice.description')" />

                        <!-- Volume -->
                        <SwitchFormComponent id="volume"
                            v-model="config.volume"
                            :label="t('pages.settings.visibility.volume.label')"
                            :description="t('pages.settings.visibility.volume.description')" />
                    </CardContent>
                </Card>

                <!-- Formatting & motion card -->
                <Card>
                    <!-- Formatting section -->
                    <CardHeader>
                        <CardTitle>{{ t('pages.settings.formattingTitle') }}</CardTitle>
                        <CardDescription>{{ t('pages.settings.formattingDescription') }}</CardDescription>
                    </CardHeader>

                    <!-- Formatting controls -->
                    <CardContent>
                        <FieldGroup>
                            <!-- Crypto coin -->
                            <Field>
                                <FieldLabel for="cryptoCoin">{{ t('pages.settings.cryptoCoin.label') }}</FieldLabel>
                                <Select v-model="config.cryptoCoin" @update:modelValue="setCryptoCoin">
                                    <SelectTrigger id="cryptoCoin" class="w-full">
                                        <SelectValue :placeholder="t('pages.settings.cryptoCoin.placeholder')" />
                                    </SelectTrigger>
                                    <SelectContent>
                                        <SelectItem v-for="option in cryptoCoinOptions" :key="option.value" :value="option.value" :text-value="option.label">
                                            <div class="flex items-center gap-2">
                                                <component :is="option.icon" class="size-6 shrink-0" />
                                                <span>{{ option.label }}</span>
                                            </div>
                                        </SelectItem>
                                    </SelectContent>
                                </Select>
                                <FieldDescription>{{ t('pages.settings.cryptoCoin.note') }}</FieldDescription>
                            </Field>

                            <!-- Thousands separator format -->
                            <Field>
                                <FieldLabel for="selectFormatType">{{ t('pages.settings.formatType.label') }}</FieldLabel>
                                <Select v-model="config.formatType">
                                    <SelectTrigger id="selectFormatType" class="w-full">
                                        <SelectValue :placeholder="t('pages.settings.formatType.placeholder')" />
                                    </SelectTrigger>
                                    <SelectContent>
                                        <SelectItem v-for="option in separatorOptions" :key="option.value" :value="option.value" :text-value="option.label">
                                            <div class="flex flex-col">
                                                <span>{{ option.label }}</span>
                                                <span class="text-xs text-muted-foreground">{{ option.meta }}</span>
                                            </div>
                                        </SelectItem>
                                    </SelectContent>
                                </Select>
                                <FieldDescription>{{ t('pages.settings.formatType.note') }}</FieldDescription>
                            </Field>

                            <!-- Matrix intensity -->
                            <SliderFormComponent id="matrixIntensity"
                                v-model="config.matrixIntensity"
                                :label="t('pages.settings.matrixIntensity.label')"
                                :description="t('pages.settings.matrixIntensity.note')"
                                :min="0"
                                :max="15"
                                :value-text="`${matrixIntensityPercentage}%`" />

                            <!-- Scroll speed -->
                            <SliderFormComponent id="scrollSpeed"
                                v-model="config.scrollSpeed"
                                :label="t('pages.settings.scrollSpeed.label')"
                                :description="t('pages.settings.scrollSpeed.note')"
                                :min="0"
                                :max="15"
                                :value-text="`${scrollSpeedPercentage}%`" />
                        </FieldGroup>
                    </CardContent>
                </Card>

                <!-- Action buttons -->
                <div class="flex flex-col gap-3 md:flex-row md:justify-end">
                    <!-- Save button -->
                    <Button variant="default" type="submit" class="w-full md:w-auto">
                        <HugeiconsIcon :icon="SaveIcon" :stroke-width="1.8" class="size-4" />
                        {{ t('pages.settings.saveAction') }}
                    </Button>
                    
                    <!-- Reset button -->
                    <Button variant="outline" type="button" class="w-full md:w-auto" @click="handleResetPress">
                        <HugeiconsIcon :icon="RefreshIcon" :stroke-width="1.8" class="size-4" />
                        {{ t('pages.settings.resetAction') }}
                    </Button>
                </div>
            </form>
        </div>
    </div>
</template>