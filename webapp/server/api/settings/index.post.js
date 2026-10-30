import { EspMockError, createSuccessResponse, getEspMockState, handleEspMockError, readMockJsonBody } from '~~/server/utils/esp-mock-state.js';

// Fields the firmware reads, anything else in the body is ignored
const booleanFields = ['currentPrice', 'priceChange', 'marketCap', 'dailyHighLow', 'yearHighLow', 'openPrice', 'volume'];
const levelFields = ['matrixIntensity', 'scrollSpeed'];
const enumFields = { cryptoCoin: ['bitcoin', 'kaspa'], formatType: ['US', 'EU'] };
const settingsFields = [...booleanFields, ...Object.keys(enumFields), ...levelFields];

// Check one field like the firmware (missing fields keep their current value)
const isValidField = (field, value) => {
    if (value == null) return true;
    if (booleanFields.includes(field)) return typeof value === 'boolean';
    if (levelFields.includes(field)) return Number.isInteger(value) && value >= 0 && value <= 15;
    return enumFields[field].includes(value);
};

export default defineEventHandler(async event => {
    try {
        // Get the request body
        const body = await readMockJsonBody(event);

        // Validate the fields in the firmware order
        const invalidField = settingsFields.find(field => !isValidField(field, body[field]));
        if (invalidField) {
            throw new EspMockError(400, 'invalid_settings_field', invalidField);
        }

        // Apply only the settings fields that are present
        const { config } = getEspMockState();
        for (const field of settingsFields) {
            if (body[field] != null) config[field] = body[field];
        }

        // Return the firmware success shape
        return createSuccessResponse();
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'settings_save_failed', errorMessage: 'Failed to save settings' });
    }
});
