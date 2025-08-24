#include "../../includes/core/Server.hpp"
#include "../../includes/commands/channelCommands.hpp"


void	Server::INVITE(std::string cmd, int fd)
{
	//1. Check if user is registered
	if (!isregistered(fd))
	{
		_sendResponse(ERROR_NOT_REGISTERED_YET(std::string("*")), fd);
		return ;
	}

	//2. Get client object
	Client *client = GetClient(fd);
	if (!client)
		return ;
	std::string client_nick = client->GetNickName();

	//3. Parsing
	std::vector<std::string> args = split_cmd(cmd);
	if (args.size() != 3)
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(GetClient(fd)->GetNickName()), fd);
		return ;
	}
	std::string guest_nick = args[1];
	Client *guest = GetClientNick(guest_nick);
	std::string channel_name = args[2];
	Channel *channel = GetChannel(channel_name);

	//4. Validation checks
	if (!channel || channel_name.empty() || channel_name[0] != '#')
	{
		_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client_nick, channel_name), fd);
		return ;
	}
	if (!channel->get_client(fd) && !channel->get_admin(fd))
	{
		_sendResponse(ERROR_NOT_IN_CHANNEL(client_nick, channel_name), fd);
		return ;
	}
	if (!GetClientNick(guest_nick))
	{
		_sendResponse(ERROR_NICK_NOT_FOUND(guest_nick, client_nick), fd);
		return ;
	}
	if (GetChannel(channel_name)->GetClientInChannel(guest_nick))
	{
		_sendResponse(ERROR_ALREADY_IN_CHANNEL(client_nick, channel_name), fd);
		return ;
	}
	if (channel->GetInvitOnly() && !channel->get_admin(fd))
	{
		_sendResponse(ERROR_NOT_CHANNEL_OP(channel_name), fd);
		return ;
	}
	if ((GetChannel(channel_name)->GetLimit()
		&& GetChannel(channel_name)->GetClientsNumber() >= GetChannel(channel_name)->GetLimit()))
	{
		_sendResponse(ERROR_CHANNEL_FULL(client_nick, channel_name), fd);
		return ;
	}

	//5. Store invite in vector and send it to guest
	guest->AddChannelInvite(channel_name);
	_sendResponse(MSG_TO_INVITER(client_nick, guest_nick, channel_name), fd);
	_sendResponse(MSG_TO_INVITEE(guest->getHostname(), guest_nick, channel_name), guest->GetFd());
}
