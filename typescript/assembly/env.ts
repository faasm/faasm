declare function __faasm_read_input(ptr: Uint8Array, len: i32): i32;
declare function __faasm_write_output(ptr: Uint8Array, len: i32): void;

export namespace faasm {
    export function getInput(): Uint8Array {
        // Get the input length
        let inputLength: i32 = 10;

        let inputData:Uint8Array = new Uint8Array(inputLength);

        // Read into the buffer
        __faasm_read_input(inputData, inputData.byteLength);

        // Return pointer to the input
        return inputData;
    }

    export function setOutput(data:Uint8Array): void {
        __faasm_write_output(data, data.length);
    }
}
