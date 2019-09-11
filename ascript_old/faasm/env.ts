import "../node_modules/assemblyscript/std/assembly/allocator/system"  // Import the system memory allocator

declare function __faasm_read_input(ptr: i32, len: i32): i32;
declare function __faasm_write_output(ptr: i32, len: i32): void;


export namespace faasm {
    export function getInput(): Uint8Array {
        // Get the input length
        var inputLength: i32 = __faasm_read_input(0, 0);

        var inputData:Uint8Array = new Uint8Array(inputLength);

        // Read into the buffer
        __faasm_read_input(inputData.byteOffset, inputData.byteLength);

        // Return pointer to the input
        return inputData;
    }

    export function setOutput(data:Uint8Array): void {
        __faasm_write_output(data.byteOffset, data.byteLength);
    }
}
