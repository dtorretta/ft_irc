#include "../../includes/core/Server.hpp"

extern Server* g_server;

/**
 * @brief Signal handler for graceful server shutdown.
 * @param sig The signal number received (unused but required by signal handler interface)
 * @return void
 *
 * @details Sets server shutdown flag when signals are received:
 * - Handles SIGINT (Ctrl+C) and other termination signals
 * - Sets _signalRecieved flag to trigger graceful shutdown
 * - Allows server main loop to detect shutdown request
 * - Enables proper cleanup of resources before exit
 *
 * @note Static function registered with signal() system call
 * @see Server::execute() for main loop that checks shutdown flag
 */
void Server::signalHandler(int sig)
{
	(void) sig;
	_signalRecieved = true;
}

/**
 * @brief Checks if a client has completed the full IRC registration process.
 * @param fd The file descriptor of the client to check
 * @return bool True if client is fully registered, false otherwise
 */
bool Server::isregistered(int fd)
{
	if (!get_client(fd) ||
		!get_client(fd)->get_passRegistered() ||
		get_client(fd)->get_nickname().empty() ||
		get_client(fd)->get_username().empty() ||
		get_client(fd)->get_nickname() == "*") //cuando usamos el *???
		return false;

	return true;
}

/**
 * @brief Sends a response message to a client over IRC connection.
 * @param response The response string to send to client
 * @param fd The file descriptor of the target client
 * @return void
 * @details Handles IRC message transmission:
 */
void Server::_sendResponse(std::string response, int fd)
{
	std::string colored = YELLOW + response + RESET;

	if(send(fd, colored.c_str(), colored.size(), 0) == -1)
		std::cerr << RED << "Response send() failed" << RESET << std::endl;
}

/**
 * @brief Normalizes IRC command parameters by trimming whitespace and removing prefix colon.
 * @param s The parameter string to normalize
 * @param flag If true, removes leading ':' character from parameter
 * @return std::string The normalized parameter string
 *
 * @details Performs IRC parameter cleanup:
 * - Removes leading and trailing whitespace (spaces, tabs, CRLF)
 * - Optionally removes leading ':' when flag is true
 * - Handles empty strings after whitespace removal
 * - Essential for proper IRC command parameter parsing
 *
 * @note IRC protocol uses ':' prefix for parameters containing spaces
 * @see IRC RFC 2812 for parameter format specifications
 * @see split_cmd() for complete command tokenization
 */
std::string Server::normalize_param(const std::string &s, bool flag)
{
	std::string result = s;

	//Remove spaces/tabs at the beginnin
	size_t start = result.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";

	//Remove spaces/tabs at the end
	size_t end = result.find_last_not_of(" \t\r\n");
	if (end != std::string::npos)
		result =  result.substr(start, end - start + 1);
	if(flag)
	{
		if(!result.empty() && result[0] == ':') //Remove ':'
			result.erase(result.begin());
	}
	return result;
}

/**
 * @brief Splits IRC command string into individual tokens for parsing.
 * @param cmd The complete IRC command string to split
 * @return std::vector<std::string> Vector of command tokens
 *
 * @details Implements IRC command tokenization:
 * - Splits command by whitespace into individual tokens
 * - Handles parameters with spaces prefixed by ':'
 * - Preserves trailing parameters as single token
 * - Uses stringstream for efficient parsing
 * - Essential for IRC protocol command processing
 *
 * @note IRC protocol format: COMMAND param1 param2 :trailing parameter
 * @note Trailing parameter (after ':') can contain spaces
 * @see normalize_param() for individual parameter cleanup
 * @see Server::parser() for command execution using parsed tokens
 */
std::vector<std::string> Server::split_cmd(std::string &cmd)
{
	std::vector<std::string> commands;
	std::istringstream iss(cmd); //converts the string into an input stream so it can work with >>
	std::string token;
	while(iss >> token) //the >> operator reads until the first whitespace and stores it in the token variable
	{
		if(token[0] == ':')
		{
			std::string rest;
			std::getline(iss, rest);
			token.erase(token.begin());
			token = token + rest;
		}
		commands.push_back(token);
		token.clear();
	}
	return commands;
}

/**
 * @brief Performs complete client cleanup and socket closure.
 * @param Fd The file descriptor of the client to disconnect
 * @return void
 *
 * @details Executes comprehensive client disconnection:
 * - Removes client from all joined channels
 * - Removes client from server client list
 * - Removes file descriptor from poll() monitoring
 * - Closes socket connection
 * - Ensures no resource leaks on client disconnect
 *
 * @note Called when client disconnects or encounters errors
 * @see RemoveClientFromChannel() for channel cleanup
 * @see RemoveClient() for client list cleanup
 * @see RemoveFd() for poll monitoring cleanup
 */
void Server::ft_close(int Fd)
{
	RemoveClientFromChannel(Fd);
	RemoveClient(Fd);
	RemoveFd(Fd);
	close(Fd);
}

/**
 * @brief Removes a client from the server's client list.
 * @param clientFd The file descriptor of the client to remove
 * @return void
 *
 * @details Searches and removes client from _clients vector:
 * - Iterates through client list to find matching file descriptor
 * - Removes client object from vector when found
 * - Breaks after removal to avoid iterator invalidation
 * - Part of comprehensive client cleanup process
 *
 * @note Called during client disconnection cleanup
 * @see ft_close() for complete disconnection process
 */
void Server::RemoveClient(int clientFd)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it->get_fd() == clientFd)
		{
			_clients.erase(it);
			break;
		}
	}
}

/**
 * @brief Removes file descriptor from poll() monitoring list.
 * @param Fd The file descriptor to remove from monitoring
 * @return void
 *
 * @details Manages poll() file descriptor cleanup:
 * - Searches _fds vector for matching file descriptor
 * - Removes pollfd structure from monitoring list
 * - Resets listening socket if it was the removed descriptor
 * - Prevents poll() from monitoring closed sockets
 *
 * @note Essential for proper poll() operation after client disconnect
 * @see ft_close() for complete disconnection process
 * @see Server::execute() for poll() usage
 */
void Server::RemoveFd(int Fd)
{
	for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); it++)
	{
		if (it->fd == Fd)
		{
			_fds.erase(it);
			break;
		}
	}
}

/**
 * @brief Removes client from all channels and broadcasts QUIT message.
 * @param fd The file descriptor of the client to remove from channels
 * @return void
 *
 * @details Handles comprehensive channel cleanup on client disconnect:
 * - Iterates through all server channels
 * - Removes client from regular member and admin lists
 * - Deletes empty channels after client removal
 * - Broadcasts QUIT message to remaining channel members
 * - Maintains channel integrity after client departures
 *
 * @note QUIT message format follows IRC protocol specification
 * @note Empty channels are automatically cleaned up
 * @see ft_close() for complete disconnection process
 * @see Channel::remove_client() and Channel::remove_admin() for member removal
 */
void Server::RemoveClientFromChannel(int fd)
{
	for (size_t i = 0; i < this->_channels.size(); i++)
	{
		if (_channels[i].get_clientByFd(fd))
			_channels[i].remove_client(fd);
		else if (_channels[i].get_adminByFd(fd))
			_channels[i].remove_admin(fd);
		if (_channels[i].get_totalUsers() == 0)
		{
			_channels.erase(_channels.begin() + i);
			i--;
			continue;
		}
	}
}

/**
 * @brief Removes a channel from the server's channel list.
 * @param name The name of the channel to remove
 * @return void
 *
 * @details Searches and removes channel from _channels vector:
 * - Iterates through channel list to find matching name
 * - Removes channel object from vector when found
 * - Returns immediately after removal
 * - Used for channel cleanup when channels become empty
 *
 * @note Called when channels have no remaining members
 * @see RemoveClientFromChannel() for automatic channel cleanup
 */
void Server::RemoveChannel(std::string &name)
{
	for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->get_name() == name)
		{
			_channels.erase(it);
			return;
		}
	}
}
