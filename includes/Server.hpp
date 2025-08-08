#pragma once

#include <iostream>
#include <vector>

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
        int _port;  //Mantener el socket de escucha.
        std::string _pass;
        std::vector<Client> _clients; // Manejar la lista de clientes conectados.
        std::vector<Channel> _channels; // Manejar la lista de clientes conectados.
        bool _status; //signal to finish the execute loop. //como no hay mas de un objeto server a la vez, le quite el static
        
    public:
        Server();
        ~Server();
        
        //methods
        void init(int port, std::string pass);
        void closeFds();
        void execute(); //(loop principal de poll).
        
    //     void acceptConnection().
    //     void readFromClient(Client&).
    //     void writeToClient(Client&).
    //     void disconnectClient(Client&).
    
        //getter
        bool getStatus();


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