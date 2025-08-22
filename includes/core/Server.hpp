#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>
#include <unistd.h> // para close()
#include <fcntl.h>  // para fcntl, F_SETFL, O_NONBLOCK
#include <cstring> //memset
#include <string> //trim
#include <csignal> //signals
#include <poll.h>
#include <netinet/in.h>  // para sockaddr_in en Unix/Linux/macOS
#include <sys/socket.h>  // para funciones socket(), bind(), listen(), etc.
#include <arpa/inet.h>   // para funciones como htons()

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
//class Channel; //🚨 quitar comment out

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
		void parser(std::string &cmd, int fd);
		

		/******************/
		/*     Getters    */
		/******************/
		Client* get_client(int fd) const;
		Channel* get_channelByName(const std::string& name) const;

		/******************/
		/*      Utils     */
		/******************/
		static void signalHandler(int sig);
		std::vector<std::string> split_receivedBuffer(std::string buffer);
		std::vector<std::string> split_cmd(std::string &cmd);
		void _sendResponse(std::string response, int fd);
		bool isregistered(int fd); //old name: notregistered
		void ft_close(int Fd);
		void RemoveFd(int Fd); //si no los uso por fuera de ft_close, eliminarlos del header y agregarlos a utils
		void RemoveClient(int clientFd); //si no los uso por fuera de ft_close, eliminarlos del header y agregarlos a utils
		//void RemoveChannel(std::string &name); //🚨 quitar comment out
		//void RmChannels(int fd); //⚠️ TODO!!!!!!!!!!! //🚨 quitar comment out
		std::string trim(const std::string &s);
		

		/******************/
		/*    Commands    */
		/******************/
		typedef void (Server::*CommandHandler)(std::string, int);
		SERVER_COMMAND_METHODS
		REGISTRATION_COMMAND_METHODS //esto significa que estan incluidos en el heawder se server
	
	private:
		static bool _signalRecieved; //old name: Signal     //signal to finish the execute loop. //como no hay mas de un objeto server a la vez, le quite el static
		int _port; //old name: port  //pasado incialmente como parametro, es el puerto que quieres que tu socket de escucha use (osea, el puerto por el cual el servidor va a aceptar conexiones)
		std::string _pass; //old name: password
		int _listeningSocket; //old name: server_fdsocket   //fd del socket que escucha conexiones y que luego sera enlazado con 'bind' a _port
		std::vector<struct pollfd> _fds; //old name: fds        //este array incluye todos los Fd de clientes conectados y del _listeningSocket
		std::vector<Client> _clients; //old name: clients   // Manejar la lista de clientes conectados. este array incluye todos los objetos clientes que tienen info
		//std::vector<Channel> _channels;  //🚨 quitar comment out
		std::map<std::string, CommandHandler> _registrationCommands; 
  		std::map<std::string, CommandHandler> _channelCommands;
	
		//DESCARTADAS
		//struct pollfd new_cli;   //lo puse directo en NewClient() y se llama 'struct pollfd newClientPollFd'
		//struct sockaddr_in add;    //lo puse directo en init() y se llama  'struct sockaddr_in addr'      //Define la dirección y puerto donde el servidor aceptará conexiones
		//struct sockaddr_in cliadd; //lo puse directo en NewClient() y se llama  'struct sockaddr_in clientAddr'     //Define la dirección y puerto donde el servidor conectara al nuevo cliente

};
