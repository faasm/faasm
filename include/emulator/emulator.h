#pragma once

std::vector<uint8_t> getEmulatorOutputData();

std::string getEmulatorOutputDataString();

void resetEmulator();

void setEmulatorInputData(const std::vector<uint8_t> &inputIn);

void setEmulatorFunctionIdx(int idx);

std::string getEmulatorUser();

void setEmulatorUser(const char* user);

std::string getEmulatorFunction();

void setEmulatorFunction(const char *newFunction);

std::string getEmulatorPythonUser();

void setEmulatorPythonUser(const char* user);

std::string getEmulatorPythonFunction();

void setEmulatorPythonFunction(const char *newFunction);

void unsetEmulatorUser();
