import { EspMockError, MAX_PASSWORD_LENGTH, MAX_SSID_LENGTH, byteLength, createSuccessResponse, getEspMockState, handleEspMockError, readMockJsonBody, wifiStatus } from '~~/server/utils/esp-mock-state.js';

// Validate one credential field like gizmo-common
const validateCredentialField = (fieldName, value, maxLength, allowEmpty) => {
    if (value == null || (!allowEmpty && value === '')) {
        throw new EspMockError(400, 'missing_required_fields', fieldName);
    }

    if (typeof value !== 'string' || byteLength(value) > maxLength) {
        throw new EspMockError(400, 'invalid_connection_field', fieldName);
    }
};

export default defineEventHandler(async event => {
    try {
        // Get the request body
        const body = await readMockJsonBody(event);

        // Validate the credentials (the password can be empty for open networks)
        validateCredentialField('ssid', body.ssid, MAX_SSID_LENGTH, false);
        validateCredentialField('password', body.password, MAX_PASSWORD_LENGTH, true);

        // Start a mock connection attempt (the password "wrong" simulates a failure)
        const state = getEspMockState();
        state.pendingConnection = {
            ssid: body.ssid,
            attemptsRemaining: 2,
            shouldFail: body.password.toLowerCase() === 'wrong'
        };
        state.isFallbackPending = false;
        state.connectionStatus = wifiStatus.CONNECTING;

        // Return the connection status
        return createSuccessResponse({ connectionStatus: state.connectionStatus });
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'mock_connect_failed', errorMessage: 'Failed to start the Wi-Fi connection' });
    }
});
