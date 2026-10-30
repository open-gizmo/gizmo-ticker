import { createSuccessResponse, handleEspMockError, resetEspMockState } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async () => {
    try {
        // Reset the mock device state to defaults
        resetEspMockState();

        // Return the firmware success payload
        return createSuccessResponse({
            message: 'Settings cleared. Restarting...'
        });
    } catch (error) {
        handleEspMockError({ error, errorId: 'mock_reset_failed', errorMessage: 'Failed to reset mock settings' });
    }
});