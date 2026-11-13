import { beforeEach, describe, expect, it, vi } from 'vitest';
import { setCryptoCoin } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Record the error dialogs instead of rendering them
const dialogs = vi.hoisted(() => []);
vi.mock('@brumaombra/ui-vintage/message-dialog', () => ({
    showMessageDialog: options => {
        dialogs.push(options);
    }
}));

// Errors of the device: handleBackendErrors comes from the gizmo-common layer, which translates its own errors and then the ones of this app
describe('handleBackendErrors', () => {
    // Start without dialogs, and keep the logged errors out of the test output
    beforeEach(() => {
        dialogs.length = 0;
        vi.spyOn(console, 'error').mockImplementation(() => {});
    });

    // It used to throw the error again after the dialog, which ended every failed action with an unhandled error
    it('shows the error without throwing it again', () => {
        const error = Object.assign(new Error('Bad request'), { statusCode: 400, data: { errorId: null, errorMessage: 'scrollSpeed', data: null } });
        expect(() => handleBackendErrors({ error, defaultMessage: 'Could not save', showDialog: true })).not.toThrow();
        expect(dialogs).toHaveLength(1);
    });

    // A known error ID of the layer is shown in the user's words, not as the raw message ("ssid")
    it('translates an error ID of the layer', () => {
        const error = { data: { errorId: 'invalid_connection_field', errorMessage: 'ssid', data: null } };
        handleBackendErrors({ error, showDialog: true });
        expect(dialogs[0].message).toContain('32 characters');
        expect(dialogs[0].type).toBe('error');
    });

    // The refusal of the API key is explained in the words of this app (api.backendErrors), not shown as the raw message
    it('translates an error ID of the app', () => {
        const error = { data: { errorId: 'invalid_api_key', errorMessage: 'apiKey is too long', data: null } };
        handleBackendErrors({ error, showDialog: true });
        expect(dialogs[0].message).toContain('up to 34 characters');
    });

    // Without an error ID, the device message, then the default message, is shown
    it('falls back to the message of the device, then to the default one', () => {
        handleBackendErrors({ error: { data: { errorId: null, errorMessage: 'Device busy' } }, showDialog: true });
        handleBackendErrors({ error: {}, defaultMessage: 'Could not save', showDialog: true });
        expect(dialogs.map(dialog => dialog.message)).toEqual(['Device busy', 'Could not save']);
    });

    // Without showDialog, the error is only logged
    it('shows no dialog unless asked', () => {
        handleBackendErrors({ error: new Error('Offline') });
        expect(dialogs).toHaveLength(0);
        expect(console.error).toHaveBeenCalled();
    });
});

// Coin accent of the web app
describe('setCryptoCoin', () => {
    // An unknown coin from the device falls back to Bitcoin, for the store and the page accent
    it('falls back to Bitcoin for an unknown coin', () => {
        setCryptoCoin('dogecoin');
        expect(useGlobalStore().value.config.cryptoCoin).toBe('bitcoin');
        expect(document.documentElement.dataset.cryptoCoin).toBe('bitcoin');
    });

    // A supported coin is applied
    it('applies a supported coin', () => {
        setCryptoCoin('kaspa');
        expect(useGlobalStore().value.config.cryptoCoin).toBe('kaspa');
        expect(document.documentElement.dataset.cryptoCoin).toBe('kaspa');
    });

    // The layer shows the Gizmo Ticker logo, whatever the coin (sidebar, setup page and error page)
    it('keeps the Gizmo Ticker logo for every coin', () => {
        setCryptoCoin('kaspa');
        expect(useAppConfig().gizmo.logo).toBe('/images/logo/gizmo-ticker-icon-filled.svg');
        setCryptoCoin('bitcoin');
        expect(useAppConfig().gizmo.logo).toBe('/images/logo/gizmo-ticker-icon-filled.svg');
    });
});