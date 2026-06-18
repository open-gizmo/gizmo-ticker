<script setup>
import { computed, ref } from 'vue';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Tick02Icon } from '@hugeicons/core-free-icons';
import { Button } from '@brumaombra/ui-vintage/button';
import { Command, CommandGroup, CommandItem, CommandList } from '@brumaombra/ui-vintage/command';
import { Popover, PopoverContent, PopoverTrigger } from '@brumaombra/ui-vintage/popover';
import EnglishFlag from '~/components/flags/EnglishFlag.vue';
import ItalianFlag from '~/components/flags/ItalianFlag.vue';

const isOpen = ref(false);
const { locale, locales, setLocale, t } = useI18n();

// Available language options for the selector
const languageOptions = computed(() => {
    return locales.value.map(language => ({
        key: language.code,
        label: language.name,
        flagCode: language.code === 'en' ? 'gb' : language.code
    }));
});

// Currently selected language metadata
const currentLanguage = computed(() => {
    const currentLanguage = languageOptions.value.find(language => language.key === locale.value);
    return currentLanguage || null;
});

// Get the appropriate flag component based on the language code
const getFlagComponent = flagCode => {
    // Check the flag code
    if (flagCode === 'it') {
        return ItalianFlag;
    } else {
        return EnglishFlag;
    }
};

// Apply a selected language and close the menu
const handleSelectLanguage = async language => {
    await setLocale(language);
    isOpen.value = false;
};
</script>

<template>
    <Popover v-model:open="isOpen">
        <PopoverTrigger as-child>
            <Button variant="outline" size="icon" :aria-label="t('language.toggleMenu')">
                <component :is="getFlagComponent(currentLanguage?.flagCode)" class="h-4 w-[1.35rem] rounded-[2px]" />
            </Button>
        </PopoverTrigger>

        <!-- Menu -->
        <PopoverContent side="bottom" align="end" :side-offset="8" class="w-44 !p-0">
            <Command :model-value="locale">
                <CommandList>
                    <CommandGroup :heading="t('language.title')">
                        <CommandItem v-for="option in languageOptions" :key="option.key" :value="option.key" @select="handleSelectLanguage(option.key)">
                            <component :is="getFlagComponent(option.flagCode)" class="h-4 w-[1.35rem] rounded-[2px]" />
                            <span class="flex-1">{{ option.label }}</span>
                            <HugeiconsIcon :icon="Tick02Icon" :stroke-width="1.8" class="ml-auto h-4 w-4 shrink-0" :class="locale === option.key ? 'opacity-100' : 'opacity-0'" />
                        </CommandItem>
                    </CommandGroup>
                </CommandList>
            </Command>
        </PopoverContent>
    </Popover>
</template>