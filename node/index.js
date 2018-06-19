const wasm = require('./js/wasm/loader');

const funcFile = 'doubler';

const lib = wasm.loadFromFile('functions/' + funcFile).then(lib => {
    let exports = instance.exports;
    let execFunc = exports._execute;

    let result = execFunc(10);
    console.log('Result: ' + result);
});



