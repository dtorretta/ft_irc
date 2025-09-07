#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <csignal>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "Client.hpp"
#include "Channel.hpp"
#include "../commands/ChannelCommands.hpp"
#include "../commands/RegistrationCommands.hpp"
#include "../utils/messages.hpp"

#define GREEN	"\033[32m"
#define RED  	"\033[31m"
#define BLUE	"\033[36m"
#define YELLOW	"\033[0;33m"
#define MAGENTA "\033[35m"
#define ORANGE  "\033[38;2;255;165;0m"
#define FORM    "\033[4m"
#define RESET	"\033[0m"

class Client;
class Channel;


class Server
{
	public:
		Server(int port, std::string pass); // Constructor
		Server(Server const &copy); // Copy constructor
		Server& operator=(Server const &copy); // Copy assignment operator
		~Server(); // Destructor

		/******************/
		/*     Methods    */
		/******************/
		void init();
		void execute();
		void NewClient();
		void NewData(int clientFd);
		void parser(const std::string &command, int fd);
		std::vector<std::string> split_receivedBuffer(std::string buffer);


		/******************/
		/*     Getters    */
		/******************/
		Client* get_client(int fd);
		Client *get_clientNick(std::string nickname);
		Channel* get_channelByName(const std::string& name);


		/******************/
		/*      Utils     */
		/******************/
		static void signalHandler(int sig);
		std::vector<std::string> split_cmd(std::string &cmd);
		void _sendResponse(std::string response, int fd);
		bool isregistered(int fd); //old name: notregistered
		void ft_close(int Fd);
		void RemoveFd(int Fd);
		void RemoveClient(int clientFd);
		void RemoveClientFromChannel(int fd);
		void RemoveChannel(std::string &name);
		std::string normalize_param(const std::string &s, bool flag);
		void addChannel(Channel newChannel);


		/******************/
		/*    Commands    */
		/******************/
		typedef void (Server::*CommandHandler)(std::string, int);
		SERVER_COMMAND_METHODS
		REGISTRATION_COMMAND_METHODS

	private:
		static bool _signalRecieved; //old name: Signal
		int _port; //old name: port
		std::string _pass; //old name: password
		int _listeningSocket; //old name: server_fdsocket
		std::vector<struct pollfd> _fds; //old name: fds        //este array incluye todos los Fd de clientes conectados y del _listeningSocket
		std::vector<Client> _clients; //old name: clients   // Manejar la lista de clientes conectados. este array incluye todos los objetos clientes que tienen info
		std::vector<Channel> _channels;
		std::map<std::string, CommandHandler> _registrationCommands;
  		std::map<std::string, CommandHandler> _channelCommands;
};
