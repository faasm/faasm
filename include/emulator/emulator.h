#pragma once

#include <vector>
#include <string>
#include <proto/faasm.pb.h>

#include <state/State.h>

unsigned int setEmulatedMessage(const message::Message &msg);

void resetEmulator();

void setEmulatorUser(const char *user);

state::State *getEmulatorState();

void setEmulatorState(state::State *state);

std::vector<uint8_t> getEmulatorOutputData();

std::string getEmulatorOutputDataString();

std::string getEmulatorUser();