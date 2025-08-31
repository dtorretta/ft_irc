#include "../../includes/core/Server.hpp"

/**
 * @brief Handles IRC PASS command for client authentication.
 * @param cmd The complete PASS command string from client
 * @param fd The file descriptor of the client sending the command
 * @return void
 *
 * @details Implements IRC password authentication process:
 * - Extracts and normalizes password parameter from command
 * - Validates that password parameter is provided
 * - Prevents re-authentication if client already registered
 * - Compares provided password with server password
 * - Sets client authentication flag on successful validation
 * - Sends appropriate error responses for various failure cases
 *
 * @note PASS must be the first command in IRC registration sequence
 * @note Password authentication is required before NICK/USER commands
 * @note Once registered, clients cannot re-authenticate with PASS
 * @see Server::NICK() for next step in registration sequence
 * @see Server::USER() for final step in registration sequence
 */
void Server::PASS(std::string cmd, int fd)
{
	Client* cli = get_client(fd);
	if(!cli)
		return;

	std::string pass = normalize_param(cmd.substr(4), 1);
	if(pass.empty())
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(std::string("*")), fd);
		return;
	}
	if (isregistered(fd) || cli->get_passRegistered())
	{
		_sendResponse(ERROR_ALREADY_REGISTERED(std::string("*")), fd);
		return;
	}
	if (pass != _pass) //ver
	{
		_sendResponse(ERROR_WRONG_PASSWORD(std::string("*")), fd);
		return;
	}
	cli->set_passRegistered(true);
}
