"use strict";

// Our C++ Addon
const addon = require('bindings')('node-quirc.node');

/**
 * @typedef {object} QRCode
 * @property {number} version
 * @property {typeof constants["ECC_LEVEL_M"] | typeof constants["ECC_LEVEL_L"] | typeof constants["ECC_LEVEL_H"] | typeof constants["ECC_LEVEL_Q"]} ecc_level
 * @property {number} mask
 * @property {typeof constants["MODE_NUMERIC"] | typeof constants["MODE_ALNUM"] | typeof constants["MODE_BYTE"] | typeof constants["MODE_KANJI"]} mode
 * @property {Buffer} data
 * @property {string=} eci
 *
 * @typedef {object} InvalidQRCode
 * @property {string} err
 * 
 * @typedef {(QRCode | InvalidQRCode)[]} DecodeResult
 * @typedef {(err: Error | null, results?: DecodeResult) => void} DecodeCallback
 * @typedef {{(img: Buffer): Promise<DecodeResult>; (img: Buffer, callback: DecodeCallback): void}} Decode
 */

/**
 * @param {Buffer} img
 * @param {DecodeCallback=} callback
 * @returns {Promise<DecodeResult> | void}
 */
function decode(img, callback) {
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
}

const constants = {
    // QR-code versions.
    VERSION_MIN:  /** @type {1} */(1),
    VERSION_MAX: /** @type {40} */(40),
    // QR-code ECC levels.
    ECC_LEVEL_M: /** @type {"M"} */("M"),
    ECC_LEVEL_L: /** @type {"L"} */("L"),
    ECC_LEVEL_H: /** @type {"H"} */("H"),
    ECC_LEVEL_Q: /** @type {"Q"} */("Q"),
    // QR-code encoding modes.
    MODE_NUMERIC: /** @type {"NUMERIC"} */("NUMERIC"),
    MODE_ALNUM:   /** @type {"ALNUM"} */("ALNUM"),
    MODE_BYTE:    /** @type {"BYTE"} */("BYTE"),
    MODE_KANJI:   /** @type {"KANJI"} */("KANJI"),
    // Common character encodings
    ECI_ISO_8859_1:  /** @type {"ISO_8859_1"} */("ISO_8859_1"),
    ECI_IBM437:      /** @type {"IBM437"} */("IBM437"),
    ECI_ISO_8859_2:  /** @type {"ISO_8859_2"} */("ISO_8859_2"),
    ECI_ISO_8859_3:  /** @type {"ISO_8859_3"} */("ISO_8859_3"),
    ECI_ISO_8859_4:  /** @type {"ISO_8859_4"} */("ISO_8859_4"),
    ECI_ISO_8859_5:  /** @type {"ISO_8859_5"} */("ISO_8859_5"),
    ECI_ISO_8859_6:  /** @type {"ISO_8859_6"} */("ISO_8859_6"),
    ECI_ISO_8859_7:  /** @type {"ISO_8859_7"} */("ISO_8859_7"),
    ECI_ISO_8859_8:  /** @type {"ISO_8859_8"} */("ISO_8859_8"),
    ECI_ISO_8859_9:  /** @type {"ISO_8859_9"} */("ISO_8859_9"),
    ECI_WINDOWS_874: /** @type {"WINDOWS_874"} */("WINDOWS_874"),
    ECI_ISO_8859_13: /** @type {"ISO_8859_13"} */("ISO_8859_13"),
    ECI_ISO_8859_15: /** @type {"ISO_8859_15"} */("ISO_8859_15"),
    ECI_SHIFT_JIS:   /** @type {"SHIFT_JIS"} */("SHIFT_JIS"),
    ECI_UTF_8:       /** @type {"UTF_8"} */("UTF_8"),
};

// public API
module.exports = {
    decode: /** @type {Decode} */(decode),
    constants: constants
};
