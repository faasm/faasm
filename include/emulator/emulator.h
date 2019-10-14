#pragma once

std::vector<uint8_t> getEmulatorOutputData();

void resetEmulator();

void setEmulatorInputData(const std::vector<uint8_t> &inputIn);

std::string getEmulatorUser();

void setEmulatorUser(const char* user);

std::string getEmulatorFunction();

void setEmulatorFunction(const char *newFunction);

void unsetEmulatorUser();
