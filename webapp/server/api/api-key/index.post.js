import { createMockError, createSuccessResponse, getEspMockState, handleEspMockError } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Get the request body
        const body = await readBody(event);
        const apiKey = body?.apiKey;

        if (typeof apiKey !== 'string' || !apiKey.trim()) {
            throw createMockError(400, 'missing_api_key', 'Missing apiKey field');
        }

        if (apiKey.length > 34) {
            throw createMockError(400, 'invalid_api_key', 'apiKey is too long');
        }

        // Persist the API key in mock state
        const state = getEspMockState();
        state.config.apiKey = apiKey.trim();

        // Return the firmware success shape
        return createSuccessResponse();
    } catch (error) {
        handleEspMockError({ error, errorId: 'mock_api_key_save_failed', errorMessage: 'Failed to save mock API key' });
    }
});