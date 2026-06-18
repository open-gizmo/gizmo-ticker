import tailwindcss from '@tailwindcss/vite';

export default defineNuxtConfig({
    srcDir: 'app/',

    ssr: false,

    app: {
        head: {
            title: 'Bitcoin Ticker'
        }
    },

    runtimeConfig: {
        public: {
            espBaseUrl: ''
        }
    },

    css: [
        '~/assets/main.css'
    ],

    modules: [
        '@nuxtjs/i18n'
    ],

    i18n: {
        defaultLocale: 'en',
        strategy: 'no_prefix',
        langDir: 'locales',
        detectBrowserLanguage: {
            useCookie: true,
            cookieKey: 'bitcoin_ticker_locale',
            alwaysRedirect: false,
            redirectOn: 'root',
            fallbackLocale: 'en'
        },
        locales: [
            { code: 'en', name: 'English', language: 'en-US', file: 'en.json' },
            { code: 'it', name: 'Italiano', language: 'it-IT', file: 'it.json' }
        ]
    },

    vite: {
        plugins: [
            tailwindcss()
        ],
        optimizeDeps: {
            include: [
                '@hugeicons/vue',
                '@hugeicons/core-free-icons',
                '@brumaombra/ui-vintage/busy-indicator',
                '@brumaombra/ui-vintage/background-grid',
                '@brumaombra/ui-vintage/button',
                '@brumaombra/ui-vintage/card',
                '@brumaombra/ui-vintage/message-dialog',
                '@brumaombra/ui-vintage/command',
                '@brumaombra/ui-vintage/popover',
                '@brumaombra/ui-vintage/dashboard-shell',
                '@brumaombra/ui-vintage/sidebar',
                '@brumaombra/ui-vintage/alert',
                '@brumaombra/ui-vintage/input'
            ]
        }
    },

    devtools: {
        enabled: false
    },

    nitro: {
        prerender: {
            routes: [
                '/',
                '/ap-setup',
                '/wifi',
                '/settings',
                '/api-key'
            ]
        }
    },

    compatibilityDate: '2026-03-16'
});