<script setup>
import { HugeiconsIcon } from '@hugeicons/vue';
import { InformationCircleIcon, WifiFullSignalIcon } from '@hugeicons/core-free-icons';

// Props
const props = defineProps({
    currentNetworkLabel: { type: String, required: true },
    scannedNetworksCount: { type: Number, required: true }
});

const { t } = useI18n();
</script>

<template>
    <div class="overflow-hidden rounded border border-[var(--border-light)] bg-[var(--bg-surface-light)] dark:border-[var(--border-dark)] dark:bg-[var(--bg-surface-dark)]">
        <div class="grid sm:grid-cols-2">
            <!-- Current network status panel -->
            <div class="bg-[var(--bg-surface-light)] p-4 dark:bg-[var(--bg-surface-dark)]">
                <div class="flex items-start gap-3">
                    <!-- WiFi status icon -->
                    <div class="flex h-11 w-11 shrink-0 items-center justify-center rounded border border-[var(--border-light)] bg-[color:color-mix(in_srgb,var(--button-primary-light)_14%,white)] text-[var(--button-primary-light)] dark:border-[var(--border-dark)] dark:bg-[color:color-mix(in_srgb,var(--button-primary-dark)_18%,black)] dark:text-[var(--button-primary-dark)]">
                        <HugeiconsIcon :icon="WifiFullSignalIcon" :stroke-width="1.8" class="h-[18px] w-[18px]" />
                    </div>

                    <!-- Current network info -->
                    <div class="min-w-0 flex-1">
                        <div class="text-[11px] font-semibold uppercase tracking-[0.14em] text-[var(--text-secondary-light)] dark:text-[var(--text-secondary-dark)]">{{ t('pages.wifi.currentNetworkLabel') }}</div>
                        <div class="mt-1 truncate text-base font-semibold text-[var(--text-primary-light)] dark:text-[var(--text-primary-dark)]">{{ props.currentNetworkLabel }}</div>
                    </div>
                </div>
            </div>

            <!-- Detected networks count panel -->
            <div class="border-t border-[var(--border-light)] bg-[var(--bg-surface-light)] p-4 dark:border-[var(--border-dark)] dark:bg-[var(--bg-surface-dark)] sm:border-t-0 sm:border-l">
                <div class="flex items-start gap-3">
                    <!-- Networks count icon -->
                    <div class="flex h-11 w-11 shrink-0 items-center justify-center rounded border border-[var(--border-light)] bg-[color:color-mix(in_srgb,var(--bg-selected-light)_72%,white)] text-[var(--text-primary-light)] dark:border-[var(--border-dark)] dark:bg-[color:color-mix(in_srgb,var(--bg-selected-dark)_78%,black)] dark:text-[var(--text-primary-dark)]">
                        <HugeiconsIcon :icon="InformationCircleIcon" :stroke-width="1.8" class="h-[18px] w-[18px]" />
                    </div>

                    <!-- Networks count info -->
                    <div class="min-w-0 flex-1">
                        <div class="text-[11px] font-semibold uppercase tracking-[0.14em] text-[var(--text-secondary-light)] dark:text-[var(--text-secondary-dark)]">{{ t('pages.wifi.networkCountLabel') }}</div>
                        <div class="mt-1 truncate text-base font-semibold text-[var(--text-primary-light)] dark:text-[var(--text-primary-dark)]">{{ props.scannedNetworksCount }}</div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>