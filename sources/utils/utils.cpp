#include "../../includes/core/Server.hpp"

bool Server::isregistered(int fd)
{
    if (!get_client(fd) ||
        get_client(fd)->get_nickname().empty() ||
        get_client(fd)->get_username().empty() ||
        get_client(fd)->get_nickname() == "*" ||
        !get_client(fd)->get_logedIn()) //⚠️ TO DO!!
        return false;
	return true;
}

void Server::_sendResponse(std::string response, int fd)
{
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}


//quitar espacios al principio y al final
std::string trim(const std::string &s) //tengo que incluirla en el header?? sino mandar a utils
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

//splits the string with the whole command (ex: "PRIVMSG #general :Hello, world!") in different strings
std::vector<std::string> Server::split_cmd(std::string &cmd)
{
    std::vector<std::string> commands;
    std::istringstream iss(cmd); //transforma el string en un stream de entrada para que pueda funcionar con >>
    std::string token; //el primer “token” de cada línea siempre se interpreta como el comando
    while(iss >> token) //El operador >> lee hasta el primer espacio en blanco y lo guarda en la variable command.
    {
        commands.push_back(token);
        token.clear();
    }
    return commands;
}







/*
 *  Cuando queremos eliminar y cerrar todo, el Fd es -42 y la secuencia correcta es:
 *      - Cerrar cada file descriptor individualmente (con close(fd)) para liberar el recurso del sistema operativo.
 *      - Luego, hacer clear() en el vector que almacena esos fds, para eliminar todos los elementos del vector y dejarlo vacío.
 *  cuando se quiera eliminar un Fd en particular, el parametro sera algun valor > 0
*/

void Server::ft_close(int Fd)
{
    RemoveClient(Fd); //si nunca los voy a llamar por fuera de esta funcion, puedo quitarlols del header y agregarlos a utils
    RemoveFd(Fd);  //si nunca los voy a llamar por fuera de esta funcion, puedo quitarlols del header y agregarlos a utils
    RemoveChannel(Fd);
    close(Fd);
}

// void Server::RemoveClient(int clientFd)
// {
//     for (size_t i = 0; i < _clients.size(); i++)
//     {
//         if (_clients[i].get_fd() == clientFd)
//         {
//             _clients.erase(_clients.begin() + i);
//             break;
//         }
//     }
// }

void Server::RemoveClient(int clientFd) //c++ style
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (it->get_fd() == clientFd) //accede al método del objeto al que apunta.
        {
            _clients.erase(it);
            break;
        }
    }
}


// void Server::RemoveFd(int Fd)
// {
//     for (size_t i = 0; i < _fds.size(); i++)
//     {
//         if (_fds[i].fd == Fd)
//         {
//             _fds.erase(_fds.begin() + i);
//             break;
//         }
//     }
//     this->_listeningSocket = -1;
// }

void Server::RemoveFd(int Fd) //c++ style
{
    for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); it++)
    {
        if (it->fd == Fd)
        {
            _fds.erase(it);
            break;
        }
    }
    this->_listeningSocket = -1;
}

void Server::RemoveChannel(std::string &name)
{
    for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (it->GetName() == name)
        {
            _channels.erase(it);
            return;
        }
    }
}
