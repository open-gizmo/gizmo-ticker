import { EspMockError, MAX_API_KEY_LENGTH, byteLength, createSuccessResponse, getEspMockState, handleEspMockError, readMockJsonBody } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Get the request body
        const body = await readMockJsonBody(event);
        const apiKey = body.apiKey;

        // Validate the API key like the firmware (an empty key is accepted and clears it)
        if (apiKey == null) {
            throw new EspMockError(400, 'missing_api_key', 'Missing apiKey field');
        }

        if (typeof apiKey !== 'string') {
            throw new EspMockError(400, 'invalid_api_key', 'Invalid apiKey field');
        }

        if (byteLength(apiKey) > MAX_API_KEY_LENGTH) {
            throw new EspMockError(400, 'invalid_api_key', 'apiKey is too long');
        }

        // Persist the API key in mock state
        getEspMockState().config.apiKey = apiKey;

        // Return the firmware success shape
        return createSuccessResponse();
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'api_key_save_failed', errorMessage: 'Failed to save the API key' });
    }
});
