import { createSuccessResponse, handleEspMockError, resetEspMockState } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Clear the settings and the saved network (the device then restarts with its hotspot open)
        resetEspMockState();

        // Return the firmware success payload
        return createSuccessResponse({ message: 'Settings cleared. Restarting...' });
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'clear_settings_failed', errorMessage: 'Failed to clear the settings' });
    }
});
