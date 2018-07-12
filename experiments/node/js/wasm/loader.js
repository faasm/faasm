const fs = require('fs');
const util = require('util');

const readFile = util.promisify(fs.readFile);

exports.loadFromFile = async function (filePath) {
    let bytes = await readFile(filePath);

    // Compile the module
    let bytesArray = new Uint8Array(bytes);
    let module = await WebAssembly.compile(bytesArray);

    // Build an instance
    let imports = {
        env: {
            memoryBase: 0,
            tableBase: 0,
            memory: new WebAssembly.Memory({ initial: 256 }),
            table: new WebAssembly.Table({ initial: 0, element: 'anyfunc' })
        }
    };

    let instance = new WebAssembly.Instance(module, imports);

    return instance;
};