[![NPM Version](https://img.shields.io/npm/v/node-quirc.svg)](https://npmjs.org/package/node-quirc)
[![Build Status](https://travis-ci.org/kAworu/node-quirc.svg?branch=master)](https://travis-ci.org/kAworu/node-quirc)

# node-quirc
A Node.js Addon of the quirc library (QR decoder library - https://github.com/dlbeer/quirc).

# installation
First, You need libpng and libjpeg (and their header files) installed. Then, simply
```
% npm install node-quirc
```

# documentation
node-quirc aim to be simple to use, the module exposes a `decode()` function
and a `constants` object.


## decode(img[, callback])
`img` must be a `Buffer` of a PNG or JPEG encoded image file.

When `callback` is provided, it is expected to be a "classic" Node.js callback
function, taking an error as first argument and the result as second argument.
Because the provided image file may contains several QR Code, the result is
always an array on success.

When `decode` is called only with `img` as argument, a `Promise` is returned.

```javascript
const fs    = require("fs");
const quirc = require("node-quirc");

// load the PNG image file content
const img = fs.readFileSync("Hello+World.png");

// callback version
quirc.decode(img, (err, codes) => {
    if (err) {
        // handle err.
        console.error(`decode failed: ${err.message}`);
    } else {
        // do something with codes.
        console.dir(codes);
    }
});

// Promise version
quirc.decode(img).then((codes) => {
        // do something with codes.
        console.dir(codes);
}).catch((err) => {
        // handle err.
        console.error(`decode failed: ${err.message}`);
});

/* output:
[
  {
    version: 1,
    ecc_level: 'L',
    mask: 0,
    mode: 'BYTE',
    data: Buffer [Uint8Array] [ 72, 101, 108, 108, 111 ]
  },
  {
    version: 1,
    ecc_level: 'L',
    mask: 7,
    mode: 'BYTE',
    data: Buffer [Uint8Array] [ 87, 111, 114, 108, 100 ]
  }
]
*/
```

## constants
see https://github.com/kAworu/node-quirc/blob/master/index.js#L25


# testing
Clone the repo and then simply
```
% npm install && npm test
```

# license
MIT, see [LICENSE](./LICENSE).
