#include "../../includes/core/Server.hpp"
#include "../../includes/core/Channel.hpp"
#include "../../includes/commands/ChannelCommands.hpp"

std::vector<std::pair<std::string, std::string> > Server::SplitJOIN(std::string cmd)
{
	// Split by spaces
	std::vector<std::string> args = split_cmd(cmd); //Output: ["JOIN", "#chan1,#chan2", "key1,key2"]

	// Split channels and keys (if existing) by comas
	std::string channels_part = args[1]; // "#chan1,#chan2"
	std::istringstream channel_stream(channels_part);
	std::string single_channel;
	std::vector<std::string> channels;

	while (std::getline(channel_stream, single_channel, ','))
		channels.push_back(single_channel); // channels = ["#chan1", "#chan2"]

	std::vector<std::string> keys;
	if (args.size() > 2)
	{
		std::string keys_part = args[2]; // "key1,key2"
		std::istringstream key_stream(keys_part);
		std::string single_key;

		while (std::getline(key_stream, single_key, ','))
			keys.push_back(single_key); // keys = ["key1", "key2"]
	}

	// Pair up
	std::vector<std::pair<std::string, std::string> > result;
	for (size_t i = 0; i < channels.size(); i++)
	{
		std::string channel = channels[i];
		std::string key = ""; // Default no key

		if (i < keys.size())
			key = keys[i];
		result.push_back(std::make_pair(channel, key)); // result = [{"#chan1", "key1"}, {"#chan2", "key2"}]
	}
	return (result);
}

void	Server::Channel_Exist(Channel *channel, Client *client, int fd, std::string key, std::string name)
{
	// Check if user is already in the channel
	if (channel->get_clientByFd(fd) || channel->get_adminByFd(fd))
		return ;

	// Check user channel limit
	int count = 0;
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (this->_channels[i].get_clientByname(client->get_nickname()))
			count++;
	}
	if (count > 10)
	{
		_sendResponse(ERROR_IN_TOO_MANY_CHANNELS(client->get_nickname()), fd);
		return ;
	}

	// Check channel modes
	if (!channel->get_password().empty() && channel->get_password() != key)
	{
		_sendResponse(ERROR_WRONG_KEY(client->get_nickname(), name), fd);
		return ;
	}
	// 1. Channel is invite-only
	if (channel->get_ModeAtIndex(0)) // Check 'i' mode at index 0
	{
		if (!client->get_channelInvitation(name))
		{
			_sendResponse(ERROR_INVITE_ONLY(client->get_nickname(), name), fd);
			return ;
		}
		client->removeChannelInvitation(name);
	}
	// 2. Channel has user limit
	if (channel->get_ModeAtIndex(4)) // Check 'l' mode at index 4
	{
		if (channel->get_totalUsers() >= channel->get_userLimit())
		{
			_sendResponse(ERROR_CHANNEL_FULL(client->get_nickname(), name), fd);
				return;
		}
	}

	// Add user to channel
	channel->add_client(*client);

	// 1. JOIN message to ALL (including joiner)
	channel->broadcast_message(MSG_USER_JOIN(client->get_hostname(), client->get_IPaddress(), name));

	// 2. Names list to joiner only
	_sendResponse(MSG_NAMES_LIST(client->get_nickname(), name, channel->get_memberList()), fd);
	_sendResponse(MSG_NAMES_END(client->get_nickname(), name), fd);

	// 3. Topic to joiner only (if exists)
	if (!channel->get_topicName().empty())
		_sendResponse(MSG_CHANNEL_TOPIC(client->get_nickname(), name, channel->get_topicName()), fd);
}

void	Server::Channel_Not_Exist(std::string channel_name, Client *client, int fd)
{
	Channel new_channel;
	new_channel.set_server(this);
	new_channel.set_name(channel_name);
	new_channel.set_channelCreationTime();

	new_channel.add_admin(*client);
	addChannel(new_channel);

	Channel *channel = get_channelByName(channel_name);

	// 1. JOIN message to ALL (including joiner)
	channel->broadcast_message(MSG_USER_JOIN(client->get_hostname(), client->get_IPaddress(), channel_name));

	// 2. Names list to joiner only
	_sendResponse(MSG_NAMES_LIST(client->get_nickname(), channel_name, channel->get_memberList()), fd);
	_sendResponse(MSG_NAMES_END(client->get_nickname(), channel_name), fd);

	// 3. Topic to joiner only (if exists)
	if (!channel->get_topicName().empty())
		_sendResponse(MSG_CHANNEL_TOPIC(client->get_nickname(), channel_name, channel->get_topicName()), fd);
}

void	Server::JOIN(std::string cmd, int fd)
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
	std::vector<std::pair<std::string, std::string> > token = SplitJOIN(cmd);
	if (token.size() == 0)
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(client->get_nickname()), fd);
		return ;
	}
	else if (token.size() > 10)
	{
		_sendResponse(ERROR_TOO_MANY_TARGETS(client->get_nickname()), fd);
		return ;
	}
	else
	{
		for (size_t i = 0; i < token.size(); i++)
		{
			if (token[i].first.empty() || token[i].first[0] != '#')
			{
				_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client->get_nickname(), token[i].first), fd);
				return ;
			}
		}
	}

	//4. Chanel Existence Logic
	for (size_t i = 0; i < token.size(); i++)
	{
		std::string channel_name = token[i].first; // "#general"
		std::string channel_key = token[i].second; // "password" or ""

		Channel *channel = get_channelByName(channel_name);
		if (channel)
			Channel_Exist(channel, client, fd, channel_key, channel_name);
		else
			Channel_Not_Exist(channel_name, client, fd);
	}
}
