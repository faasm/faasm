#ifndef FAASM_EMULATOR_API_H
#define FAASM_EMULATOR_API_H

void setEmulatedMessage(const char *msgJson);

char *emulatorGetCallStatus(unsigned int messageId);

void emulatorSetCallStatus(unsigned int messageId, bool success);

void setEmulatorUser(const char *user);

#endif
