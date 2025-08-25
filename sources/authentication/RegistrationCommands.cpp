#include "../../includes/core/Server.hpp"

bool isValidNick(const std::string &nick) //tengo que gregarlo al header p;ara que funcione?
{
    if (nick.empty())
        return false;

    // El primer caracter debe ser letra
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

void Server::NICK(std::string nickname, int fd)
{
    // 1️⃣ Obtener puntero al cliente
    Client* cli = get_client(fd); //puntero a client fd  //comol NICK es parte de server, las funciones que llame van a ser de esa clase
    if(!cli)
        return;

    // 2️⃣ Validar parámetro
    if(nickname.empty())
    {
        _sendResponse(ERROR_NO_NICKNAME_PROVIDED(nickname), fd);
        return ;
    }

     // 3️⃣ Validar formato del nickname
    if(!isValidNick(nickname))
    {
        _sendResponse(ERROR_INVALID_NICKNAME(nickname), fd);
        return ;
    }

    // 4️⃣ Verificar si el nickname ya está en uso
    std::vector<Client>::iterator it = _clients.begin();
    for(it; it != _clients.end(); it++)
    {
        if(it->get_nickname() == nickname && &(*it) != cli) //&(*it) != cli evita que el cliente se compare consigo mismo.
        {
            _sendResponse(ERROR_NICKNAME_IN_USE(nickname), fd);
            return;
        }
    }

    // 5️⃣ Guardar nickname antiguo
    std::string oldNickname = cli->get_nickname();

    if (oldNickname == nickname) // 🔹 Si es exactamente el mismo nick, no hacemos nada
        return;

    // 6️⃣ Propagar cambio en todos los canales del cliente
    std::vector<Channel>::iterator It = _channels.begin(); //It ya es el puntero a cada channels del server
    for (It; It != _channels.end(); It++)
    {
        if (It->isClientInChannel(oldNickname) == cli) //It ya es el puntero a cada channel //VEEEER SI CAMBIA EL NOMBRE
            It->broadcast_messageExcept(MSG_NICK_UPDATE(oldNickname, nickname), fd);  // notificar a todos los miembros del cana
    }

    // 7️⃣ Actualizar el nickname del cliente
    cli->set_nickname(nickname);

    // 8️⃣ Enviar respuesta al cliente si es un cambio
    if (!oldNickname.empty() && oldNickname != nickname)
        _sendResponse(MSG_NICK_CHANGE(oldNickname, nickname), fd);
}

void Server::USER(std::string nickname, int fd)
{


}

void Server::PASS(std::string nickname, int fd)
{


}
