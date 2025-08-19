
#pragma once

#include <string>
#include <vector>

// Forward declarations
class Client;
class Channel;
class Server;

/*******************/
/* Server Commands */
/*******************/

// Command function pointer type
//typedef void (Server::*CommandHandler)(std::string cmd, int fd);

// This macro defines all Server command methods
// Include this inside the Server class definition
#define SERVER_COMMAND_METHODS \
	/* JOIN Command */ \
	void	JOIN(std::string cmd, int fd); \
	std::vector<std::pair<std::string, std::string> > SplitJOIN(std::string cmd); \
	void	Channel_Exist(Channel *channel, Client *client, int fd, std::string key, std::string name); \
	void	Channel_Not_Exist(std::string channel_name, Client *client, int fd); \
	/* PART Command */ \
	void	PART(std::string cmd, int fd); \
	std::vector<std::string> SplitPART(std::string cmd); \
	/* PRIVMSG Command */ \
	void	PRIVMSG(std::string cmd, int fd); \
	std::vector<std::string> SplitPM(std::string command);
