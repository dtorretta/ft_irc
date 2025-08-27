#include "../../includes/core/Server.hpp"

bool isValidNick(const std::string &nick)
{
    if (nick.empty())
        return false;

    //The first character must be a letter
    if (!isalpha(nick[0]))
        return false;

    for (size_t i = 1; i < nick.size(); i++)
    {
        if (!(isalnum(nick[i]) ||
              nick[i] == '-' || nick[i] == '_' ||
              nick[i] == '[' || nick[i] == ']' ||
              nick[i] == '\\' || nick[i] == '^' ||
              nick[i] == '{' || nick[i] == '}'))
            return false;
    }
    return true;
}

void Server::NICK(std::string cmd, int fd)
{
    const std::string nickname = normalize_param(cmd.substr(4), true);
    
    //1. Get pointer to the client
    Client* cli = get_client(fd);
    if(!cli)
        return;

    //2. Validate parameter
    if(nickname.empty())
    {
        _sendResponse(ERROR_NO_NICKNAME_PROVIDED(nickname), fd);
        return ;
    }

    //3. Validate nickname format
    if(!isValidNick(nickname))
    {
        _sendResponse(ERROR_INVALID_NICKNAME(nickname), fd);
        return ;
    }

    //4. Check if the nickname is already in use
    std::vector<Client>::iterator it;
    for(it = _clients.begin(); it != _clients.end(); it++)
    {
        if(it->get_nickname() == nickname && &(*it) != cli) //&(*it) != cli --> avoids comparing the client with itself.
        {
            _sendResponse(ERROR_NICKNAME_IN_USE(nickname), fd);
            return;
        }
    }

    if(cli && cli->get_passRegistered())
    {
        //5. Save old nickname
        std::string oldNickname = cli->get_nickname();

        if (oldNickname == nickname) //If it is exactly the same nick, do nothinga
            return;

        //6. Propagate change to all the client's channels
        std::vector<Channel>::iterator It;
        for (It = _channels.begin(); It != _channels.end(); It++)
        {
            if (It->get_clientByname(oldNickname) == cli)
                It->broadcast_messageExcept(MSG_NICK_UPDATE(oldNickname, nickname), fd); //notify all channel members
        }

        //7. Update the client's nickname
        cli->set_nickname(nickname);

        //8. Send response to the client if it is a change
        if (!oldNickname.empty() && oldNickname != nickname)
            _sendResponse(MSG_NICK_UPDATE(oldNickname, nickname), fd);
    }
    else
    {
        _sendResponse(ERROR_NOT_REGISTERED_YET(nickname), fd);
        return;
    }
    if(this->isregistered(fd))
    {
        cli->set_logedIn(true);
        _sendResponse(MSG_WELCOME(nickname), fd);  
    }
}
