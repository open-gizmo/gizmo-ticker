import { createMockError, createSuccessResponse, getEspMockState, handleEspMockError } from '~~/server/utils/esp-mock-state.js';

const isBooleanOrNullish = value => value == null || typeof value === 'boolean';
const isLevelOrNullish = value => value == null || (Number.isInteger(value) && Number(value) >= 0 && Number(value) <= 15);

export default defineEventHandler(async event => {
    try {
        // Get the request body
        const body = await readBody(event);

        if (!body || typeof body !== 'object' || Array.isArray(body)) {
            throw createMockError(400, 'invalid_json', 'Invalid JSON object');
        }

        const record = body;
        const booleanFields = ['currentPrice', 'priceChange', 'marketCap', 'dailyHighLow', 'yearHighLow', 'openPrice', 'volume'];

        // Validate boolean fields
        for (const field of booleanFields) {
            if (!isBooleanOrNullish(record[field])) {
                throw createMockError(400, 'invalid_settings_field', field);
            }
        }

        // Validate string enum fields
        if (record.cryptoCoin != null && record.cryptoCoin !== 'bitcoin' && record.cryptoCoin !== 'kaspa') {
            throw createMockError(400, 'invalid_settings_field', 'cryptoCoin');
        }

        if (record.formatType != null && record.formatType !== 'US' && record.formatType !== 'EU') {
            throw createMockError(400, 'invalid_settings_field', 'formatType');
        }

        // Validate numeric fields
        if (!isLevelOrNullish(record.matrixIntensity)) {
            throw createMockError(400, 'invalid_settings_field', 'matrixIntensity');
        }

        if (!isLevelOrNullish(record.scrollSpeed)) {
            throw createMockError(400, 'invalid_settings_field', 'scrollSpeed');
        }

        // Persist the updated config in mock state
        const state = getEspMockState();
        state.config = {
            ...state.config,
            ...Object.fromEntries(Object.entries(record).filter(([, value]) => value != null))
        };

        // Return the firmware success shape
        return createSuccessResponse();
    } catch (error) {
        handleEspMockError({ error, errorId: 'mock_settings_save_failed', errorMessage: 'Failed to save mock settings' });
    }
});