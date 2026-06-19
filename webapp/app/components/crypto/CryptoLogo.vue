<script setup>
import { computed } from 'vue';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const globalStore = useGlobalStore();
const cryptoLogo = computed(() => {
    return globalStore.value.config.cryptoCoin === 'kaspa' ? '/crypto/kaspa.svg' : '/crypto/bitcoin.svg';
});
</script>

<template>
    <img :src="cryptoLogo" alt="" aria-hidden="true" v-bind="$attrs">
</template>