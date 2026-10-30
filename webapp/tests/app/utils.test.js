import { beforeEach, describe, expect, it, vi } from 'vitest';
import { handleBackendErrors, setCryptoCoin } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Record the error dialogs instead of rendering them
const dialogs = vi.hoisted(() => []);
vi.mock('@brumaombra/ui-vintage/message-dialog', () => ({
    showMessageDialog: options => {
        dialogs.push(options);
    }
}));

// Errors of the device: one dialog, with the translated message when the error ID is known
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

    // A known error ID of the firmware is shown in the user's words, not as the raw message ("ssid")
    it('translates a known error ID of the device', () => {
        const error = { data: { errorId: 'invalid_connection_field', errorMessage: 'ssid', data: null } };
        handleBackendErrors({ error, showDialog: true });
        expect(dialogs[0].message).toContain('32 characters');
        expect(dialogs[0].type).toBe('error');
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
});