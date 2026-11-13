import { describe, expect, it } from 'vitest';
import { defineComponent } from 'vue';
import { mountSuspended, registerEndpoint } from '@nuxt/test-utils/runtime';
import ErrorPage from 'gizmo-common/web/app/error.vue';
import ApSetupPage from 'gizmo-common/web/app/pages/ap-setup.vue';
import { setCryptoCoin } from '~/composables/useUtils.js';

// Items of the navigation, read inside a component (the composable needs the Nuxt context)
const readNavItems = async () => {
    let items;
    await mountSuspended(defineComponent({
        setup() {
            items = useGizmoNavItems();
            return () => null;
        }
    }), { route: '/' });
    return items.value;
};

// Pages the app gets from the gizmo-common layer, set up by the app.config.ts
describe('pages of the layer', () => {
    // The pages of the ticker, in the order the sidebar and the bottom navigation show them
    it('lists the pages of the ticker in the navigation', async () => {
        const items = await readNavItems();
        expect(items.map(item => [item.id, item.label, item.to])).toEqual([
            ['ticker', 'Ticker', '/ticker'],
            ['api-key', 'API Key', '/api-key'],
            ['device', 'Device', '/device'],
            ['wifi', 'WiFi', '/wifi']
        ]);
    });

    // The error page has the words of the ticker: its button goes to the Wi-Fi setup, and the layer's own texts stay
    it('shows the error page with the ticker wording', async () => {
        const wrapper = await mountSuspended(ErrorPage, { props: { error: { statusCode: 404 } } });
        expect(wrapper.text()).toContain('Page not found');
        expect(wrapper.text()).toContain('Back to setup');
        expect(wrapper.text()).toContain('Gizmo Ticker');
    });

    // The setup page of the hotspot shows the name and the logo of the app, whatever the coin chosen on the device
    it('shows the name and the logo on the setup page', async () => {
        registerEndpoint('/api/networks', () => ({ errorId: null, errorMessage: null, data: { networks: [] } }));
        setCryptoCoin('kaspa');
        const wrapper = await mountSuspended(ApSetupPage);
        expect(wrapper.text()).toContain('Gizmo Ticker');
        expect(wrapper.find('img').attributes('src')).toBe('/images/logo/gizmo-ticker-mark.svg');
        setCryptoCoin('bitcoin');
    });
});