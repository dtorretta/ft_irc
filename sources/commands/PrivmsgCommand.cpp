#include "../../includes/core/Server.hpp"

/**
 * @brief Parses PRIVMSG command parameters to extract targets and message.
 * @param cmd The complete PRIVMSG command string received from the client
 * @return std::vector<std::string> Vector containing targets and message (message is last element)
 *
 * @details Parses the PRIVMSG command syntax which supports multiple targets:
 * - Locates the message content after the ':' delimiter
 * - Extracts target list before the ':' character
 * - Handles comma-separated target lists (e.g., "alice,bob,#general")
 * - Splits individual targets and adds them to the result vector
 * - Appends the message content as the last element of the vector
 * - Example: "PRIVMSG alice,#general :Hello everyone" returns ["alice", "#general", "Hello everyone"]
 * - Returns empty vector if invalid format or no message found
 *
 * @note The message content is always the last element in the returned vector
 * @note Empty targets are automatically filtered out during parsing
 * @see RFC 2812 Section 3.3.1 for PRIVMSG command syntax specifications
 */
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

/**
 * @brief Handles the IRC PRIVMSG command for sending messages to users or channels.
 * @param cmd The complete PRIVMSG command string received from the client
 * @param fd File descriptor of the client who sent the command
 * @return void
 *
 * @details Processes the IRC PRIVMSG command which allows clients to send messages:
 * - Verifies the client is registered and authenticated on the server
 * - Retrieves the client object associated with the file descriptor
 * - Parses command parameters using SplitPM() to extract targets and message
 * - Validates command format (minimum 1 target + 1 message required)
 * - Extracts message content and target list from parsed parameters
 * - Performs comprehensive validation checks:
 *   - Ensures at least one target is specified
 *   - Verifies message content is not empty
 *   - Limits maximum targets to 10 per command
 * - For each target, determines if it's a channel (starts with '#') or user:
 *   - **Channel messages**: Validates channel existence and sender membership,
 *     then broadcasts to all channel members except the sender
 *   - **User messages**: Validates target user existence, then sends direct message
 * - Continues processing remaining targets even if some fail validation
 * - Sends appropriate error responses for invalid targets or conditions
 *
 * @note Supports sending to multiple targets in a single command with comma separation
 * @note Channel names must start with '#' character to be recognized as channels
 * @note Messages are broadcast to all channel members except the sender
 * @see RFC 2812 Section 3.3.1 for complete PRIVMSG command specifications
 */
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
			Channel *channel = get_channelByName(target);
			// Validate parameters (empty target, if user in channel)
			if (!channel)
			{
				_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client_nick, target), fd);
				continue ; // Continue to next target
			}
			else if (!channel->get_clientByFd(fd) && !channel->get_adminByFd(fd))
			{
				_sendResponse(ERROR_NOT_IN_CHANNEL(client_nick, channel->get_name()), fd);
				continue ; // Continue to next target
			}
			// Send to channel
			channel->broadcast_messageExcept(MSG_PRIVMSG_CHANNEL(client_nick, client->get_username(), target, message), fd);
		}
		else // User
		{
			if (!get_clientNick(target))
			{
				_sendResponse(ERROR_NICK_NOT_FOUND(target, client_nick), fd);
				continue ; // Continue to next target
			}
			// Send to user
			_sendResponse(MSG_PRIVMSG_USER(client->get_nickname(), client->get_username(), target, message), get_clientNick(target)->get_fd());
		}
	}
}
