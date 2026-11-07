<script setup>
import { computed, ref, watch } from 'vue';
import Alert02Icon from '@hugeicons/core-free-icons/Alert02Icon';
import Cancel01Icon from '@hugeicons/core-free-icons/Cancel01Icon';
import CheckmarkCircle02Icon from '@hugeicons/core-free-icons/CheckmarkCircle02Icon';
import Coins01Icon from '@hugeicons/core-free-icons/Coins01Icon';
import DashboardSpeed01Icon from '@hugeicons/core-free-icons/DashboardSpeed01Icon';
import GridViewIcon from '@hugeicons/core-free-icons/GridViewIcon';
import InformationCircleIcon from '@hugeicons/core-free-icons/InformationCircleIcon';
import RefreshIcon from '@hugeicons/core-free-icons/RefreshIcon';
import SaveIcon from '@hugeicons/core-free-icons/SaveIcon';
import Sun03Icon from '@hugeicons/core-free-icons/Sun03Icon';
import TextNumberSignIcon from '@hugeicons/core-free-icons/TextNumberSignIcon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { showConfirmDialog } from '@brumaombra/ui-vintage/confirm-dialog';
import { EmptyStateCard } from '@brumaombra/ui-vintage/empty-state-card';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { LoadingStateCard } from '@brumaombra/ui-vintage/loading-state-card';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';
import { OptionPickerDialog } from '@brumaombra/ui-vintage/option-picker-dialog';
import { Slider } from '@brumaombra/ui-vintage/slider';
import { Switch } from '@brumaombra/ui-vintage/switch';
import { resetSettings, saveSettings } from '~/composables/useDeviceApi.js';
import { loadDeviceConfig } from '~/composables/useDeviceConfig.js';
import { getCryptoCoinById, getCryptoCoins, setCryptoCoin } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const globalStore = useGlobalStore();
const cryptoCoins = getCryptoCoins();
const { t } = useI18n();
const openPicker = ref(''); // Picker currently open ('coin', 'format', 'modules', or none)

// Reactive config payload
const config = computed(() => globalStore.value.config);

// Number of modules the device runs with now (the one last read or saved), to tell when a save restarts it
const savedMatrixModules = ref(null);
watch(() => globalStore.value.configLoaded, isLoaded => {
    if (isLoaded) savedMatrixModules.value = globalStore.value.config.matrixModules;
}, { immediate: true });

// Slider percentage for matrix intensity
const matrixIntensityPercentage = computed(() => {
    return Math.round((Number(config.value?.matrixIntensity ?? 0) / 15) * 100);
});

// Slider percentage for scroll speed
const scrollSpeedPercentage = computed(() => {
    return Math.round((Number(config.value?.scrollSpeed ?? 0) / 15) * 100);
});

// Help under the matrix group: what each slider does
const matrixHint = computed(() => `${t('pages.settings.matrixIntensity.note')} ${t('pages.settings.scrollSpeed.note')} ${t('pages.settings.matrixModules.note')}`);

// Available thousands separator formats
const separatorOptions = computed(() => ([
    { value: 'US', label: '21,000.00', description: t('pages.settings.formatType.usMeta') },
    { value: 'EU', label: '21.000,00', description: t('pages.settings.formatType.euMeta') }
]));

// Numbers of matrix modules the firmware can drive: whole 4-in-1 boards, from 1 to 8
const matrixModuleOptions = computed(() => Array.from({ length: 8 }, (_, index) => (index + 1) * 4).map(count => ({
    value: count,
    label: t('pages.settings.matrixModules.option', { count }),
    description: t('pages.settings.matrixModules.boards', { count: count / 4 }, count / 4)
})));

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

// Set the matrix intensity from the slider (it gives an array with one value)
const handleMatrixIntensityUpdate = value => {
    config.value.matrixIntensity = value?.[0] ?? 0;
};

// Set the scroll speed from the slider (it gives an array with one value)
const handleScrollSpeedUpdate = value => {
    config.value.scrollSpeed = value?.[0] ?? 0;
};

// Pick a coin (the accent color of the web app follows it right away)
const handleCryptoCoinSelect = cryptoCoin => {
    setCryptoCoin(cryptoCoin);
};

// Pick the number of matrix modules
const handleMatrixModulesSelect = matrixModules => {
    config.value.matrixModules = matrixModules;
};

// Pick a number format
const handleFormatTypeSelect = formatType => {
    config.value.formatType = formatType;
};

// Load the device config again after a failed load
const handleRetryLoadPress = async () => {
    try {
        setBusy(true, { label: t('common.syncingDevice') });
        await loadDeviceConfig();
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('app.loadError'), showDialog: true });
    } finally {
        setBusy(false);
    }
};

// Save the current settings payload
const saveCurrentSettings = async () => {
    // Only save the config read from the device (never the store defaults)
    if (!config.value || !globalStore.value.configLoaded) {
        return;
    }

    try {
        setBusy(true);
        const isRestarting = config.value.matrixModules !== savedMatrixModules.value; // The firmware restarts to use a new number of modules
        await saveSettings(config.value);
        savedMatrixModules.value = config.value.matrixModules;
        showMessageDialog({
            type: 'success',
            icon: CheckmarkCircle02Icon,
            title: t('dialogs.successTitle'),
            message: isRestarting ? t('pages.settings.matrixModules.restartMessage', { count: config.value.matrixModules }) : t('pages.settings.saveSuccess'),
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
    <GizmoPage :title="t('pages.settings.title')">
        <!-- The settings couldn't be read: offer to try again (a form here would save the defaults over the real settings) -->
        <EmptyStateCard v-if="globalStore.configLoadFailed && !globalStore.configLoaded" :icon="Alert02Icon" :title="t('pages.settings.loadErrorTitle')" :description="t('pages.settings.loadErrorDescription')">
            <template #action>
                <Button variant="primary" @click="handleRetryLoadPress">
                    <HugeiconsIcon :icon="RefreshIcon" class="size-4" />
                    {{ t('pages.settings.loadRetryAction') }}
                </Button>
            </template>
        </EmptyStateCard>

        <!-- Still reading the settings from the device -->
        <LoadingStateCard v-else-if="!globalStore.configLoaded" :title="t('common.syncingDevice')" />

        <!-- Settings form -->
        <form v-else class="space-y-6" @submit.prevent="handleSavePress">
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

            <!-- Matrix brightness and speed (the sliders stay in the rows, so the device can be watched while dragging) -->
            <ListGroup :title="t('pages.settings.matrixTitle')" :hint="matrixHint">
                <ListRow :label="t('pages.settings.matrixIntensity.label')" :icon="Sun03Icon" :value="`${matrixIntensityPercentage}%`">
                    <Slider :model-value="[Number(config.matrixIntensity ?? 0)]" :min="0" :max="15" :step="1" :thumb-label="t('pages.settings.matrixIntensity.label')" @update:model-value="handleMatrixIntensityUpdate" />
                </ListRow>
                <ListRow :label="t('pages.settings.scrollSpeed.label')" :icon="DashboardSpeed01Icon" :value="`${scrollSpeedPercentage}%`">
                    <Slider :model-value="[Number(config.scrollSpeed ?? 0)]" :min="0" :max="15" :step="1" :thumb-label="t('pages.settings.scrollSpeed.label')" @update:model-value="handleScrollSpeedUpdate" />
                </ListRow>
                <ListRow :label="t('pages.settings.matrixModules.label')" :icon="GridViewIcon" :value="String(config.matrixModules ?? '')" action @click="openPicker = 'modules'" />
            </ListGroup>

            <!-- Save button -->
            <Button variant="primary" type="submit" class="w-full">
                <HugeiconsIcon :icon="SaveIcon" class="size-4" />
                {{ t('pages.settings.saveAction') }}
            </Button>
        </form>

        <!-- Device info, network, restart and forget network (from the gizmo-common layer, it reads the device by itself). The reset of the saved settings, the ticker's own action, joins the destructive rows at the end -->
        <GizmoDeviceCard>
            <template v-if="globalStore.configLoaded" #danger-rows>
                <ListRow :label="t('pages.settings.resetAction')" :icon="RefreshIcon" action danger @click="handleResetPress" />
            </template>
        </GizmoDeviceCard>

        <!-- Coin picker -->
        <OptionPickerDialog :open="openPicker === 'coin'" :title="t('pages.settings.cryptoCoin.label')" :description="t('pages.settings.cryptoCoin.note')" :options="cryptoCoinOptions" :model-value="config.cryptoCoin" @update:model-value="handleCryptoCoinSelect" @update:open="open => { if (!open) openPicker = ''; }">
            <template #icon="{ option }">
                <img :src="option.image" alt="" aria-hidden="true" decoding="async" class="size-8 shrink-0">
            </template>
        </OptionPickerDialog>

        <!-- Matrix modules picker -->
        <OptionPickerDialog :open="openPicker === 'modules'" :title="t('pages.settings.matrixModules.label')" :description="t('pages.settings.matrixModules.note')" :options="matrixModuleOptions" :model-value="config.matrixModules" @update:model-value="handleMatrixModulesSelect" @update:open="open => { if (!open) openPicker = ''; }" />

        <!-- Number format picker -->
        <OptionPickerDialog :open="openPicker === 'format'" :title="t('pages.settings.formatType.label')" :options="separatorOptions" :model-value="config.formatType" @update:model-value="handleFormatTypeSelect" @update:open="open => { if (!open) openPicker = ''; }" />
    </GizmoPage>
</template>