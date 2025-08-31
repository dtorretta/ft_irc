#include "../../includes/core/Server.hpp"

/**
 * @brief Parses QUIT command parameters to extract the quit reason message.
 * @param cmd The complete QUIT command string received from the client
 * @return std::string The quit reason message or "Leaving" as default
 *
 * @details Parses the QUIT command syntax to extract the optional quit reason:
 * - Splits the command by spaces to separate arguments
 * - Concatenates all arguments after "QUIT" to form the reason message
 * - Handles space preservation between multiple words in the reason
 * - Removes leading ':' character if present (IRC protocol syntax)
 * - Provides default reason "Leaving" if no custom reason is specified
 * - Examples:
 *   - "QUIT" returns "Leaving" (default)
 *   - "QUIT :Going to bed" returns "Going to bed"
 *   - "QUIT See you later" returns "See you later"
 *
 * @note Leading ':' character is automatically stripped from the reason
 * @note Multiple words are properly concatenated with spaces preserved
 * @see RFC 2812 Section 3.1.7 for QUIT command syntax specifications
 */
std::string	Server::SplitQUIT(std::string cmd)
{
	// Split by spaces
	std::vector<std::string> args = split_cmd(cmd);
	std::string result;
	for (size_t i = 1; i < args.size(); i++)
	{
		if (!result.empty())
			result += " ";
		result += args[i];
	}

	// Remove leading ':' if present
	if (!result.empty() && result[0] == ':')
		result.erase(0, 1);

	// Default reason if empty
	if (result.empty())
		result = "Leaving";

	return (result);
}

/**
 * @brief Handles the IRC QUIT command for client disconnection from the server.
 * @param cmd The complete QUIT command string received from the client
 * @param fd File descriptor of the client who sent the command
 * @return void
 *
 * @details Processes the IRC QUIT command which disconnects a client from the server:
 * - Verifies the client is registered and authenticated on the server
 * - Retrieves the client object associated with the file descriptor
 * - Parses command parameters using SplitQUIT() to extract optional quit reason
 * - Performs comprehensive cleanup and notification sequence:
 *
 * **Broadcast Phase**:
 *   - Iterates through all channels on the server
 *   - For each channel containing the quitting client:
 *     - Broadcasts QUIT message to all channel members
 *     - Includes client nickname, username, and quit reason in message
 *
 * **Cleanup Phase**:
 *   - Removes client from all channels they were members of
 *   - Checks for empty channels after client removal from server
 *   - Automatically removes channels with zero remaining users
 *   - Handles index adjustment during channel removal iteration
 *
 * @note QUIT messages are sent to all channels the client was in before removal
 * @note Empty channels are automatically cleaned up after user leaves
 * @note Client connection is fully closed and all resources are freed
 * @warning Index adjustment (i--) is critical during channel removal to prevent skipping
 * @see RFC 2812 Section 3.1.7 for complete QUIT command specifications
 */
void	Server::QUIT(std::string cmd, int fd)
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

	//3. Parse parameters
	std::string reason = SplitQUIT(cmd);

	//4. Broadcast message to channel(s)
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (_channels[i].get_clientByname(client_nick))
			_channels[i].broadcast_message(MSG_QUIT(client_nick, client->get_username(), reason));
	}

	//5. Remove client and close empty channel(s)
	ft_close(fd);
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (_channels[i].get_totalUsers() == 0)
		{
			std::string channel_name = _channels[i].get_name();
			RemoveChannel(channel_name);
			i--; // avoids skipping since every channel is shigting back one possition when a channel is removed
		}
	}
}
