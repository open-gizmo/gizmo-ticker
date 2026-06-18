<script setup>
import { Card, CardContent, CardHeader } from '@brumaombra/ui-vintage/card';
import CardHeaderWithIcon from '~/components/CardHeaderWithIcon.vue';

// Props
const props = defineProps({
    eyebrow: { type: String, required: true },
    title: { type: String, required: true },
    description: { type: String, required: true },
    icon: { type: Object, required: true },
    iconLabel: { type: String, default: '' }
});
</script>

<template>
    <Card>
        <!-- Header with icon -->
        <CardHeader>
            <CardHeaderWithIcon :eyebrow="props.eyebrow"
                :title="props.title"
                :icon="props.icon"
                :icon-label="props.iconLabel" />
        </CardHeader>

        <!-- Description -->
        <CardContent>
            <p class="text-xs leading-5 text-[var(--text-secondary-light)] dark:text-[var(--text-secondary-dark)] sm:text-sm sm:leading-6">
                {{ props.description }}
            </p>
        </CardContent>
    </Card>
</template>