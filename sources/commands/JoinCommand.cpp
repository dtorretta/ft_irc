#include "../../includes/core/Server.hpp"

/**
 * @brief Parses JOIN command parameters into channel-key pairs.
 * @param cmd The complete JOIN command string received from the client
 * @return std::vector<std::pair<std::string, std::string>> Vector of channel-key pairs
 *
 * @details Parses the JOIN command syntax which supports multiple channels and keys:
 * - Splits the command by spaces to extract channels and keys
 * - Handles comma-separated channel lists (e.g., "#chan1,#chan2")
 * - Handles comma-separated key lists (e.g., "key1,key2")
 * - Pairs each channel with its corresponding key (empty string if no key)
 * - Example: "JOIN #chan1,#chan2 key1,key2" returns [{"#chan1","key1"}, {"#chan2","key2"}]
 *
 * @note Keys are optional and will be paired with channels by index order
 * @see RFC 2812 Section 3.2.1 for JOIN command syntax specifications
 */
std::vector<std::pair<std::string, std::string> > Server::SplitJOIN(std::string cmd)
{
	// Split by spaces
	std::vector<std::string> args = split_cmd(cmd); //Output: ["JOIN", "#chan1,#chan2", "key1,key2"]

	if (args.size() < 2)
		return (std::vector<std::pair<std::string, std::string> >());

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

/**
 * @brief Handles joining an existing channel with validation checks.
 * @param channel Pointer to the existing channel object
 * @param client Pointer to the client attempting to join
 * @param fd File descriptor of the client
 * @param key Channel password/key provided by the client
 * @param name Name of the channel being joined
 * @return void
 *
 * @details Processes a client's request to join an existing channel with comprehensive validation:
 * - Checks if the user is already in the channel (prevents duplicate joins)
 * - Validates user channel limit (maximum 10 channels per user)
 * - Verifies channel password if the channel is password-protected
 * - Handles invite-only channels by checking invitation status
 * - Enforces user limit restrictions for channels with limits enabled
 * - Adds the client to the channel upon successful validation
 * - Broadcasts JOIN message to all channel members
 * - Sends names list and topic information to the joining client
 *
 * @note Automatically removes invitation after successful join to invite-only channel
 * @see Channel modes: 'i' (invite-only) at index 0, 'l' (user limit) at index 4
 */
void	Server::Channel_Exist(Channel *channel, Client *client, int fd, std::string key, std::string name)
{
	// Check if user is already in the channel
	if (channel->get_clientByFd(fd) || channel->get_adminByFd(fd))
	{
		_sendResponse(ERROR_ALREADY_IN_CHANNEL(client->get_nickname(), channel->get_name()), fd);
		return ;
	}

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

/**
 * @brief Creates and joins a new channel when it doesn't exist.
 * @param channel_name Name of the new channel to be created
 * @param client Pointer to the client who will become the channel operator
 * @param fd File descriptor of the client
 * @return void
 *
 * @details Handles the creation of a new channel when a client attempts to join
 * a channel that doesn't exist on the server:
 * - Creates a new Channel object with the specified name
 * - Sets the server reference for the new channel
 * - Records the channel creation timestamp
 * - Automatically makes the creating client a channel administrator/operator
 * - Adds the new channel to the server's channel list
 * - Broadcasts JOIN message to notify the client
 * - Sends names list showing the client as the only member
 * - Sends topic information if a topic exists (typically empty for new channels)
 *
 * @note The first user to join a non-existent channel automatically becomes its operator
 * @see RFC 2812 Section 1.3 for channel creation behavior
 */
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

/**
 * @brief Handles the IRC JOIN command for joining one or more channels.
 * @param cmd The complete JOIN command string received from the client
 * @param fd File descriptor of the client who sent the command
 * @return void
 *
 * @details Processes the IRC JOIN command which allows clients to join channels:
 * - Verifies the client is registered and authenticated on the server
 * - Retrieves the client object associated with the file descriptor
 * - Parses command parameters to extract channel names and optional keys
 * - Validates parameter count (minimum 1, maximum 10 channels per command)
 * - Ensures all channel names start with '#' character (valid channel format)
 * - For each requested channel, determines if it exists or needs to be created
 * - Calls Channel_Exist() for existing channels with validation checks
 * - Calls Channel_Not_Exist() for new channels that need to be created
 * - Sends appropriate error responses for invalid requests
 *
 * @note Supports joining multiple channels in a single command with comma separation
 * @see RFC 2812 Section 3.2.1 for complete JOIN command specifications
 */
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
