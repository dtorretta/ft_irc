#include "../../includes/core/Server.hpp"

/**
 * @brief Parses PART command parameters to extract channel names.
 * @param command The complete PART command string received from the client
 * @return std::vector<std::string> Vector of channel names to leave
 *
 * @details Parses the PART command syntax which supports leaving multiple channels:
 * - Splits the command by spaces to extract the channel list
 * - Handles comma-separated channel lists (e.g., "#chan1,#chan2")
 * - Returns a vector containing individual channel names
 * - Example: "PART #chan1,#chan2 :Goodbye" returns ["#chan1", "#chan2"]
 *
 * @note The reason message (after ':') is not extracted by this function
 * @see RFC 2812 Section 3.2.2 for PART command syntax specifications
 */
std::vector<std::string> Server::SplitPART(std::string command)
{
	std::vector<std::string> args = split_cmd(command);  //Output: ["PART", "#chan1,#chan2"]

	if (args.size() < 2)
		return (std::vector<std::string>());

	std::string channels_part = args[1]; // "#chan1,#chan2"
	std::istringstream channel_stream(channels_part);
	std::string single_channel;
	std::vector<std::string> channels;

	while (std::getline(channel_stream, single_channel, ','))
		channels.push_back(single_channel); // channels = ["#chan1", "#chan2"]

	return (channels);
}

/**
 * @brief Handles the IRC PART command for leaving one or more channels.
 * @param cmd The complete PART command string received from the client
 * @param fd File descriptor of the client who sent the command
 * @return void
 *
 * @details Processes the IRC PART command which allows clients to leave channels:
 * - Verifies the client is registered and authenticated on the server
 * - Retrieves the client object associated with the file descriptor
 * - Parses command parameters to extract channel names using SplitPART()
 * - Validates that at least one channel name is provided
 * - Extracts optional reason message (text after ':' character, defaults to "Leaving")
 * - For each specified channel:
 *   - Verifies the channel exists on the server
 *   - Confirms the client is actually a member of the channel
 *   - Broadcasts PART message to all remaining channel members
 *   - Removes the client from the channel (handles both regular members and admins)
 * - Sends appropriate error responses for non-existent channels or membership issues
 *
 * @note Supports leaving multiple channels in a single command with comma separation
 * @note Default reason "Leaving" is used if no custom reason is provided
 * @see RFC 2812 Section 3.2.2 for complete PART command specifications
 */
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
