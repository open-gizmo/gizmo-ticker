<script setup>
import { computed, resolveComponent } from 'vue';
import { useRoute } from 'vue-router';
import { Key01Icon, Settings01Icon, WifiFullSignalIcon } from '@hugeicons/core-free-icons';
import { DashboardShell } from '@brumaombra/ui-vintage/dashboard-shell';
import { SidebarMenu, SidebarMenuButton, SidebarMenuItem } from '@brumaombra/ui-vintage/sidebar';
import CryptoLogo from '~/components/crypto/CryptoLogo.vue';
import LanguageSelector from '~/components/LanguageSelector.vue';
import ThemeSelector from '~/components/ThemeSelector.vue';
import { createIconComponent } from '~/composables/useUtils.js';

const { t } = useI18n();
const route = useRoute();
const localePath = useLocalePath();
const nuxtLinkComponent = resolveComponent('NuxtLink');

// Icon components
const wifiSidebarIcon = createIconComponent(WifiFullSignalIcon);
const settingsSidebarIcon = createIconComponent(Settings01Icon);
const apiKeySidebarIcon = createIconComponent(Key01Icon);

// Sidebar sections and items configuration
const sidebarSections = computed(() => ([{
    id: 'control-panel',
    label: t('common.controlPanel'),
    items: [{
        id: 'wifi',
        label: t('nav.wifi.label'),
        description: t('nav.wifi.description'),
        icon: wifiSidebarIcon,
        to: localePath('/wifi'),
        active: route.path === localePath('/wifi')
    }, {
        id: 'settings',
        label: t('nav.settings.label'),
        description: t('nav.settings.description'),
        icon: settingsSidebarIcon,
        to: localePath('/settings'),
        active: route.path === localePath('/settings')
    }, {
        id: 'api-key',
        label: t('nav.apiKey.label'),
        description: t('nav.apiKey.description'),
        icon: apiKeySidebarIcon,
        to: localePath('/api-key'),
        active: route.path === localePath('/api-key')
    }]
}]));

// The currently active sidebar item
const activeItem = computed(() => {
    return sidebarSections.value.flatMap(section => section.items).find(item => item.active) || null;
});
</script>

<template>
    <DashboardShell :title="activeItem?.label || t('common.controlPanel')" :description="activeItem?.description || ''" :sidebar-sections="sidebarSections" :sidebar-link-component="nuxtLinkComponent">
        <!-- Sidebar header slot -->
        <template #sidebar-header>
            <SidebarMenu>
                <SidebarMenuItem>
                    <SidebarMenuButton as-child size="lg" class="rounded">
                        <NuxtLink :to="localePath('/')">
                            <!-- Crypto logo -->
                            <div class="bg-sidebar-primary text-sidebar-primary-foreground flex aspect-square size-8 items-center justify-center rounded">
                                <CryptoLogo class="size-5 rounded" />
                            </div>

                            <!-- Brand name and tagline -->
                            <div class="grid min-w-0 flex-1 text-left text-sm leading-tight">
                                <span class="truncate font-semibold">{{ t('app.title') }}</span>
                                <span class="truncate text-xs text-sidebar-foreground/70">{{ t('common.deviceUi') }}</span>
                            </div>
                        </NuxtLink>
                    </SidebarMenuButton>
                </SidebarMenuItem>
            </SidebarMenu>
        </template>

        <!-- Sidebar footer slot -->
        <template #sidebar-footer>
            <div class="flex items-center gap-2 px-2">
                <!-- Language selector -->
                <LanguageSelector />

                <!-- Theme selector -->
                <ThemeSelector />
            </div>
        </template>

        <!-- Main page slot -->
        <slot />
    </DashboardShell>
</template>