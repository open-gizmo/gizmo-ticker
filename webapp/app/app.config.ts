import Key01Icon from '@hugeicons/core-free-icons/Key01Icon';
import Settings01Icon from '@hugeicons/core-free-icons/Settings01Icon';

// Name, logo and pages of the app, read by the layout and the pages of the gizmo-common layer (the logo is the Gizmo Ticker one, whatever the coin)
export default defineAppConfig({
    gizmo: {
        name: 'Gizmo Ticker',

        // mDNS name of the firmware (http://gizmo-ticker.local)
        hostname: 'gizmo-ticker',

        logo: '/images/logo/gizmo-ticker-icon-filled.svg',
        homePath: '/wifi',

        // Pages of the dashboard, top to bottom in the sidebar and left to right in the bottom navigation ('wifi' is the page of the layer)
        nav: [
            'wifi',
            { id: 'settings', labelKey: 'nav.settings.label', descriptionKey: 'nav.settings.description', icon: Settings01Icon, to: '/settings' },
            { id: 'api-key', labelKey: 'nav.apiKey.label', descriptionKey: 'nav.apiKey.description', icon: Key01Icon, to: '/api-key' }
        ]
    }
});