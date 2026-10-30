import { createSuccessResponse, getEspMockState, handleEspMockError, maskApiKey } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Get the current mock device state
        const state = getEspMockState();

        // Return the settings, the saved network and the masked API key, like the firmware
        return createSuccessResponse({
            ...state.config,
            ssid: state.savedSsid,
            apiKey: maskApiKey(state.config.apiKey)
        });
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'mock_config_read_failed', errorMessage: 'Failed to read the config' });
    }
});
