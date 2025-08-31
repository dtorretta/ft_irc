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

// This macro defines all Server command methods
#define SERVER_COMMAND_METHODS \
	/***JOIN Command***/ \
	void	JOIN(std::string cmd, int fd); \
	std::vector<std::pair<std::string, std::string> > SplitJOIN(std::string cmd); \
	void	Channel_Exist(Channel *channel, Client *client, int fd, std::string key, std::string name); \
	void	Channel_Not_Exist(std::string channel_name, Client *client, int fd); \
	/***PART Command***/ \
	void	PART(std::string cmd, int fd); \
	std::vector<std::string> SplitPART(std::string cmd); \
	/***PRIVMSG Command***/ \
	void	PRIVMSG(std::string cmd, int fd); \
	std::vector<std::string> SplitPM(std::string command); \
	/***TOPIC Command***/ \
	void	TOPIC(std::string cmd, int fd); \
	std::vector<std::string> SplitTopic(std::string cmd); \
	/***INVITE Command***/ \
	void	INVITE(std::string cmd, int fd); \
	/***KICK Command***/ \
	void	KICK(std::string cmd, int fd); \
	std::vector<std::string> SplitKICK(std::string cmd); \
	/***QUIT Command***/ \
	void	QUIT(std::string cmd, int fd); \
	std::string	SplitQUIT(std::string cmd); \
	/***MODE Command***/ \
	void	MODE(std::string cmd, int fd); \
	std::vector<std::string> SplitMODE(std::string cmd); \
	bool	isChannelValid(Channel *channel, std::string channel_string, std::string client_nick, int fd); \
	bool	deactivateMode(Client *client,char mode, std::string parameter, Channel *channel); \
	bool	activateMode(Client *client, char mode, std::string parameter, Channel *channel);
