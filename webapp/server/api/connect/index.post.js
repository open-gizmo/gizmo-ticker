import { createMockError, createSuccessResponse, getEspMockState, handleEspMockError } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Get the request body
        const body = await readBody(event);
        const ssid = typeof body?.ssid === 'string' ? body.ssid.trim() : '';
        const password = typeof body?.password === 'string' ? body.password : '';

        // Validate WiFi credentials
        if (!ssid) {
            throw createMockError(400, 'missing_required_fields', 'ssid');
        }

        if (ssid.length > 34) {
            throw createMockError(400, 'invalid_connection_field', 'ssid');
        }

        if (password.length > 69) {
            throw createMockError(400, 'invalid_connection_field', 'password');
        }

        // Start a mock connection attempt
        const state = getEspMockState();
        state.pendingConnection = {
            ssid,
            attemptsRemaining: 2,
            shouldFail: password.toLowerCase() === 'wrong'
        };
        state.connectionStatus = 2;

        // Return the initial connection status payload
        return createSuccessResponse({
            connectionStatus: state.connectionStatus
        });
    } catch (error) {
        handleEspMockError({ error, errorId: 'mock_connect_failed', errorMessage: 'Failed to start mock WiFi connection' });
    }
});