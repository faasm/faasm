const wasm = require('./js/wasm/loader');

const lib = wasm.loadFromFile('./program.wasm').then(lib => {
    // Construct the state
    var stateStart = wasmInstance.exports.getState();

    // Read in the current state
    var stateMemory = new Uint32Array(wasmInstance.exports.memory.buffer, stateStart, 10);

    // Modify
    wasmInstance.exports.modifyState(10);

    // Print modified state
    for (var i = 0; i < linearMemory.length; i++) {
        log(linearMemory[i]);
    }
})



