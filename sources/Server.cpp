#include "../includes/Server.hpp"

void Server::init(int port, std::string pass)
{
    this->_pass = pass;
    this->_port = port;
    this->_status = false;
    //etc?
    //solo init, despues todo lo que sea del eterno while loop de la ejecucion va en el main en execute
}

bool Server::getStatus()
{
    return this->_status;
}

