import { beforeEach, describe, expect, it, vi } from 'vitest';
import { readBody } from 'h3';
import { flushPromises } from '@vue/test-utils';
import { mountSuspended, registerEndpoint } from '@nuxt/test-utils/runtime';
import SettingsPage from '~/pages/settings.vue';
import ApiKeyPage from '~/pages/api-key.vue';
import { loadDeviceConfig } from '~/composables/useDeviceConfig.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Confirm every confirmation dialog right away (the tests check what is sent, not the dialog)
vi.mock('@brumaombra/ui-vintage/confirm-dialog', () => ({
    showConfirmDialog: ({ onConfirm }) => onConfirm()
}));

// Keep the success and error dialogs out of the page
vi.mock('@brumaombra/ui-vintage/message-dialog', () => ({
    showMessageDialog: () => {}
}));

// Settings of the device, as GET /api/config answers them
const deviceConfig = {
    currentPrice: true,
    priceChange: false,
    marketCap: true,
    dailyHighLow: false,
    yearHighLow: false,
    openPrice: true,
    volume: false,
    cryptoCoin: 'kaspa',
    formatType: 'EU',
    matrixIntensity: 12,
    scrollSpeed: 4,
    ssid: 'Home',
    apiKey: 'CG-***************xyz'
};

// The device API: whether the config answers, and the bodies of the saves
let isConfigAvailable;
let savedBodies;

// Answer like the device, or fail like a device that doesn't answer
const registerDeviceEndpoints = () => {
    registerEndpoint('/api/config', () => {
        if (!isConfigAvailable) throw createError({ statusCode: 500, data: { errorId: null, errorMessage: 'Unreachable', data: null } });
        return { errorId: null, errorMessage: null, data: deviceConfig };
    });
    registerEndpoint('/api/settings', {
        method: 'POST',
        handler: async event => {
            savedBodies.push(await readBody(event));
            return { errorId: null, errorMessage: null, data: null };
        }
    });
    registerEndpoint('/api/api-key', {
        method: 'POST',
        handler: async event => {
            savedBodies.push(await readBody(event));
            return { errorId: null, errorMessage: null, data: null };
        }
    });
};

// Start each test with the store of a page that has not loaded the config yet
beforeEach(() => {
    isConfigAvailable = true;
    savedBodies = [];
    const globalStore = useGlobalStore();
    globalStore.value.configLoaded = false;
    globalStore.value.configLoadFailed = false;
    globalStore.value.config = { ...globalStore.value.config, currentPrice: false, priceChange: false, marketCap: false, matrixIntensity: 0, scrollSpeed: 0 };
    vi.spyOn(console, 'error').mockImplementation(() => {});
    registerDeviceEndpoints();
});

// Loading the config: the settings page may only save what the device sent
describe('loadDeviceConfig', () => {
    // A loaded config replaces the store defaults
    it('loads the settings of the device', async () => {
        await loadDeviceConfig();
        const globalStore = useGlobalStore();
        expect(globalStore.value.configLoaded).toBe(true);
        expect(globalStore.value.config.matrixIntensity).toBe(12);
        expect(globalStore.value.config.cryptoCoin).toBe('kaspa');
    });

    // A failed load is remembered, and the store keeps its defaults marked as not loaded
    it('remembers a failed load', async () => {
        isConfigAvailable = false;
        await expect(loadDeviceConfig()).rejects.toThrow();
        const globalStore = useGlobalStore();
        expect(globalStore.value.configLoaded).toBe(false);
        expect(globalStore.value.configLoadFailed).toBe(true);
    });
});

// Settings page: never a form full of defaults that could be saved over the real settings
describe('settings page', () => {
    // A failed Wi-Fi scan used to skip the config load, and Save then sent everything hidden and intensity 0 to the device
    it('offers to try again instead of the form when the config failed to load', async () => {
        // Fail the load
        isConfigAvailable = false;
        await loadDeviceConfig().catch(() => {});

        // Expect the retry, and no way to save
        const wrapper = await mountSuspended(SettingsPage);
        expect(wrapper.text()).toContain('Couldn\'t read the settings');
        expect(wrapper.find('form').exists()).toBe(false);
        expect(wrapper.findAll('button').some(button => button.text().includes('Save settings'))).toBe(false);
    });

    // Trying again once the device answers shows the real settings
    it('shows the real settings after trying again', async () => {
        // Fail the first load, then let the device answer
        isConfigAvailable = false;
        await loadDeviceConfig().catch(() => {});
        const wrapper = await mountSuspended(SettingsPage);
        isConfigAvailable = true;

        // Try again
        await wrapper.findAll('button').find(button => button.text().includes('Try again')).trigger('click');

        // Expect the form with the values of the device
        await vi.waitFor(() => expect(wrapper.find('form').exists()).toBe(true));
        expect(wrapper.text()).toContain('80%'); // Intensity 12 of 15
    });

    // While the config is still loading, there is no form either
    it('waits for the config before showing the form', async () => {
        const wrapper = await mountSuspended(SettingsPage);
        expect(wrapper.find('form').exists()).toBe(false);
    });

    // Saving sends the settings read from the device
    it('saves the settings of the device', async () => {
        // Load the config and save it as is
        await loadDeviceConfig();
        const wrapper = await mountSuspended(SettingsPage);
        await wrapper.find('form').trigger('submit');

        // Expect the real values in the saved body
        await vi.waitFor(() => expect(savedBodies).toHaveLength(1));
        expect(savedBodies[0]).toMatchObject({ currentPrice: true, marketCap: true, matrixIntensity: 12, scrollSpeed: 4, formatType: 'EU' });
    });
});

// API key page: the key the device stores is the one CoinGecko accepts
describe('api key page', () => {
    // Type a key in the field and submit the form
    const submitApiKey = async (wrapper, apiKey) => {
        await wrapper.find('#apiKey').setValue(apiKey);
        await wrapper.find('form').trigger('submit');
        await flushPromises();
    };

    // A pasted key used to keep its spaces and line breaks, and CoinGecko refused it forever
    it('saves the key without the spaces around it', async () => {
        const wrapper = await mountSuspended(ApiKeyPage);
        await submitApiKey(wrapper, '  CG-pasted-key-123 \n');

        // Expect the trimmed key to be sent, then shown masked once the device answered
        await vi.waitFor(() => expect(savedBodies).toEqual([{ apiKey: 'CG-pasted-key-123' }]));
        await vi.waitFor(() => expect(wrapper.text()).toContain('CG-***********123'));
    });

    // A key of only spaces is not a key
    it('refuses a key made of spaces', async () => {
        const wrapper = await mountSuspended(ApiKeyPage);
        await submitApiKey(wrapper, '    ');
        expect(savedBodies).toEqual([]);
    });

    // The field stops at the 34 characters the device stores
    it('limits the key to what the device stores', async () => {
        const wrapper = await mountSuspended(ApiKeyPage);
        expect(wrapper.find('#apiKey').attributes('maxlength')).toBe('34');
    });

    // After a failed save, the typed key stays in the field to try again
    it('keeps the key in the field when the save fails', async () => {
        // Make the save fail
        registerEndpoint('/api/api-key', {
            method: 'POST',
            handler: () => {
                throw createError({ statusCode: 500, data: { errorId: 'api_key_save_failed', errorMessage: 'Failed', data: null } });
            }
        });

        // Expect the key to still be there
        const wrapper = await mountSuspended(ApiKeyPage);
        await submitApiKey(wrapper, 'CG-key-to-keep');

        // Wait for the failure to be handled, then expect the key to still be there
        await vi.waitFor(() => expect(console.error).toHaveBeenCalled());
        expect(wrapper.find('#apiKey').element.value).toBe('CG-key-to-keep');
    });
});