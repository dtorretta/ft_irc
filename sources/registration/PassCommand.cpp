#include "../../includes/core/Server.hpp"

void Server::PASS(std::string cmd, int fd)
{
    Client* cli = get_client(fd);
    if(!cli)
        return;
        
    std::string pass = normalize_param(cmd.substr(4), 1);
    if(pass.empty())
    {
        _sendResponse(ERROR_INSUFFICIENT_PARAMS(std::string("*")), fd);
        return;
	}
    if (isregistered(fd) || cli->get_passRegistered())
	{
        _sendResponse(ERROR_ALREADY_REGISTERED(std::string("*")), fd);
        return;
	}
    if (pass != _pass) //ver
	{
        _sendResponse(ERROR_WRONG_PASSWORD(std::string("*")), fd);
        return;
	}
    cli->set_passRegistered(true);
}