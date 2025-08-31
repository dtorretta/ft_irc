#pragma once

#include <string>
#include <vector>

// Forward declarations
class Client;
class Channel;
class Server;

/*************************/
/* Registration Commands */
/*************************/

// This macro defines all registration command methods
#define REGISTRATION_COMMAND_METHODS \
	void NICK(std::string cmd, int fd); \
	void USER(std::string cmd, int fd); \
	void PASS(std::string cmd, int fd);
