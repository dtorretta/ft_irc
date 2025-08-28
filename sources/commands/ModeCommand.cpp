#include "../../includes/core/Server.hpp"
#include "../../includes/core/Channel.hpp"
#include "../../includes/commands/ChannelCommands.hpp"


bool	deactivateMode(char mode, std::string parameter, Channel *channel)
{
	switch(mode)
	{
		case 'i':
			channel->set_inviteOnly(false); channel->set_modeAtIndex(0, false);
			return (true);
		case 't':
			channel->set_topicRestriction(false); channel->set_modeAtIndex(1, false);
			return (true);
		case 'k':
			if (parameter == channel->get_password())
			{
				channel->set_password("");
				channel->set_modeAtIndex(2, false);
				return (true);
			}
			return (false);
		case 'o':
			if (channel->change_adminToClient(parameter))
			{
				channel->set_modeAtIndex(3, false);
				return (true);
			}
			return (false);
		case 'l':
			channel->set_userLimit(0); channel->set_modeAtIndex(4, false);
			return (true);
		default:
			return (false); // invalid mode
	}
}

bool	activateMode(char mode, std::string parameter, Channel *channel)
{
	switch(mode)
	{
		case 'i':
			channel->set_inviteOnly(true); channel->set_modeAtIndex(0, true);
			return (true);
		case 't':
			channel->set_topicRestriction(true); channel->set_modeAtIndex(1, true);
			return (true);
		case 'k':
			channel->set_password(parameter); channel->set_modeAtIndex(2, true);
			return (true);
		case 'o':
			if (channel->change_clientToAdmin(parameter))
			{
				channel->set_modeAtIndex(3, true);
				return (true);
			}
			return (false);
		case 'l':
			channel->set_userLimit(atoi(parameter.c_str())); channel->set_modeAtIndex(4, true);
			return (true);
		default:
			return (false); // invalid mode
	}
}

bool	Server::isChannelValid(Channel *channel, std::string channel_string, std::string client_nick, int fd)
{
	if (!channel)
	{
		_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client_nick, channel_string), fd);
		return (false);
	}
	else if (!channel->get_clientByFd(fd) && !channel->get_adminByFd(fd))
	{
		_sendResponse(ERROR_NOT_IN_CHANNEL(client_nick, channel->get_name()), fd);
		return (false);
	}
	else if (!channel->get_adminByFd(fd))
	{
		_sendResponse(ERROR_NOT_CHANNEL_OP(channel->get_name()), fd);
		return (false);
	}
	return (true);
}

bool	needsParameter(char mode, char operation)
{
	if (mode == 'k' || mode == 'o')
		return (true);
	if (mode == 'l' && operation == '+')
		return (true);
	else
		return (false);
}

std::vector<std::string> processModeString(const std::string &modeString,
	std::vector<std::string> &parameters)
{
	std::vector<std::string> result;
	size_t paramIndex = 0;
	char currOperation = '+'; // default in case user sends 'i'

	for (size_t i = 0; i < modeString.size(); i++)
	{
		char c = modeString[i];

		if (c == '+' || c == '-')
			currOperation = c; // Update current sign
		else
		{
			std::string mode_operation = std::string(1, currOperation) + c; // "+o" or "-k"
			if (needsParameter(c, currOperation))
			{
				if (paramIndex < parameters.size())
				{
					mode_operation += " " + parameters[paramIndex++]; // "+l 50"
					result.push_back(mode_operation);
				}
				else
					return (std::vector<std::string>());
			}
			else
				result.push_back(mode_operation);
		}
	}
	return (result);
}

std::vector<std::string>	Server::SplitMODE(std::string cmd)
{
	std::vector<std::string> args = split_cmd(cmd); //Output: ["MODE", "#chan1", "+o", "alice", "-o", "bob", "+l", "50"]
	if (args.size() < 2)
		return (std::vector<std::string>());

	std::vector<std::string> result;
	result.push_back(args[1]); // [0] = channel
	if (args.size() == 2)
	{
		result.push_back("");
		return (result);
	}

	std::string modeStrings = "";
	std::vector<std::string> params;
	for (size_t i = 2; i < args.size(); i++)
	{
		if (args[i][0] == '+' || args[i][0] == '-')
			modeStrings.append(args[i]);
		else
			params.push_back(args[i]);
	}
	result.push_back(modeStrings);
	for (size_t i = 0; i < params.size(); i++)
		result.push_back(params[i]);

	return (result); // ["#chan1", "+o-o+l", "alice", "bob", "50"]
}

void	Server::MODE(std::string cmd, int fd)
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
	std::string client_nick = client->get_nickname();

	//3. Parse and validate parameters
	std::vector<std::string> token = SplitMODE(cmd);
	if (token.size() == 0)
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(client->get_nickname()), fd);
		return ;
	}
	std::string channel_string = token[0];

	//4. Display Mode
	Channel *channel = get_channelByName(channel_string);
	if (token.size() == 1)
	{
		if (!isChannelValid(channel, channel_string, client_nick, fd))
			return ;
		_sendResponse(MSG_CHANNEL_MODES(client_nick, channel_string, channel->get_activeModes()), fd);
		_sendResponse(MSG_CREATION_TIME(client_nick, channel_string, channel->get_channelCreationTime()), fd);
	}
	//5. Handle Modes
	else
	{
		// Parse mode operations
		std::string modeString = token[1]; // "+k+o+l"
		if (!modeString.empty())
		{
			std::vector<std::string> params(token.begin() + 2, token.end()); // ["mypass", "alice", "50"]
			std::vector<std::string> operations = processModeString(modeString, params);
			if (operations.empty() && !modeString.empty())
			{
				_sendResponse(ERROR_INSUFFICIENT_PARAMS(client->get_nickname()), fd);
				return;
			}
			if (!isChannelValid(channel, channel_string, client_nick, fd))
				return ;

			// Apply Modes changes and track success for broadcasting
			std::string successfulModes = ""; // Will build "+o-t+k"
			std::string modeParams = ""; // Will build "alice password"
			for (size_t i = 0; i < operations.size(); i++)
			{
				char sign = operations[i][0]; // '+' or '-'
				char mode = operations[i][1]; // 'i', 't', 'k', 'o', 'l'
				std::string parameter = "";
				size_t SpacePos = operations[i].find(' ');
				if (SpacePos != std::string::npos)
					parameter = operations[i].substr(SpacePos + 1);

				bool success = false;
				if (operations[i][0] == '+')
					success = activateMode(mode, parameter, channel);
				else
					success = deactivateMode(mode, parameter, channel);

				if (success)
				{
					successfulModes += sign;
					successfulModes += mode;

					if (!parameter.empty() && needsParameter(mode, sign))
					{
						if (!modeParams.empty())
							modeParams += " ";
						modeParams += parameter;
					}
				}
			}
			//Broadcast to all channel members
			if (!successfulModes.empty())
				channel->broadcast_message(MSG_MODE_CHANGE(client_nick, client->get_username(),
					channel_string, successfulModes, modeParams));
		}
	}
}
