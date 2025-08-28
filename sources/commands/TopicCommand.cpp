#include "../../includes/core/Server.hpp"
#include "../../includes/commands/channelCommands.hpp"


std::string	getCurrentTime()
{
	std::time_t now = std::time(0);
	std::tm *timeinfo = std::localtime(&now);
	std::stringstream ss;
	ss << timeinfo->tm_hour << ":"
		<< timeinfo->tm_min << ":"
		<< timeinfo->tm_sec;
	return (ss.str());
}

std::vector<std::string>	Server::SplitTopic(std::string cmd)
{
	std::vector<std::string> result;

	// Split by spaces
	std::vector<std::string> args = split_cmd(cmd); //Output: ["TOPIC", "#chan1, ":message"]
	std::string channel = args[1]; // "#chan1"

	// Parse message (everything after ':')
	if (cmd.find(':') != std::string::npos)
	{
		size_t colon_pos = cmd.find(':');
		result.push_back(channel); // [#general]

		// Add message at the end of the array
		std::string message = cmd.substr(colon_pos + 1);
		result.push_back(message); // [#general, :message]
		return (result);
	}
	result.push_back(channel);
	return (result);
}

void  Server::TOPIC(std::string cmd, int fd)
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

	// 2. Parse and validate parameters
	std::vector<std::string> token = SplitTopic(cmd);
	if (token.size() == 0)
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(client_nick), fd);
		return ;
	}
	std::string target = token[0];
	if (target.empty() || target[0] != '#')
	{
		_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client_nick, token[0]), fd);
			return ;
	}
	std::string topic = "";
	if (token.size() == 2)
		topic = token[1];

	// 3. Check membership and existence
	Channel *channel = get_channelByName(target);
	if (!channel)
	{
		_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client_nick, target), fd);
		return ;
	}
	else if (!channel->get_clientByFd(fd) && !channel->get_adminByFd(fd))
	{
		_sendResponse(ERROR_NOT_IN_CHANNEL(client_nick, channel->get_name()), fd);
		return ;
	}

	// Topic SET mode
	if (token.size() == 2)
	{
		if (channel->get_topicRestriction() && !channel->get_adminByFd(fd))
		{
			_sendResponse(ERROR_NOT_CHANNEL_OP(channel->get_name()), fd);
			return ;
		}
		channel->set_topicName(topic);
		channel->set_topicModificationTime(getCurrentTime());
		channel->set_topicCreator(client_nick);
		channel->broadcast_messageExcept(MSG_CHANNEL_TOPIC(client_nick, channel->get_name(), topic), fd);
		channel->broadcast_messageExcept(MSG_TOPIC_WHO_TIME(client_nick, channel->get_name(), channel->get_topicModificationTime()), fd);
	}

	// Topic VIEW mode
	if (token.size() == 1)
	{
		if (channel->get_topicName() == "")
			_sendResponse(MSG_NO_SET_TOPIC(client_nick, channel->get_name()), fd);
		else
		{
			_sendResponse(MSG_CHANNEL_TOPIC(client_nick, channel->get_name(), channel->get_topicName()), fd);
			_sendResponse(MSG_TOPIC_WHO_TIME(channel->get_topicCreator(), channel->get_name(), channel->get_topicModificationTime()), fd);
		}
	}
}
