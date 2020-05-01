"use strict";

// Our C++ Addon
const addon = require('bindings')('node-quirc.node');

/**
 * @typedef {import('./index').DecodeResult} DecodeResult
 * @typedef {import('./index').DecodeCallback} DecodeCallback
 */

// public API
module.exports = {
    /**
     * @param {Buffer} img
     * @param {DecodeCallback=} callback 
     * @returns {Promise<DecodeResult> | undefined}
     */
    decode(img, callback) {
        if (!Buffer.isBuffer(img)) {
            throw new TypeError('img must be a Buffer');
        }
        if (callback) {
            return addon.decode(img, callback);
        } else {
            return new Promise((resolve, reject) => {
                callback = (err, results) => {
                    if (err) {
                        return reject(err);
                    } else {
                        return resolve(results);
                    }
                };
                addon.decode(img, callback);
            });
        }
    },
    constants: {
        // QR-code versions.
        VERSION_MIN:  1,
        VERSION_MAX: 40,
        // QR-code ECC levels.
        ECC_LEVEL_M: "M",
        ECC_LEVEL_L: "L",
        ECC_LEVEL_H: "H",
        ECC_LEVEL_Q: "Q",
        // QR-code encoding modes.
        MODE_NUMERIC: "NUMERIC",
        MODE_ALNUM:   "ALNUM",
        MODE_BYTE:    "BYTE",
        MODE_KANJI:   "KANJI",
        // Common character encodings
        ECI_ISO_8859_1:  "ISO_8859_1",
        ECI_IBM437:      "IBM437",
        ECI_ISO_8859_2:  "ISO_8859_2",
        ECI_ISO_8859_3:  "ISO_8859_3",
        ECI_ISO_8859_4:  "ISO_8859_4",
        ECI_ISO_8859_5:  "ISO_8859_5",
        ECI_ISO_8859_6:  "ISO_8859_6",
        ECI_ISO_8859_7:  "ISO_8859_7",
        ECI_ISO_8859_8:  "ISO_8859_8",
        ECI_ISO_8859_9:  "ISO_8859_9",
        ECI_WINDOWS_874: "WINDOWS_874",
        ECI_ISO_8859_13: "ISO_8859_13",
        ECI_ISO_8859_15: "ISO_8859_15",
        ECI_SHIFT_JIS:   "SHIFT_JIS",
        ECI_UTF_8:       "UTF_8",
    },
};
