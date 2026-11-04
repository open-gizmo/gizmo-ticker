// The request wrapper (fetchEsp) and the Wi-Fi and device calls come from the gizmo-common layer: only the calls of the ticker are here

// Fetch the full saved device config
export const getConfig = async () => {
    const response = await fetchEsp('/api/config');
    return response.data || {};
};

// Persist the current device settings
export const saveSettings = async settings => {
    // Call the device API to save the settings
    await fetchEsp('/api/settings', {
        method: 'POST',
        body: settings
    });

    // Return result status
    return true;
};

// Reset the saved device settings and restart the firmware
export const resetSettings = async () => {
    // Call the device API to reset the settings (POST, so other pages can't trigger it with a simple link)
    const response = await fetchEsp('/api/reset-settings', { method: 'POST' });

    // Return the response data
    return response.data || {};
};

// Save the market data API key on the device
export const saveApiKey = async apiKey => {
    // Call the device API to save the API key
    await fetchEsp('/api/api-key', {
        method: 'POST',
        body: {
            apiKey
        }
    });

    // Return result status
    return true;
};