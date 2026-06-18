<script setup>
import { onMounted } from 'vue';

const { t } = useI18n();
const localePath = useLocalePath();

// Redirect the root page to the WiFi screen
onMounted(() => {
    navigateTo(localePath('/wifi'), { replace: true });
});

// Page metadata
definePageMeta({
    layout: 'private'
});
</script>

<template>
    <div class="flex min-h-[50vh] items-center justify-center text-sm text-[var(--text-secondary-light)] dark:text-[var(--text-secondary-dark)]">{{ t('common.redirectingWifiSetup') }}</div>
</template>