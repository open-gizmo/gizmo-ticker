import tailwindcss from '@tailwindcss/vite';

export default defineNuxtConfig({
    srcDir: 'app/',

    ssr: false,

    app: {
        // Global head configuration
        head: {
            title: 'Gizmo Ticker'
        },

        // Global page transition
        pageTransition: {
            name: 'page',
            mode: 'out-in'
        }
    },

    runtimeConfig: {
        public: {
            espBaseUrl: ''
        }
    },

    css: ['~/assets/main.css'],

    modules: [
        '@brumaombra/ui-vintage',
        '@nuxtjs/i18n'
    ],

    i18n: {
        defaultLocale: 'en',
        langDir: 'locales',
        detectBrowserLanguage: false,
        locales: [
            { code: 'en', name: 'English', language: 'en-US', iso: 'en-US', file: 'en.json' },
            { code: 'it', name: 'Italiano', language: 'it-IT', iso: 'it-IT', file: 'it.json' }
        ]
    },

    vite: {
        plugins: [
            tailwindcss()
        ],
        optimizeDeps: {
            include: [
                '@brumaombra/ui-vintage',
                '@hugeicons/vue',
                '@hugeicons/core-free-icons'
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

    compatibilityDate: '2026-06-18'
});