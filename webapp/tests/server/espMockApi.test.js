import { beforeEach, describe, expect, it, vi } from 'vitest';

// Stub the Nitro globals before the handler modules load (the event carries the raw body and receives the status code)
vi.hoisted(() => {
    globalThis.defineEventHandler = handler => handler;
    globalThis.readRawBody = async event => event.body;
    globalThis.setResponseStatus = (event, statusCode) => {
        event.statusCode = statusCode;
    };
});

// Load the handlers of the mock device API
const { default: getConfig } = await import('../../server/api/config/index.get.js');
const { default: postSettings } = await import('../../server/api/settings/index.post.js');
const { default: postApiKey } = await import('../../server/api/api-key/index.post.js');
const { default: getNetworks } = await import('../../server/api/networks/index.get.js');
const { default: postConnect } = await import('../../server/api/connect/index.post.js');
const { default: getCheckConnection } = await import('../../server/api/check-connection/index.get.js');
const { default: postResetSettings } = await import('../../server/api/reset-settings/index.post.js');

// Build a request event, with a JSON body when one is given
const buildEvent = (body = undefined) => ({ body: body === undefined ? undefined : JSON.stringify(body) });

// Call a handler and return its answer with the status code it set
const callHandler = async (handler, event = buildEvent()) => {
    const response = await handler(event);
    return { statusCode: event.statusCode || 200, response };
};

// Start every test from the demo device the dev server boots with
beforeEach(() => {
    delete globalThis.__gizmoEspMockState;
});

// Response shape: the web app reads the same { errorId, errorMessage, data } as on the device
describe('mock device responses', () => {
    // A success has no error and the data
    it('answers a success like the firmware', async () => {
        const { statusCode, response } = await callHandler(getConfig);
        expect(statusCode).toBe(200);
        expect(response.errorId).toBeNull();
        expect(response.errorMessage).toBeNull();
        expect(response.data.cryptoCoin).toBe('bitcoin');
    });

    // The mock used to send h3 errors, so the app never found the errorId and never showed its translated device errors
    it('answers an error as the firmware body with its status code', async () => {
        const { statusCode, response } = await callHandler(postSettings, buildEvent({ scrollSpeed: 16 }));
        expect(statusCode).toBe(400);
        expect(response).toEqual({ errorId: 'invalid_settings_field', errorMessage: 'scrollSpeed', data: null });
    });

    // Bodies are checked like gizmo-common reads them
    it('refuses invalid, non-object and oversized bodies', async () => {
        const invalidJson = await callHandler(postSettings, { body: '{nope' });
        const arrayBody = await callHandler(postSettings, buildEvent([1]));
        const largeBody = await callHandler(postSettings, buildEvent({ text: 'a'.repeat(2100) }));
        expect([invalidJson.statusCode, invalidJson.response.errorMessage]).toEqual([400, 'Invalid JSON']);
        expect([arrayBody.statusCode, arrayBody.response.errorMessage]).toEqual([400, 'Invalid JSON object']);
        expect([largeBody.statusCode, largeBody.response.errorId]).toEqual([413, 'request_body_too_large']);
    });
});

// Settings and API key
describe('mock settings', () => {
    // The mock used to save any field it got, while the firmware ignores the unknown ones
    it('saves the settings fields and ignores the others', async () => {
        // Send a setting with fields the device doesn't take from this endpoint
        await callHandler(postSettings, buildEvent({ volume: true, apiKey: 'hack', ssid: 'elsewhere' }));

        // Expect only the setting to change
        const { response } = await callHandler(getConfig);
        expect(response.data.volume).toBe(true);
        expect(response.data.ssid).toBe('BrumaLab WiFi');
        expect(response.data.apiKey).toBe('coi****************789');
    });

    // The key is masked like the firmware: 3 characters at each end
    it('stores the API key and returns it masked', async () => {
        await callHandler(postApiKey, buildEvent({ apiKey: 'CG-abcdefghijklmnop' }));
        const { response } = await callHandler(getConfig);
        expect(response.data.apiKey).toBe('CG-*************nop');
    });

    // The device stores up to 34 bytes, and an empty key clears it
    it('checks the API key like the firmware', async () => {
        const tooLong = await callHandler(postApiKey, buildEvent({ apiKey: 'k'.repeat(35) }));
        const notText = await callHandler(postApiKey, buildEvent({ apiKey: 12 }));
        const empty = await callHandler(postApiKey, buildEvent({ apiKey: '' }));
        expect(tooLong.response.errorId).toBe('invalid_api_key');
        expect(notText.response.errorMessage).toBe('Invalid apiKey field');
        expect(empty.statusCode).toBe(200);
    });
});

// Wi-Fi setup
describe('mock Wi-Fi setup', () => {
    // Like gizmo-common, the scan started by the first call is not done yet
    it('answers the first scan empty, then the networks', async () => {
        const firstScan = await callHandler(getNetworks);
        const secondScan = await callHandler(getNetworks);
        expect(firstScan.response.data.networks).toEqual([]);
        expect(secondScan.response.data.networks.length).toBeGreaterThan(0);
    });

    // The SSID is up to 32 bytes and the password up to 64, and the password must be sent (empty for open networks)
    it('checks the credentials like gizmo-common', async () => {
        const longSsid = await callHandler(postConnect, buildEvent({ ssid: 'a'.repeat(33), password: '' }));
        const noPassword = await callHandler(postConnect, buildEvent({ ssid: 'Office Fiber' }));
        expect(longSsid.response).toEqual({ errorId: 'invalid_connection_field', errorMessage: 'ssid', data: null });
        expect(noPassword.response).toEqual({ errorId: 'missing_required_fields', errorMessage: 'password', data: null });
    });

    // Good credentials connect after a few polls and are saved
    it('connects and saves the new network', async () => {
        // Start the attempt
        const { response } = await callHandler(postConnect, buildEvent({ ssid: 'Warehouse Guest', password: '' }));
        expect(response.data.connectionStatus).toBe(2);

        // Poll until the attempt ends
        const statuses = [];
        for (let poll = 0; poll < 3; poll += 1) {
            const { response: pollResponse } = await callHandler(getCheckConnection);
            statuses.push(pollResponse.data.connectionStatus);
        }

        // Expect two polls while connecting, then the connection on the new network
        expect(statuses).toEqual([2, 2, 1]);
        const { response: configResponse } = await callHandler(getConfig);
        expect(configResponse.data.ssid).toBe('Warehouse Guest');
    });

    // Wrong credentials fail once, then the device is back on the saved network
    it('goes back to the saved network after a failed attempt', async () => {
        // Try a wrong password
        await callHandler(postConnect, buildEvent({ ssid: 'Office Fiber', password: 'wrong' }));

        // Poll through the failure
        const statuses = [];
        for (let poll = 0; poll < 4; poll += 1) {
            const { response } = await callHandler(getCheckConnection);
            statuses.push(response.data.connectionStatus);
        }

        // Expect the failure, then the saved network again
        expect(statuses).toEqual([2, 2, 0, 1]);
        const { response } = await callHandler(getCheckConnection);
        expect(response.data.ssid).toBe('BrumaLab WiFi');
    });

    // A reset brings back the firmware defaults, with no saved network and the hotspot open
    it('resets like a device back to factory settings', async () => {
        await callHandler(postResetSettings);
        const { response: configResponse } = await callHandler(getConfig);
        const { response: statusResponse } = await callHandler(getCheckConnection);
        expect(configResponse.data).toMatchObject({ apiKey: '', ssid: '', matrixIntensity: 1, scrollSpeed: 1, marketCap: true });
        expect(statusResponse.data.connectionStatus).toBe(0);
    });
});