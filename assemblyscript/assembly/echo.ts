import {faasm} from "../faasm/env";

export function _asMain(): i32 {
  var inputData:Uint8Array = faasm.getInput();

  faasm.setOutput(inputData);

  return 0;
}
