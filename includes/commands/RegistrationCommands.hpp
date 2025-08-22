#pragma once

#include <string>
#include <vector>

// Forward declarations
class Client;
//class Channel;
class Server;

// This macro defines all Server command methods
#define REGISTRATION_COMMAND_METHODS \
	void NICK(std::string nickname, int fd); \
	void USER(std::string nickname, int fd); \ //TODO
	void PASS(std::string nickname, int fd);  //TODO

