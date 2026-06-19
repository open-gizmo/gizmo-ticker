<script setup>
import { computed } from 'vue';
import { HugeiconsIcon } from '@hugeicons/vue';
import { ArrowLeft01Icon } from '@hugeicons/core-free-icons';
import BackgroundGrid from '@brumaombra/ui-vintage/background-grid';
import { Button } from '@brumaombra/ui-vintage/button';
import { Card, CardContent, CardFooter } from '@brumaombra/ui-vintage/card';
import LanguageSelector from '@brumaombra/ui-vintage/language-selector';
import ThemeSelector from '@brumaombra/ui-vintage/theme-selector';
import BrandLogo from '~/components/BrandLogo.vue';

// Props
const props = defineProps({
    error: { type: Object, default: () => ({}) }
});

const { locale, locales, setLocale, t } = useI18n();
const localePath = useLocalePath();

// Supported locale codes for the shared language selector
const availableLanguages = computed(() => {
    return locales.value.map(language => language.code);
});

// Resolve the displayed status code
const statusCode = computed(() => {
    return props.error?.statusCode ?? 500;
});

// Resolve the page title
const title = computed(() => {
    if (statusCode.value === 404) {
        return t('errors.pageNotFound');
    }

    return t('errors.genericTitle');
});

// Resolve the supporting message
const message = computed(() => {
    if (statusCode.value === 404) {
        return t('errors.pageNotFoundMessage');
    }

    return props.error?.statusMessage || t('errors.genericMessage');
});

// Clear the error and return to the main setup flow
const handleBackHome = async () => {
    await clearError({ redirect: localePath('/wifi') });
};

// Apply the selected locale from the shared language selector
const handleSelectLanguage = async language => {
    await setLocale(language);
};
</script>

<template>
    <div class="relative min-h-screen bg-[var(--bg-main-light)] text-[var(--text-primary-light)] dark:bg-[var(--bg-main-dark)] dark:text-[var(--text-primary-dark)]">
        <!-- Decorative background grid -->
        <BackgroundGrid />

        <!-- Main content -->
        <main class="relative z-10 mx-auto flex min-h-screen w-full max-w-6xl items-center justify-center px-4 py-10 sm:px-6 lg:px-8">
            <div class="w-full max-w-xl">
                <!-- Toolbar -->
                <div class="mb-4 flex justify-end gap-3">
                    <LanguageSelector :model-value="locale" :languages="availableLanguages" @update:model-value="handleSelectLanguage" />
                    <ThemeSelector />
                </div>

                <!-- Error card -->
                <Card>
                    <CardContent>
                        <!-- Brand -->
                        <BrandLogo :brand-name="t('app.title')" />

                        <!-- Error copy -->
                        <div>
                            <!-- Status code -->
                            <div class="text-sm font-semibold uppercase tracking-[0.24em] text-[var(--button-primary-light)] dark:text-[var(--button-primary-dark)]">
                                {{ t('errors.label') }} {{ statusCode }}
                            </div>

                            <!-- Title -->
                            <h1 class="mt-3 text-2xl font-bold">
                                {{ title }}
                            </h1>

                            <!-- Message -->
                            <p class="mt-4 text-sm leading-7 text-[var(--text-secondary-light)] dark:text-[var(--text-secondary-dark)] sm:text-base">
                                {{ message }}
                            </p>
                        </div>
                    </CardContent>

                    <!-- Actions -->
                    <CardFooter class="items-start flex-col gap-3 sm:flex-row sm:items-center">
                        <!-- Back home button -->
                        <Button variant="primary" @click="handleBackHome">
                            <HugeiconsIcon :icon="ArrowLeft01Icon" class="size-4" />
                            {{ t('errors.backToSetup') }}
                        </Button>
                    </CardFooter>
                </Card>
            </div>
        </main>
    </div>
</template>