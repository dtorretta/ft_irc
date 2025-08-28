#include "../../includes/core/Server.hpp"
#include "../../includes/commands/ChannelCommands.hpp"


std::vector<std::string> Server::SplitPART(std::string command)
{
	std::vector<std::string> args = split_cmd(command);  //Output: ["PART", "#chan1,#chan2"]

	std::string channels_part = args[1]; // "#chan1,#chan2"
	std::istringstream channel_stream(channels_part);
	std::string single_channel;
	std::vector<std::string> channels;

	while (std::getline(channel_stream, single_channel, ','))
		channels.push_back(single_channel); // channels = ["#chan1", "#chan2"]

	return (channels);
}

void	Server::PART(std::string cmd, int fd)
{
	//1. Check if user is registered
	if (!isregistered(fd))
	{
		_sendResponse(ERROR_NOT_REGISTERED_YET(std::string("*")), fd);
		return ;
	}

	//2. Get client object
	Client *client = get_client(fd);
	if (!client)
		return ;

	//3. Parse and validate parameters
	std::vector<std::string> token = SplitPART(cmd);
	if (token.size() == 0)
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(client->get_nickname()), fd);
		return ;
	}

	// Parse reason (everything after ':')
	std::string reason = "Leaving";
	size_t reason_pos = cmd.find(':');
	if (reason_pos != std::string::npos)
		reason = cmd.substr(reason_pos + 1);

	//4. Chanel Existence Logic
	for (size_t i = 0; i < token.size(); i++)
	{
		std::string channel_name = token[i];
		Channel *channel = get_channelByName(channel_name);
		if (channel)
		{
			if (!channel->get_clientByFd(fd) && !channel->get_adminByFd(fd))
			{
				_sendResponse(ERROR_NOT_IN_CHANNEL(client->get_nickname(), channel_name), fd);
				continue ;
			}

			// Send PART message to ALL channel members
			channel->broadcast_message(MSG_USER_PART(client->get_nickname(), client->get_username(), client->get_IPaddress(), channel_name, reason));

			// Remove client from channel
			if (channel->get_clientByFd(fd))
				channel->remove_client(fd);
			else if (channel->get_adminByFd(fd))
				channel->remove_admin(fd);
		}
		else
		{
			_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client->get_nickname(), channel_name), fd);
			return ;
		}
	}
}
