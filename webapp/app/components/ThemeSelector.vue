<script setup>
import { computed, ref } from 'vue';
import { HugeiconsIcon } from '@hugeicons/vue';
import { ComputerSettingsIcon, Moon02Icon, Sun03Icon, Tick02Icon } from '@hugeicons/core-free-icons';
import { Button } from '@brumaombra/ui-vintage/button';
import { Command, CommandGroup, CommandItem, CommandList } from '@brumaombra/ui-vintage/command';
import { Popover, PopoverContent, PopoverTrigger } from '@brumaombra/ui-vintage/popover';
import { getThemes, setTheme } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const isOpen = ref(false);
const globalStore = useGlobalStore();
const themes = getThemes();
const { t } = useI18n();

// Theme options for the dropdown menu
const themeOptions = computed(() => {
    return themes.map(theme => ({
        key: theme.id,
        label: t(`theme.${theme.id}`),
        icon: getThemeIcon(theme.id)
    }));
});

// Resolve the icon for each theme mode
const getThemeIcon = theme => {
    if (theme === 'light') {
        return Sun03Icon;
    } else if (theme === 'dark') {
        return Moon02Icon;
    } else {
        return ComputerSettingsIcon;
    }
};

// Resolve the icon for the current theme
const currentThemeIcon = computed(() => {
    return getThemeIcon(globalStore.value.themeMode);
});

// Apply the selected theme and close the menu
const handleSelectTheme = theme => {
    setTheme(theme);
    isOpen.value = false;
};
</script>

<template>
    <Popover v-model:open="isOpen">
        <PopoverTrigger as-child>
            <Button variant="outline" size="icon" :aria-label="t('theme.toggleMenu')">
                <HugeiconsIcon :icon="currentThemeIcon" :stroke-width="1.8" class="h-5 w-5" />
            </Button>
        </PopoverTrigger>

        <!-- Theme options -->
        <PopoverContent side="bottom" align="end" :side-offset="8" class="w-44 !p-0">
            <Command :model-value="globalStore.themeMode">
                <CommandList>
                    <CommandGroup :heading="t('theme.title')">
                        <CommandItem v-for="option in themeOptions" :key="option.key" :value="option.key" @select="handleSelectTheme(option.key)">
                            <HugeiconsIcon :icon="option.icon" :stroke-width="1.8" class="h-4 w-4" />
                            <span class="flex-1">{{ option.label }}</span>
                            <HugeiconsIcon :icon="Tick02Icon" :stroke-width="1.8" class="ml-auto h-4 w-4 shrink-0" :class="globalStore.themeMode === option.key ? 'opacity-100' : 'opacity-0'" />
                        </CommandItem>
                    </CommandGroup>
                </CommandList>
            </Command>
        </PopoverContent>
    </Popover>
</template>