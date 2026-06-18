<script setup>
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import BitcoinLogo from '~/components/crypto/BitcoinLogo.vue';
import KaspaLogo from '~/components/crypto/KaspaLogo.vue';

const globalStore = useGlobalStore();
</script>

<template>
    <KaspaLogo v-if="globalStore.config.cryptoCoin === 'kaspa'" v-bind="$attrs" />
    <BitcoinLogo v-else v-bind="$attrs" />
</template>