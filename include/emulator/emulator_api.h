#ifndef FAASM_EMULATOR_API_H
#define FAASM_EMULATOR_API_H

unsigned int setEmulatedMessageFromJson(const char *msgJson);

char* emulatorGetAsyncResponse();

void emulatorSetCallStatus(int success);

#endif
