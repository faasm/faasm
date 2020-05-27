#ifndef FAASM_EMULATOR_API_H
#define FAASM_EMULATOR_API_H

unsigned int setEmulatedMessageFromJson(const char *msgJson);

void emulatorSetCallStatus(bool success);

#endif
