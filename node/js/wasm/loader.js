const fs = require('fs');
const util = require('util');

const readFile = util.promisify(fs.readFile);

exports.loadFromFile = async function (filePath) {
    let bytes = await readFile(filePath);

    // Compile the module
    let module = await WebAssembly.compile(new Uint8Array(bytes));

    // Build an instance
    let imports = {};
    let instance = new WebAssembly.Instance(module, imports);

    return instance;
}