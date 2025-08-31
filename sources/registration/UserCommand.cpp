#include "../../includes/core/Server.hpp"


/**
 * @brief Handles IRC USER command for completing client registration.
 * @param cmd The complete USER command string from client
 * @param fd The file descriptor of the client sending the command
 * @return void
 *
 * @details Implements the final step of IRC client registration:
 * - Parses USER command parameters (username, hostname, servername, realname)
 * - Validates minimum required parameter count (5 tokens including command)
 * - Requires prior password authentication via PASS command
 * - Prevents re-registration of already registered clients
 * - Sets client username from first parameter
 * - Completes registration process if all requirements met
 * - Sends welcome message upon successful registration
 *
 * @note USER is the final command in IRC registration sequence (PASS → NICK → USER)
 * @note Format: USER <username> <hostname> <servername> :<realname>
 * @note Client becomes fully registered and can use IRC commands after USER
 * @see Server::PASS() for password authentication step
 * @see Server::NICK() for nickname registration step
 * @see isregistered() for checking complete registration status
 */
void Server::USER(std::string cmd, int fd)
{
	Client* cli = get_client(fd);
	if(!cli)
		return;

	std::vector<std::string> commands = split_cmd(cmd);

	//1. Check number of parameters
	if(commands.size() < 5)
	{
		if(!cli->get_nickname().empty())
			_sendResponse(ERROR_INSUFFICIENT_PARAMS(cli->get_nickname()), fd);
		else
			_sendResponse(ERROR_INSUFFICIENT_PARAMS(std::string("*")), fd);
		return;
	}

	//2. Verify password if it is required
	if(!cli->get_passRegistered())
	{
		if(!cli->get_nickname().empty())
			_sendResponse(ERROR_NOT_REGISTERED_YET(cli->get_nickname()), fd);
		else
			_sendResponse(ERROR_NOT_REGISTERED_YET(std::string("*")), fd);
		return;
	}

	//3. Check that the client is not already registered
	if (isregistered(fd) || !cli->get_username().empty())
	{
		 if(!cli->get_nickname().empty())
			_sendResponse(ERROR_ALREADY_REGISTERED(cli->get_nickname()), fd);
		else
			_sendResponse(ERROR_ALREADY_REGISTERED(std::string("*")), fd);
		return;
	}

	//4. Set username
	if(cli && cli->get_passRegistered())
		cli->set_username(commands[1]);

	//5. Mark as logged_in if now meeting the requirements
	if(this->isregistered(fd))
	{
		cli->set_logedIn(true);
		_sendResponse(MSG_WELCOME(cli->get_nickname()), fd);
	}
}
