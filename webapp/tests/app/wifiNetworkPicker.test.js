import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { mountSuspended, registerEndpoint } from '@nuxt/test-utils/runtime';
import WifiNetworkPicker from '~/components/wifi/WifiNetworkPicker.vue';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Record the confirmations instead of showing them
const confirmations = vi.hoisted(() => []);
vi.mock('@brumaombra/ui-vintage/confirm-dialog', () => ({
    showConfirmDialog: options => {
        confirmations.push(options);
    }
}));

// Networks found by the device scan
const scannedNetworks = [
    { ssid: 'Office Fiber', signal: -63, quality: 74, channel: 6, secured: true },
    { ssid: 'Warehouse Guest', signal: -71, quality: 58, channel: 1, secured: false }
];

// The scans the device answered, and what the next ones return
let scanCount;
let scanAnswers;

// Answer the scans in order, then with the last answer
const registerScanEndpoint = () => {
    registerEndpoint('/api/networks', () => {
        const networks = scanAnswers[Math.min(scanCount, scanAnswers.length - 1)];
        scanCount += 1;
        return { errorId: null, errorMessage: null, data: { networks } };
    });
};

// Mount the picker like the Wi-Fi page does
const mountPicker = () => mountSuspended(WifiNetworkPicker, { props: { confirmTitle: 'Switch network', confirmMessageKey: 'pages.wifi.connectConfirmMessage' } });

// Find a network row by its name
const findNetworkRow = (wrapper, ssid) => wrapper.findAll('button').find(button => button.text().includes(ssid));

// Network list of the Wi-Fi pages: the page finds the networks by itself
describe('WifiNetworkPicker', () => {
    // Start with no known network, and the scans answering the networks
    beforeEach(() => {
        confirmations.length = 0;
        scanCount = 0;
        scanAnswers = [scannedNetworks];
        useGlobalStore().value.networksList = [];
        registerScanEndpoint();
    });

    // Close the dialogs between tests
    afterEach(() => {
        document.body.innerHTML = '';
    });

    // The page scans as soon as it opens (the app used to scan once at startup, and a failed scan blocked the config)
    it('scans when it opens', async () => {
        const wrapper = await mountPicker();
        await vi.waitFor(() => expect(wrapper.text()).toContain('Office Fiber'));
        expect(scanCount).toBe(1);
    });

    // The first scan after boot comes back empty while the device is still scanning, so the page tries once more by itself
    it('scans once more by itself after an empty first scan', async () => {
        // Answer the first scan empty, like gizmo-common right after boot
        scanAnswers = [[], scannedNetworks];
        const wrapper = await mountPicker();
        await vi.waitFor(() => expect(scanCount).toBe(1));
        expect(wrapper.text()).toContain('No networks found yet');

        // Expect the networks after the automatic retry, and no more scans
        await vi.waitFor(() => expect(wrapper.text()).toContain('Office Fiber'), { timeout: 5000 });
        expect(scanCount).toBe(2);
    });

    // Known networks need no scan when the page opens
    it('does not scan again when the networks are known', async () => {
        useGlobalStore().value.networksList = scannedNetworks;
        const wrapper = await mountPicker();
        expect(wrapper.text()).toContain('Warehouse Guest');
        expect(scanCount).toBe(0);
    });

    // Open networks used to need a password before the Connect button worked, so they could never be joined
    it('connects to an open network without asking for a password', async () => {
        const wrapper = await mountPicker();
        await vi.waitFor(() => expect(wrapper.text()).toContain('Warehouse Guest'));
        await findNetworkRow(wrapper, 'Warehouse Guest').trigger('click');
        expect(confirmations).toHaveLength(1);
        expect(confirmations[0].message).toContain('Warehouse Guest');
    });

    // Secured networks ask for the password first
    it('asks for the password of a secured network', async () => {
        const wrapper = await mountPicker();
        await vi.waitFor(() => expect(wrapper.text()).toContain('Office Fiber'));
        await findNetworkRow(wrapper, 'Office Fiber').trigger('click');
        await vi.waitFor(() => expect(document.body.textContent).toContain('Join Office Fiber'));
        expect(confirmations).toHaveLength(0);
    });
});