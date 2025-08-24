#include "../../includes/core/Server.hpp"
#include "../../includes/commands/channelCommands.hpp"


std::vector<std::string>	Server::SplitKICK(std::string cmd)
{
	std::vector<std::string> args = split_cmd(cmd);  //Output: ["KICK", "#chan1,#chan2", "migue", ":Bad", "behavior"]
	if (args.size() < 3)
		return (std::vector<std::string>());

	std::vector<std::string> result;

	// Split channels by comas
	std::string channels = args[1]; // "#chan1,#chan2"
	std::string target_user = args[2]; // "migue"
	std::string reason = "";

	// Parse reason (everything after ':') if there's one
	if (args.size() > 3)
	{
		size_t colon_pos = cmd.find(':');
		if (colon_pos != std::string::npos) // Found ':', everything after is the reason
			reason = cmd.substr(colon_pos + 1); // "bad behavior"
	}

	// Merge channels, username, and reason into one vector
	result.push_back(channels);
	result.push_back(target_user);
	result.push_back(reason);

	return (result);
}

void Server::KICK(std::string cmd, int fd)
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

	//3. Parse parameters and checks
	std::vector<std::string> token = SplitKICK(cmd);
	if (token.size() == 0)
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(client_nick), fd);
		return ;
	}

	std::string channels_string = token[0];
	std::string target_user = token[1];
	std::string reason = token[2];

	//4. Parse channels
	std::vector<std::string> individual_channels;
	std::istringstream ss(channels_string);
	std::string single_channel;

	while (std::getline(ss, single_channel, ','))
		individual_channels.push_back(single_channel);

	// 5. Validation loop for each channel
	for (size_t i = 0; i < individual_channels.size(); i++)
	{
		std::string target = individual_channels[i];
		Channel *channel = GetChannel(target);
		// Validate parameters (empty target, if user in channel)
		if (!channel)
		{
			_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client_nick, target), fd);
			continue ; // Continue to next target
		}
		else if (!channel->get_client(fd) && !channel->get_admin(fd))
		{
			_sendResponse(ERROR_NOT_IN_CHANNEL(client_nick, channel->GetName()), fd);
			continue ; // Continue to next target
		}
		else if (!channel->get_admin(fd))
		{
			_sendResponse(ERROR_NOT_CHANNEL_OP(channel->GetName()), fd);
			continue ;
		}
		else if (!channel->clientInChannel(target_user))
		{
			_sendResponse(ERROR_USER_NOT_IN_CHANNEL(target_user, channel->GetName()), fd);
			continue ; // Continue to next target
		}
		// Kick execution
		else
		{
			if (reason.empty())
				channel->sendTo_all(MSG_KICK_USER(client_nick, GetClient(fd)->GetUserName(), channel->GetName(), target_user), fd);
			else
				channel->sendTo_all(MSG_KICK_USER_REASON(client_nick, GetClient(fd)->GetUserName(), channel->GetName(), target_user, reason), fd);

			if (channel->get_admin(channel->GetClientInChannel(target_user)->GetFd()))
				channel->remove_admin(channel->GetClientInChannel(target_user)->GetFd());
			else
				channel->remove_client(channel->GetClientInChannel(target_user)->GetFd());

			if (channel->GetClientsNumber() == 0)
				RemoveChannel(channel->GetName());
		}
	}
}
