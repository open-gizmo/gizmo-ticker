import ChartUpIcon from '@hugeicons/core-free-icons/ChartUpIcon';
import CpuIcon from '@hugeicons/core-free-icons/CpuIcon';
import Key01Icon from '@hugeicons/core-free-icons/Key01Icon';

// Name, logo and pages of the app, read by the layout and the pages of the gizmo-common layer (the logo is the Gizmo Ticker one, whatever the coin)
export default defineAppConfig({
    gizmo: {
        name: 'Gizmo Ticker',

        // mDNS name of the firmware (http://gizmo-ticker.local)
        hostname: 'gizmo-ticker',

        logo: '/images/logo/gizmo-ticker-mark.svg',
        logoDark: '/images/logo/gizmo-ticker-mark-dark.svg',
        homePath: '/ticker',

        // Pages of the dashboard, top to bottom in the sidebar and left to right in the bottom navigation ('wifi' is the page of the layer)
        nav: [
            { id: 'ticker', labelKey: 'nav.ticker.label', descriptionKey: 'nav.ticker.description', icon: ChartUpIcon, to: '/ticker' },
            { id: 'api-key', labelKey: 'nav.apiKey.label', descriptionKey: 'nav.apiKey.description', icon: Key01Icon, to: '/api-key' },
            { id: 'device', labelKey: 'nav.device.label', descriptionKey: 'nav.device.description', icon: CpuIcon, to: '/device' },
            'wifi'
        ]
    }
});