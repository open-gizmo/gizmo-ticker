<script setup>
import { HugeiconsIcon } from '@hugeicons/vue';

// Props
const props = defineProps({
    eyebrow: { type: String, required: true },
    title: { type: String, required: true },
    icon: { type: Object, required: true },
    iconLabel: { type: String, default: '' }
});
</script>

<template>
    <div class="flex items-center gap-3 sm:gap-4">
        <!-- Leading icon -->
        <div class="flex h-12 w-12 shrink-0 items-center justify-center rounded border border-[var(--border-light)] bg-[var(--bg-surface-light)] dark:border-[var(--border-dark)] dark:bg-[var(--bg-surface-dark)] sm:h-14 sm:w-14">
            <HugeiconsIcon :icon="props.icon"
                :aria-label="props.iconLabel || undefined"
                role="img"
                :stroke-width="1.8"
                class="h-6 w-6" />
        </div>

        <!-- Title copy -->
        <div>
            <div class="text-xs font-semibold uppercase tracking-[0.24em] text-[var(--text-secondary-light)] dark:text-[var(--text-secondary-dark)]">{{ props.eyebrow }}</div>
            <h1 class="mt-1 text-lg sm:text-2xl font-bold">{{ props.title }}</h1>
        </div>
    </div>
</template>