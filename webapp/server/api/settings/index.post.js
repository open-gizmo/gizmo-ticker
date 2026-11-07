import { EspMockError, createSuccessResponse, handleEspMockError, readMockJsonBody, scheduleMockRestart } from 'gizmo-common/web/server/utils/gizmo-mock.js';
import { getEspMockState } from '~~/server/utils/esp-mock-state.js';

// Fields the firmware reads, anything else in the body is ignored
const booleanFields = ['currentPrice', 'priceChange', 'marketCap', 'dailyHighLow', 'yearHighLow', 'openPrice', 'volume'];
const levelFields = ['matrixIntensity', 'scrollSpeed'];
const enumFields = { cryptoCoin: ['bitcoin', 'kaspa'], formatType: ['US', 'EU'] };
const moduleFields = ['matrixModules'];
const settingsFields = [...booleanFields, ...Object.keys(enumFields), ...levelFields, ...moduleFields];

// Check one field like the firmware (missing fields keep their current value)
const isValidField = (field, value) => {
    if (value == null) return true;
    if (booleanFields.includes(field)) return typeof value === 'boolean';
    if (levelFields.includes(field)) return Number.isInteger(value) && value >= 0 && value <= 15;
    if (moduleFields.includes(field)) return Number.isInteger(value) && value >= 4 && value <= 32 && value % 4 === 0; // Whole 4-in-1 boards
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
        const state = getEspMockState();
        const previousModules = state.config.matrixModules;
        for (const field of settingsFields) {
            if (body[field] != null) state.config[field] = body[field];
        }

        // The firmware restarts to set the display up for a new number of modules
        if (state.config.matrixModules !== previousModules) {
            scheduleMockRestart(state);
        }

        // Return the firmware success shape
        return createSuccessResponse();
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'settings_save_failed', errorMessage: 'Failed to save settings' });
    }
});
