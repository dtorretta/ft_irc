#pragma once

#include <iostream>
#include <vector>
#include <sstream>

#include <poll.h>
#include <netinet/in.h>  // para sockaddr_in en Unix/Linux/macOS
#include <sys/socket.h>  // para funciones socket(), bind(), listen(), etc.
#include <arpa/inet.h>   // para funciones como htons()

#include "Client.hpp"
#include "Channel.hpp"

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
	private:
		int _port;  //pasado incialmente como parametro, es el puerto que quieres que tu socket de escucha use (osea, el puerto por el cual el servidor va a aceptar conexiones)
		std::string _pass;
		int _listeningSocket; //fd del socket que escucha conexiones y que luego sera enlazado con 'bind' a _port
		std::vector<struct pollfd> _fds; //este array incluye todos los Fd de clientes conectados y del _listeningSocket
		std::vector<Client> _clients; // Manejar la lista de clientes conectados. este array incluye todos los objetos clientes que tienen info
		std::vector<Channel> _channels;


		//struct sockaddr_in addr;    //VER SI LO PUEDO SACAR          //Define la dirección y puerto donde el servidor aceptará conexiones
		//struct sockaddr_in _clientAddr; //VER SI LO PUEDO SACAR     //Define la dirección y puerto donde el servidor conectara al nuevo cliente

		bool _signalRecieved; //signal to finish the execute loop. //como no hay mas de un objeto server a la vez, le quite el static


	public:
		Server();
		~Server();

		//Methods
		void init(int port, std::string pass);
		void execute(); //(loop principal de poll).
		void NewClient();
		void NewData(int clientFd);

		//Close
		void ft_close(int Fd);
		void RemoveFd(int Fd); //si no los uso por fuera de ft_close, eliminarlos del header y agregarlos a utils
		void RemoveClient(int clientFd); //si no los uso por fuera de ft_close, eliminarlos del header y agregarlos a utils

		/******************/
		/*    Commands    */
		/******************/
		//JOIN
		void	JOIN(std::string cmd, int fd);
		std::vector<std::pair<std::string, std::string> > SplitJOIN(std::string cmd); //NEW!!!
		void	Channel_Exist(Channel *channel, Client *client, int fd, std::string key, std::string name); //NEW!!!
		void	Channel_Not_Exist(std::string channel_name, Client *client, int fd); //NEW!!!

	//     void acceptConnection().
	//     void readFromClient(Client&).
	//     void writeToClient(Client&).
	//     void disconnectClient(Client&).



	// // Otros métodos importantes:
	// void handleCommand(int clientFd, const std::string& command);  // interpretar comandos IRC
	// void broadcastToChannel(const std::string& channel, const std::string& message, int exceptFd = -1); // enviar mensajes a todos de un canal

	// // Manejo señales para cerrar el server
	// static void SignalHandler(int signum);
};


/*
Clase Server
Responsabilidades:
	Mantener el socket de escucha.
	Manejar la lista de clientes conectados.
	Agregar/quitar clientes.
	Gestionar el estado global (canales, usuarios).
	Procesar eventos recibidos por poll() (aceptar conexiones, leer datos, enviar respuestas).
*/
