"use strict";

// Our C++ Addon
const addon = require('bindings')('node-quirc.node');

// public API
module.exports = {
    decode(img, callback) {
        if (!Buffer.isBuffer(img)) {
            throw new TypeError('img must be a Buffer');
        }
        if (callback) {
            return addon.decode(img, callback);
        } else {
            return new Promise((resolve, reject) => {
                addon.decode(img, (err, results) => {
                    if (err)
                        return reject(err);
                    else
                        return resolve(results);
                });
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
    },
};
