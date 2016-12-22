const fs = require('fs');

let raw;

if (process.platform === 'win32') {
  const polarr_rf = require('./dcraw_bundle.js')
  raw = {
    extract: (input, output, callback) => {
      raw.extractThumb(input, output, callback)
    },
    extractThumb: (input, output, callback) => {
      const done = (buffer) => {
        callback(null, new Buffer(buffer))
      }
      const error = (err) => {
        callback({message: "status.raw_failed"})
      }

      fs.readFile(input, (err, data) => {
        if (err) return error(err)
        let filename = input.split('/').pop()
        polarr_rf(filename, new Uint8Array(data.buffer), ['-e', '/' + filename], done, error)
      });

    }
  }
} else {
  raw = require('./build/Release/node_libraw');
}

const libraw = {
  extract: function(input, callback) {
    raw.extract(input, "", function(err, data, width, height) {
      if (err) callback(err);
      callback(null, data, width, height);
    });
  },

  extractThumb: function(input, output) {
    return new Promise(function(resolve, reject) {
      fs.access(input, fs.R_OK | fs.W_OK, function(err) {
        if (err)
          reject(err);

        else
          raw.extractThumb(input, output, function(err, output) {
            if (err)
              reject(err);
            else
              resolve(output);
          });
      });
    });
  }
};

module.exports = libraw;
