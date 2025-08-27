#include "../../includes/core/Server.hpp"

void Server::USER(std::string cmd, int fd)
{
    Client* cli = get_client(fd);
    if(!cli)
        return;
        
    std::vector<std::string> commands = split_cmd(cmd);
    
    //1. Check number of parameters
    if(commands.size() < 5)
    {
        if(!cli->get_nickname().empty())
            _sendResponse(ERROR_INSUFFICIENT_PARAMS(cli->get_nickname()), fd);
        else
            _sendResponse(ERROR_INSUFFICIENT_PARAMS(std::string("*")), fd);
        return;
    }
    
    //2. Verify password if it is required
    if(!cli->get_passRegistered())
    {
        if(!cli->get_nickname().empty())
            _sendResponse(ERROR_NOT_REGISTERED_YET(cli->get_nickname()), fd);
        else
            _sendResponse(ERROR_NOT_REGISTERED_YET(std::string("*")), fd);
        return;
    }
    
    //3. Check that the client is not already registered
    if (isregistered(fd) || !cli->get_username().empty())
	{
         if(!cli->get_nickname().empty())
            _sendResponse(ERROR_ALREADY_REGISTERED(cli->get_nickname()), fd);
        else
            _sendResponse(ERROR_ALREADY_REGISTERED(std::string("*")), fd);
        return;
	}
	
	//4. Set username
	if(cli && cli->get_passRegistered())
	    cli->set_username(commands[1]);
    
    //5. Mark as logged_in if now meeting the requirements
    if(this->isregistered(fd))
    {
        cli->set_logedIn(true);
        _sendResponse(MSG_WELCOME(cli->get_nickname()), fd);  
    }
}