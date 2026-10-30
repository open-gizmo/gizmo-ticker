<script setup>
import { computed } from 'vue';
import { BackgroundGrid } from '@brumaombra/ui-vintage/background-grid';
import { LanguageSelector } from '@brumaombra/ui-vintage/language-selector';
import { PageHeader } from '@brumaombra/ui-vintage/page-header';
import { ThemeSelector } from '@brumaombra/ui-vintage/theme-selector';
import BrandLogo from '~/components/common/BrandLogo.vue';
import WifiNetworkPicker from '~/components/wifi/WifiNetworkPicker.vue';

const { locale, locales, setLocale, t } = useI18n();

// Available language codes for the selector
const languageOptions = computed(() => locales.value.map(language => language.code));

// Apply a selected language
const handleSelectLanguage = async language => {
    await setLocale(language);
};
</script>

<template>
    <div class="relative min-h-dvh bg-background text-foreground">
        <!-- Setup page of the hotspot (no dashboard: the device has no network yet) -->
        <!-- Decorative background grid -->
        <BackgroundGrid />

        <!-- Main content -->
        <main class="relative z-10 mx-auto w-full max-w-2xl space-y-6 px-4 py-6 sm:py-10">
            <!-- Brand, language and theme -->
            <div class="flex items-center justify-between gap-3">
                <BrandLogo :brand-name="t('app.title')" />
                <div class="flex items-center gap-2">
                    <LanguageSelector :model-value="locale" :languages="languageOptions" @select="handleSelectLanguage" />
                    <ThemeSelector />
                </div>
            </div>

            <!-- Title -->
            <PageHeader :title="t('pages.apSetup.title')" size="medium" />

            <!-- Networks found by the device, and the connection -->
            <WifiNetworkPicker :confirm-title="t('pages.apSetup.connectConfirmTitle')" confirm-message-key="pages.apSetup.connectConfirmMessage" />
        </main>
    </div>
</template>