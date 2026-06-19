import { createSuccessResponse, getEspMockState, handleEspMockError } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async () => {
    try {
        // Get the current mock device state
        const state = getEspMockState();

        // Advance the mock connection polling state
        if (state.pendingConnection) {
            if (state.pendingConnection.attemptsRemaining > 0) {
                state.pendingConnection.attemptsRemaining -= 1;
                state.connectionStatus = 2;
            } else if (state.pendingConnection.shouldFail) {
                state.connectionStatus = 0;
                state.pendingConnection = null;
            } else {
                state.connectionStatus = 1;
                state.config.ssid = state.pendingConnection.ssid;
                state.pendingConnection = null;
            }
        }

        // Return the connected payload when the mock device is online
        if (state.connectionStatus === 1) {
            return createSuccessResponse({
                connectionStatus: 1,
                ssid: state.config.ssid,
                ip: '192.168.1.42',
                hostname: 'ticker.local'
            });
        }

        // Otherwise return the current connection state
        return createSuccessResponse({
            connectionStatus: state.connectionStatus
        });
    } catch (error) {
        handleEspMockError({ error, errorId: 'mock_connection_status_failed', errorMessage: 'Failed to read mock connection status' });
    }
});