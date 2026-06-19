<script setup>
import { computed } from 'vue';
import { getCryptoCoinById } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const globalStore = useGlobalStore();
const cryptoLogo = computed(() => {
    return getCryptoCoinById(globalStore.value.config.cryptoCoin).image;
});
</script>

<template>
    <img :src="cryptoLogo" alt="" aria-hidden="true" v-bind="$attrs">
</template>