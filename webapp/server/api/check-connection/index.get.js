import { MOCK_HOSTNAME, MOCK_IP, createSuccessResponse, getEspMockState, handleEspMockError, wifiStatus } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Get the current mock device state
        const state = getEspMockState();

        // Advance the mock connection attempt (a couple of polls while connecting, then the result)
        if (state.pendingConnection) {
            if (state.pendingConnection.attemptsRemaining > 0) {
                state.pendingConnection.attemptsRemaining -= 1;
            } else if (state.pendingConnection.shouldFail) {
                // New credentials that fail are dropped, and the device goes back to the saved network later
                state.connectionStatus = wifiStatus.DISCONNECTED;
                state.pendingConnection = null;
                state.isFallbackPending = Boolean(state.savedSsid);
                return createSuccessResponse({ connectionStatus: state.connectionStatus });
            } else {
                // Credentials are saved only once they connect
                state.connectionStatus = wifiStatus.CONNECTED;
                state.savedSsid = state.pendingConnection.ssid;
                state.pendingConnection = null;
            }
        } else if (state.isFallbackPending) {
            // Back on the saved network after a failed attempt
            state.connectionStatus = wifiStatus.CONNECTED;
            state.isFallbackPending = false;
        }

        // Return the network, address and hostname once connected
        if (state.connectionStatus === wifiStatus.CONNECTED) {
            return createSuccessResponse({
                connectionStatus: state.connectionStatus,
                ssid: state.savedSsid,
                ip: MOCK_IP,
                hostname: MOCK_HOSTNAME
            });
        }

        // Otherwise return the current connection state
        return createSuccessResponse({ connectionStatus: state.connectionStatus });
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'mock_connection_status_failed', errorMessage: 'Failed to read the connection status' });
    }
});
