#include "../../includes/core/Server.hpp"

/**
 * @brief Parses KICK command parameters to extract channels, target user, and reason.
 * @param cmd The complete KICK command string received from the client
 * @return std::vector<std::string> Vector containing [channels, target_user, reason]
 *
 * @details Parses the KICK command syntax which supports kicking users from channels:
 * - Splits the command by spaces to extract basic components
 * - Validates minimum parameter count (requires at least channel and target user)
 * - Extracts channel list (comma-separated for multiple channels)
 * - Identifies the target user to be kicked
 * - Parses optional reason message (text after ':' delimiter)
 * - Returns a structured vector with exactly 3 elements:
 *   - [0]: Channel names (comma-separated if multiple)
 *   - [1]: Target username to kick
 *   - [2]: Kick reason (empty string if no reason provided)
 * - Example: "KICK #general,#random alice :Spamming" returns ["#general,#random", "alice", "Spamming"]
 *
 * @note Returns empty vector if insufficient parameters (less than 3 arguments)
 * @note Reason defaults to empty string if not provided or ':' delimiter not found
 * @see RFC 2812 Section 3.2.8 for KICK command syntax specifications
 */
std::vector<std::string> Server::SplitKICK(std::string cmd)
{
	std::vector<std::string> args = split_cmd(cmd);  //Output: ["KICK", "#chan1,#chan2", "migue", ":Bad", "behavior"]
	if (args.size() < 3)
		return (std::vector<std::string>());

	std::vector<std::string> result;

	// Split channels by comas
	std::string channels = args[1]; // "#chan1,#chan2"
	std::string target_user = args[2]; // "migue"
	std::string reason = "";

	// Parse reason
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

/**
 * @brief Handles the IRC KICK command for removing users from channels.
 * @param cmd The complete KICK command string received from the client
 * @param fd File descriptor of the client who sent the command
 * @return void
 *
 * @details Processes the IRC KICK command which allows channel operators to remove users:
 * - Verifies the client is registered and authenticated on the server
 * - Retrieves the client object associated with the file descriptor
 * - Parses command parameters using SplitKICK() to extract channels, target, and reason
 * - Validates command format and parameter availability
 * - Extracts individual components: channel list, target username, and optional reason
 * - Splits comma-separated channel list into individual channels for processing
 * - For each specified channel, performs comprehensive validation:
 *   - Verifies the channel exists on the server
 *   - Confirms the kicker is a member of the channel
 *   - Ensures the kicker has operator/admin privileges
 *   - Validates the target user is actually in the channel
 * - Upon successful validation, executes the kick:
 *   - Broadcasts KICK message to all channel members (except kicker)
 *   - Includes reason in broadcast if provided, otherwise uses default format
 *   - Removes target user from channel (handles both regular members and admins)
 *   - Automatically removes empty channels when last user is kicked
 * - Continues processing remaining channels even if some operations fail
 * - Sends appropriate error responses for invalid conditions
 *
 * @note Only channel operators/admins can kick other users
 * @note Supports kicking from multiple channels in a single command
 * @note Empty channels are automatically removed after the last user is kicked
 * @see RFC 2812 Section 3.2.8 for complete KICK command specifications
 */
void Server::KICK(std::string cmd, int fd)
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
		else if (!channel->get_adminByFd(fd))
		{
			_sendResponse(ERROR_NOT_CHANNEL_OP(channel->get_name()), fd);
			continue ;
		}
		else if (!channel->isClientInChannel(target_user))
		{
			_sendResponse(ERROR_NOT_IN_CHANNEL(target_user, channel->get_name()), fd);
			continue ; // Continue to next target
		}
		// Kick execution
		else
		{
			if (reason.empty())
				channel->broadcast_messageExcept(MSG_KICK_USER(client_nick, get_client(fd)->get_username(), channel->get_name(), target_user), fd);
			else
				channel->broadcast_messageExcept(MSG_KICK_USER_REASON(client_nick, get_client(fd)->get_username(), channel->get_name(), target_user, reason), fd);

			if (channel->get_adminByFd(channel->get_clientByname(target_user)->get_fd()))
				channel->remove_admin(channel->get_clientByname(target_user)->get_fd());
			else
				channel->remove_client(channel->get_clientByname(target_user)->get_fd());

			if (channel->get_totalUsers() == 0)
			{
				std::string channel_name = channel->get_name();
				RemoveChannel(channel_name);
			}
		}
	}
}
