#include "../../includes/core/Server.hpp"
#include "../../includes/commands/channelCommands.hpp"

std::vector<std::string> Server::SplitPM(std::string cmd)
{
	// Parse message (everything after ':')
	std::vector<std::string> result;
	size_t colon_pos = cmd.find(':');
	if (colon_pos == std::string::npos)
		return (result); // No message found

	// Extract target before ':' and split by spaces
	std::string before_colon = cmd.substr(0, colon_pos);
	std::vector<std::string> args = split_cmd(before_colon);
	if (args.size() < 2)
		return (result); // invalid format

	// Split targets by comma (args[1] = "alice,bob,#general")
	std::string targets_part = args[1];
	std::istringstream target_stream(targets_part);
	std::string single_target;

	while (std::getline(target_stream, single_target, ','))
	{
		if (!single_target.empty()) // Skip empty targets
			result.push_back(single_target); // [target1, target2, target3]
	}

	// Add message at the end of the array
	std::string message = cmd.substr(colon_pos + 1);
	result.push_back(message); // [target1, target2, target3, message]

	return (result);
}

void Server::PRIVMSG(std::string cmd, int fd)
{
	//1. Check if user is registered
	if (!isregistered(fd))
	{
		_sendResponse(ERROR_NOT_REGISTERED_YET(std::string("*")), fd);
		return ;
	}

	Client *client = get_client(fd);
	if (!client)
		return ;
	std::string client_nick = client->get_nickname();

	// 2. Parse parameters and checks
	std::vector<std::string> token = SplitPM(cmd);
	if (token.size() < 2)  // At least 1 target + 1 message
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(client_nick), fd);
		return ;
	}

	// Extract message (last element) and targets (all others)
	std::string message = token.back();
	token.pop_back(); // Remove message, leaving only targets
	std::vector<std::string> targets = token;

	if (targets.empty())
	{
		_sendResponse(ERROR_NO_RECIPIENT(client_nick), fd);
		return ;
	}
	if (message.empty())
	{
		_sendResponse(ERROR_NO_TEXT_TO_SEND(client_nick), fd);
		return ;
	}
	if (targets.size() > 10)
	{
		_sendResponse(ERROR_TOO_MANY_TARGETS(client_nick), fd);
		return ;
	}

	// 4. Process each target
	for (size_t i = 0; i < targets.size(); i++)
	{
		std::string target = targets[i];

		if (target[0] == '#') // Channel
		{
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
			// Send to channel
			channel->sendTo_all(MSG_PRIVMSG_CHANNEL(client_nick, client->GetUserName(), target, message), fd);
		}
		else // User
		{
			if (!GetClientNick(target))
			{
				_sendResponse(ERROR_NICK_NOT_FOUND(target, client_nick), fd);
				continue ; // Continue to next target
			}
			// Send to user
			_sendResponse(MSG_PRIVMSG_USER(client->GetNickName(), client->GetUserName(), target, message), GetClientNick(target)->get_fd());
		}
	}
}
