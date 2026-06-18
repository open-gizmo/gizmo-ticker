<script setup>
import CryptoLogo from '~/components/crypto/CryptoLogo.vue';

// Props
const props = defineProps({
    brandName: { type: String, required: true },
    compact: { type: Boolean, default: false }
});
</script>

<template>
    <div class="flex items-center gap-2 sm:gap-3">
        <!-- Brand mark -->
        <CryptoLogo class="h-8 w-8 sm:h-9 sm:w-9" />

        <!-- Brand text -->
        <div v-if="!props.compact">
            <!-- Brand name -->
            <div class="text-base font-bold text-[var(--text-primary-light)] dark:text-[var(--text-primary-dark)] sm:text-lg">
                {{ props.brandName }}
            </div>
        </div>
    </div>
</template>