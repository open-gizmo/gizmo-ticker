import { createSuccessResponse, getEspMockState, handleEspMockError, maskApiKey } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async () => {
    try {
        // Get the current mock device state
        const state = getEspMockState();

        // Return the config payload using the firmware response shape
        return createSuccessResponse({
            ...state.config,
            apiKey: maskApiKey(state.config.apiKey)
        });
    } catch (error) {
        handleEspMockError({ error, errorId: 'mock_config_read_failed', errorMessage: 'Failed to read mock config' });
    }
});