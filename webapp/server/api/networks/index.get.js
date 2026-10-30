import { createSuccessResponse, getEspMockState, handleEspMockError, scanNetworks } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Start a scan and return the networks found so far (the first call is empty, like on the device)
        return createSuccessResponse({ networks: scanNetworks(getEspMockState()) });
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'wifi_scan_failed', errorMessage: 'Unable to scan Wi-Fi networks.' });
    }
});
