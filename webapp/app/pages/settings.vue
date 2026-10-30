<script setup>
import { computed, ref } from 'vue';
import Cancel01Icon from '@hugeicons/core-free-icons/Cancel01Icon';
import CheckmarkCircle02Icon from '@hugeicons/core-free-icons/CheckmarkCircle02Icon';
import Coins01Icon from '@hugeicons/core-free-icons/Coins01Icon';
import InformationCircleIcon from '@hugeicons/core-free-icons/InformationCircleIcon';
import RefreshIcon from '@hugeicons/core-free-icons/RefreshIcon';
import SaveIcon from '@hugeicons/core-free-icons/SaveIcon';
import SlidersHorizontalIcon from '@hugeicons/core-free-icons/SlidersHorizontalIcon';
import TextNumberSignIcon from '@hugeicons/core-free-icons/TextNumberSignIcon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { showConfirmDialog } from '@brumaombra/ui-vintage/confirm-dialog';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { PageHeader } from '@brumaombra/ui-vintage/page-header';
import { SectionTitle } from '@brumaombra/ui-vintage/section-title';
import { SliderFormComponent } from '@brumaombra/ui-vintage/slider-form-component';
import { Switch } from '@brumaombra/ui-vintage/switch';
import { resetSettings, saveSettings } from '~/composables/useDeviceApi.js';
import { getCryptoCoinById, getCryptoCoins, handleBackendErrors, setCryptoCoin } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import OptionPickerDialog from '~/components/dialog/OptionPickerDialog.vue';

const globalStore = useGlobalStore();
const cryptoCoins = getCryptoCoins();
const { t } = useI18n();
const openPicker = ref(''); // Picker currently open ('coin', 'format', or none)

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
    { value: 'US', label: '21,000.00', description: t('pages.settings.formatType.usMeta') },
    { value: 'EU', label: '21.000,00', description: t('pages.settings.formatType.euMeta') }
]));

// Available crypto coin choices
const cryptoCoinOptions = computed(() => {
    return cryptoCoins.map(coin => ({
        value: coin.id,
        label: coin.name,
        image: coin.image
    }));
});

// Name of the selected coin, shown in its row
const selectedCryptoCoinName = computed(() => getCryptoCoinById(config.value?.cryptoCoin).name);

// Example of the selected number format, shown in its row
const selectedSeparatorLabel = computed(() => {
    return separatorOptions.value.find(option => option.value === config.value?.formatType)?.label || '';
});

// Matrix sections the user can show or hide, top to bottom
const visibilityRows = computed(() => ['currentPrice', 'priceChange', 'marketCap', 'dailyHighLow', 'yearHighLow', 'openPrice', 'volume'].map(id => ({
    id,
    label: t(`pages.settings.visibility.${id}.label`)
})));

// Pick a coin (the accent color of the web app follows it right away)
const handleCryptoCoinSelect = cryptoCoin => {
    setCryptoCoin(cryptoCoin);
};

// Pick a number format
const handleFormatTypeSelect = formatType => {
    config.value.formatType = formatType;
};

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
            type: 'success',
            icon: CheckmarkCircle02Icon,
            title: t('dialogs.successTitle'),
            message: t('pages.settings.saveSuccess'),
            closeText: t('common.close'),
            closeButtonIcon: Cancel01Icon
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
            type: 'info',
            icon: InformationCircleIcon,
            title: t('pages.settings.resetInfoTitle'),
            message: result.message || t('pages.settings.resetInfoMessage'),
            closeText: t('common.close'),
            closeButtonIcon: Cancel01Icon
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
        icon: SaveIcon,
        title: t('pages.settings.saveConfirmTitle'),
        message: t('pages.settings.saveConfirmMessage'),
        confirmText: t('pages.settings.saveAction'),
        cancelText: t('common.keepEditing'),
        confirmButtonType: 'primary',
        cancelButtonType: 'secondary',
        confirmButtonIcon: SaveIcon,
        cancelButtonIcon: Cancel01Icon,
        onConfirm: saveCurrentSettings
    });
};

// Ask for confirmation before resetting the saved settings
const handleResetPress = () => {
    showConfirmDialog({
        icon: RefreshIcon,
        title: t('pages.settings.resetConfirmTitle'),
        message: t('pages.settings.resetConfirmMessage'),
        confirmText: t('pages.settings.resetAction'),
        cancelText: t('common.cancel'),
        confirmButtonType: 'primary',
        cancelButtonType: 'secondary',
        confirmButtonIcon: RefreshIcon,
        cancelButtonIcon: Cancel01Icon,
        onConfirm: resetSavedSettings
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
        <PageHeader :title="t('pages.settings.title')" size="medium" />

        <!-- Settings form -->
        <form class="space-y-6" @submit.prevent="handleSavePress">
            <!-- Sections shown on the matrix -->
            <ListGroup :title="t('pages.settings.visibilityTitle')" :hint="t('pages.settings.visibilityDescription')">
                <ListRow v-for="row in visibilityRows" :key="row.id" :label="row.label" :label-for="row.id">
                    <template #trailing>
                        <Switch :id="row.id" v-model="config[row.id]" />
                    </template>
                </ListRow>
            </ListGroup>

            <!-- Coin and number format -->
            <ListGroup :title="t('pages.settings.formattingTitle')" :hint="t('pages.settings.formatType.note')">
                <ListRow :label="t('pages.settings.cryptoCoin.label')" :icon="Coins01Icon" :value="selectedCryptoCoinName" action @click="openPicker = 'coin'" />
                <ListRow :label="t('pages.settings.formatType.label')" :icon="TextNumberSignIcon" :value="selectedSeparatorLabel" action @click="openPicker = 'format'" />
            </ListGroup>

            <!-- Matrix brightness and speed -->
            <section>
                <SectionTitle :title="t('pages.settings.matrixTitle')" :icon="SlidersHorizontalIcon" />
                <div class="space-y-4">
                    <SliderFormComponent id="matrixIntensity"
                        v-model="config.matrixIntensity"
                        :label="t('pages.settings.matrixIntensity.label')"
                        :description="t('pages.settings.matrixIntensity.note')"
                        :min="0"
                        :max="15"
                        :value-text="`${matrixIntensityPercentage}%`" />
                    <SliderFormComponent id="scrollSpeed"
                        v-model="config.scrollSpeed"
                        :label="t('pages.settings.scrollSpeed.label')"
                        :description="t('pages.settings.scrollSpeed.note')"
                        :min="0"
                        :max="15"
                        :value-text="`${scrollSpeedPercentage}%`" />
                </div>
            </section>

            <!-- Save button -->
            <Button variant="primary" type="submit" class="w-full">
                <HugeiconsIcon :icon="SaveIcon" class="size-4" />
                {{ t('pages.settings.saveAction') }}
            </Button>
        </form>

        <!-- Device -->
        <ListGroup :title="t('pages.settings.deviceTitle')">
            <ListRow :label="t('pages.settings.resetAction')" :icon="RefreshIcon" action danger @click="handleResetPress" />
        </ListGroup>

        <!-- Coin picker -->
        <OptionPickerDialog :visible="openPicker === 'coin'" :title="t('pages.settings.cryptoCoin.label')" :description="t('pages.settings.cryptoCoin.note')" :options="cryptoCoinOptions" :model-value="config.cryptoCoin" @update:model-value="handleCryptoCoinSelect" @close="openPicker = ''">
            <template #icon="{ option }">
                <img :src="option.image" alt="" aria-hidden="true" decoding="async" class="size-5 shrink-0">
            </template>
        </OptionPickerDialog>

        <!-- Number format picker -->
        <OptionPickerDialog :visible="openPicker === 'format'" :title="t('pages.settings.formatType.label')" :options="separatorOptions" :model-value="config.formatType" @update:model-value="handleFormatTypeSelect" @close="openPicker = ''" />
    </div>
</template>