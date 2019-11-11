#pragma once

#include <vector>
#include <string>
#include <proto/faasm.pb.h>

unsigned int setEmulatedMessage(const message::Message &msg);

void resetEmulator();

void setEmulatorUser(const char *user);

std::vector<uint8_t> getEmulatorOutputData();

std::string getEmulatorOutputDataString();

std::string getEmulatorUser();