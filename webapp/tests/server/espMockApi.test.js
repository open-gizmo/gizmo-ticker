import { IncomingMessage, ServerResponse } from 'node:http';
import { Socket } from 'node:net';
import { beforeEach, describe, expect, it, vi } from 'vitest';
import { createEvent } from 'h3';

// Stub the Nitro globals the routes of this app use before the handler modules load (the mock of the gizmo-common layer imports its h3 functions, so it runs on real h3 events)
vi.hoisted(() => {
    globalThis.defineEventHandler = handler => handler;
    globalThis.defineNitroPlugin = plugin => plugin;
    globalThis.useAppConfig = () => ({ gizmo: { name: 'Gizmo Ticker' } });
});

// Load the handlers of the mock device API
const { default: getConfig } = await import('../../server/api/config/index.get.js');
const { default: postSettings } = await import('../../server/api/settings/index.post.js');
const { default: postApiKey } = await import('../../server/api/api-key/index.post.js');
const { default: postResetSettings } = await import('../../server/api/reset-settings/index.post.js');
const { getEspMockState } = await import('../../server/utils/esp-mock-state.js');
const { default: nameMockDevice } = await import('../../server/plugins/gizmo-mock.js');
const { buildDevicePayload, resetGizmoMockState } = await import('gizmo-common/web/server/utils/gizmo-mock.js');

// Build a real h3 request event, with a JSON body when one is given (a string is sent as it is)
const buildEvent = (body = undefined) => {
    const request = new IncomingMessage(new Socket());
    request.method = 'POST';
    request.rawBody = Buffer.from(body === undefined ? '' : (typeof body === 'string' ? body : JSON.stringify(body)));
    return createEvent(request, new ServerResponse(request));
};

// Call a handler and return its answer with the status code it set
const callHandler = async (handler, event = buildEvent()) => {
    const response = await handler(event);
    return { statusCode: event.node.res.statusCode, response };
};

// Start every test from the demo device the dev server boots with
beforeEach(() => {
    resetGizmoMockState();
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

    // Modules come in whole 4-in-1 boards, like the firmware checks them
    it('accepts only whole boards of matrix modules', async () => {
        for (const matrixModules of [0, 6, 36, 8.5]) {
            const { statusCode, response } = await callHandler(postSettings, buildEvent({ matrixModules }));
            expect(statusCode).toBe(400);
            expect(response.errorMessage).toBe('matrixModules');
        }
        await callHandler(postSettings, buildEvent({ matrixModules: 12 }));
        expect(getEspMockState().config.matrixModules).toBe(12);
    });

    // Bodies are checked like gizmo-common reads them
    it('refuses invalid, non-object and oversized bodies', async () => {
        const invalidJson = await callHandler(postSettings, buildEvent('{nope'));
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

// The Wi-Fi routes (/api/networks, /api/connect, /api/check-connection) and the device routes come from the gizmo-common layer, which tests them
describe('mock Wi-Fi state shared with the layer', () => {
    // The config endpoint reads the saved network of the layer's mock, so a network saved by the layer's routes shows up here
    it('shows the network saved by the Wi-Fi routes of the layer', async () => {
        getEspMockState().savedSsid = 'Warehouse Guest';
        const { response } = await callHandler(getConfig);
        expect(response.data.ssid).toBe('Warehouse Guest');
    });

    // The ticker adds its settings to the state of the layer once, and keeps them
    it('keeps the settings of the ticker in the state of the layer', async () => {
        await callHandler(postSettings, buildEvent({ scrollSpeed: 3 }));
        expect(getEspMockState().config.scrollSpeed).toBe(3);
        expect(getEspMockState()).toBe(getEspMockState());
    });

    // A reset brings back the firmware defaults, with no saved network and the hotspot open
    it('resets like a device back to factory settings', async () => {
        await callHandler(postResetSettings);
        const { response: configResponse } = await callHandler(getConfig);
        const device = buildDevicePayload({ state: getEspMockState() });
        expect(configResponse.data).toMatchObject({ apiKey: '', ssid: '', matrixIntensity: 1, scrollSpeed: 1, matrixModules: 4, marketCap: true });
        expect(device).toMatchObject({ ssid: '', accessPointActive: true });
        expect(device.ip).toBeUndefined();
    });
});

// The mock device introduces itself as the app (the plugin reads the name from the app.config.ts)
describe('mock device name', () => {
    // The device card of the layer shows the name the plugin gave
    it('names the mock device like the app', () => {
        nameMockDevice();
        expect(buildDevicePayload({ state: getEspMockState() }).name).toBe('Gizmo Ticker');
    });
});