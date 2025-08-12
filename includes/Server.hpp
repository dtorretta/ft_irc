#pragma once

#include <iostream>
#include <vector>

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
        int _port; //old name: port  //pasado incialmente como parametro, es el puerto que quieres que tu socket de escucha use (osea, el puerto por el cual el servidor va a aceptar conexiones)
        std::string _pass; //old name: password
        int _listeningSocket; //old name: server_fdsocket   //fd del socket que escucha conexiones y que luego sera enlazado con 'bind' a _port
        std::vector<struct pollfd> _fds; //old name: fds        //este array incluye todos los Fd de clientes conectados y del _listeningSocket
        std::vector<Client> _clients; //old name: clients   // Manejar la lista de clientes conectados. este array incluye todos los objetos clientes que tienen info
        std::vector<Channel> _channels;  //old name: channels 
        bool _signalRecieved; //old name: Signal     //signal to finish the execute loop. //como no hay mas de un objeto server a la vez, le quite el static

        //DESCARTADAS
        //struct pollfd new_cli;   //lo puse directo en NewClient() y se llama 'struct pollfd newClientPollFd'
        //struct sockaddr_in add;    //lo puse directo en init() y se llama  'struct sockaddr_in addr'      //Define la dirección y puerto donde el servidor aceptará conexiones 
        //struct sockaddr_in cliadd; //lo puse directo en NewClient() y se llama  'struct sockaddr_in clientAddr'     //Define la dirección y puerto donde el servidor conectara al nuevo cliente

        
        
        
    public:
        Server(); //⚠️ TO DO!!
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
        
        //Getters
        Client* get_Client(int fd);

        // NO LAS DESCARTE, SOLO NO LLEGUE A IMPLEMENTARLAS
    //     void acceptConnection().
    //     void readFromClient(Client&).
    //     void writeToClient(Client&).
    //     void disconnectClient(Client&).
};
