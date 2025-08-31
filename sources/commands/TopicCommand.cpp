#include "../../includes/core/Server.hpp"

/**
 * @brief Gets the current system time formatted as HH:MM:SS.
 * @return std::string Current time in "hour:minute:second" format
 *
 * @details Utility function that retrieves the current system time and formats it:
 * - Uses std::time(0) to get current timestamp
 * - Converts to local time using std::localtime()
 * - Formats as "HH:MM:SS" string using stringstream
 * - Used primarily for timestamping topic modifications
 *
 * @note Returns time in 24-hour format without leading zeros
 * @see Used by TOPIC command to timestamp topic changes
 */
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

/**
 * @brief Parses TOPIC command parameters to extract channel and optional topic message.
 * @param cmd The complete TOPIC command string received from the client
 * @return std::vector<std::string> Vector containing [channel] or [channel, topic_message]
 *
 * @details Parses the TOPIC command syntax which supports both viewing and setting topics:
 * - Splits the command by spaces to extract basic components
 * - Identifies the target channel from the second argument
 * - Detects presence of ':' delimiter indicating topic message
 * - If ':' found, extracts everything after it as the new topic message
 * - Returns structured vector with 1 or 2 elements:
 *   - **View mode**: [channel] - for viewing current topic
 *   - **Set mode**: [channel, topic_message] - for setting new topic
 * - Examples:
 *   - "TOPIC #general" returns ["#general"] (view mode)
 *   - "TOPIC #general :Welcome to our channel" returns ["#general", "Welcome to our channel"] (set mode)
 *
 * @note Topic message can be empty string if ':' is present but no text follows
 * @see RFC 2812 Section 3.2.4 for TOPIC command syntax specifications
 */
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

/**
 * @brief Handles the IRC TOPIC command for viewing or setting channel topics.
 * @param cmd The complete TOPIC command string received from the client
 * @param fd File descriptor of the client who sent the command
 * @return void
 *
 * @details Processes the IRC TOPIC command which supports both topic viewing and modification:
 * - Verifies the client is registered and authenticated on the server
 * - Retrieves the client object associated with the file descriptor
 * - Parses command parameters using SplitTopic() to extract channel and optional topic
 * - Validates command format and channel name (must start with '#')
 * - Performs comprehensive validation:
 *   - Ensures the target channel exists on the server
 *   - Confirms the client is a member of the channel
 * - Operates in two distinct modes based on parameter count:
 *
 * **Topic SET mode** (2 parameters: channel + topic):
 *   - Checks topic restriction mode and operator privileges if required
 *   - Updates channel topic with new message
 *   - Records modification timestamp using getCurrentTime()
 *   - Sets topic creator to the requesting client
 *   - Broadcasts topic change to all channel members except setter
 *   - Sends topic metadata (who set it and when) to channel members
 *
 * **Topic VIEW mode** (1 parameter: channel only):
 *   - Checks if channel has a topic set
 *   - Sends current topic to requesting client if one exists
 *   - Includes topic metadata (creator and modification time)
 *   - Sends "no topic set" message if channel has no topic
 *
 * @note Topic restrictions ('t' mode) require operator privileges to modify topics
 * @note All topic changes are timestamped and attributed to the setting user
 * @see RFC 2812 Section 3.2.4 for complete TOPIC command specifications
 */
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
