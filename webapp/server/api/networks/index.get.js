import { createSuccessResponse, getEspMockState, handleEspMockError } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async () => {
    try {
        // Get the current mock device state
        const state = getEspMockState();

        // Return the scanned networks list
        return createSuccessResponse({
            networks: state.networks
        });
    } catch (error) {
        handleEspMockError({ error, errorId: 'mock_networks_read_failed', errorMessage: 'Failed to read mock networks' });
    }
});