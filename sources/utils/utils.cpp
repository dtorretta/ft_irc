#include "../../includes/core/Server.hpp"

void Server::signalHandler(int sig)
{
    (void) sig;
    _signalRecieved = true;
}

bool Server::isregistered(int fd)
{
    if (!get_client(fd) ||
        !get_client(fd)->get_passRegistered() ||
        get_client(fd)->get_nickname().empty() ||
        get_client(fd)->get_username().empty() ||
        get_client(fd)->get_nickname() == "*") //cuando usamos el *???
        return false;
	
    return true;
}

void Server::_sendResponse(std::string response, int fd)
{
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() failed" << std::endl;
}


//Trim spaces at the beginning and end. Additionally, when 'flag' is true, remove the leading ':'
std::string Server::normalize_param(const std::string &s, bool flag)
{
    std::string result = s;

    //Remove spaces/tabs at the beginnin
    size_t start = result.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";

    //Remove spaces/tabs at the end
    size_t end = result.find_last_not_of(" \t\r\n");
    if (end != std::string::npos)
        result =  result.substr(start, end - start + 1);
    if(flag)
    {
        if(!result.empty() && result[0] == ':') //Remove ':'
            result.erase(result.begin());
    }
    return result;
}

//splits the string with the whole command (ex: "PRIVMSG #general :Hello, world!") in different strings
std::vector<std::string> Server::split_cmd(std::string &cmd)
{
    std::vector<std::string> commands;
    std::istringstream iss(cmd); //converts the string into an input stream so it can work with >>
    std::string token; 
    while(iss >> token) //the >> operator reads until the first whitespace and stores it in the token variable
    {
        if(token[0] == ':')
        {
            std::string rest;
            std::getline(iss, rest);
            token.erase(token.begin());
            token = token + rest;
        }
        commands.push_back(token);
        token.clear();
    }
    return commands;
}

void Server::ft_close(int Fd)
{
    RemoveClient(Fd);
    RemoveFd(Fd);
    RemoveClientFromChannel(Fd);
    close(Fd);
}

void Server::RemoveClient(int clientFd)
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (it->get_fd() == clientFd)
        {
            _clients.erase(it);
            break;
        }
    }
}

void Server::RemoveFd(int Fd)
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

void Server::RemoveClientFromChannel(int fd)
{
	for (size_t i = 0; i < this->_channels.size(); i++)
    {
		int flag = 0;
		if (_channels[i].get_clientByFd(fd))
			{_channels[i].remove_client(fd); flag = 1;}
		else if (_channels[i].get_adminByFd(fd))
			{_channels[i].remove_admin(fd); flag = 1;}
		if (_channels[i].get_totalUsers() == 0)
			{_channels.erase(_channels.begin() + i); i--; continue;}
		if (flag){
			std::string rpl = ":" + get_client(fd)->get_nickname() + "!~" + get_client(fd)->get_username() + "@localhost QUIT Quit\r\n";
			_channels[i].broadcast_message(rpl); //deberia no incluir al que lo emitio?
		}
	}
}

void Server::RemoveChannel(std::string &name)
{
    for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (it->get_name() == name)
        {
            _channels.erase(it);
            return;
        }
    }
}
