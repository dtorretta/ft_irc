#include "../../includes/core/Server.hpp"

/**
 * @brief Handles the IRC INVITE command to invite a user to a channel.
 * @param cmd The complete command string received from the client
 * @param fd File descriptor of the client who sent the command
 * @return void
 *
 * @details Processes an IRC INVITE command which allows a user to invite another user
 * to join a specific channel. The function performs multiple validation checks:
 * - Verifies the inviting user is registered and authenticated
 * - Validates command syntax and parameter count (expects exactly 3 arguments)
 * - Ensures the target channel exists and starts with '#'
 * - Confirms the inviter is a member of the channel
 * - Checks if the invited user exists on the server
 * - Verifies the invited user is not already in the channel
 * - For invite-only channels, ensures the inviter has operator privileges
 * - Checks if the channel has reached its user limit
 *
 * @note If all validations pass, adds the invitation to the guest's invitation list
 * and sends appropriate messages to both the inviter and the invited user.
 * @see RFC 2812 Section 3.2.7 for IRC INVITE command specifications
 */
void Server::INVITE(std::string cmd, int fd)
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

	//3. Parsing
	std::vector<std::string> args = split_cmd(cmd);
	if (args.size() != 3)
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(get_client(fd)->get_nickname()), fd);
		return ;
	}
	std::string guest_nick = args[1];
	Client *guest = get_clientNick(guest_nick);
	std::string channel_name = args[2];
	Channel *channel = get_channelByName(channel_name);

	//4. Validation checks
	if (!channel || channel_name.empty() || channel_name[0] != '#')
	{
		_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client_nick, channel_name), fd);
		return ;
	}
	if (!channel->get_clientByFd(fd) && !channel->get_adminByFd(fd))
	{
		_sendResponse(ERROR_NOT_IN_CHANNEL(client_nick, channel_name), fd);
		return ;
	}
	if (!get_clientNick(guest_nick))
	{
		_sendResponse(ERROR_NICK_NOT_FOUND(guest_nick, client_nick), fd);
		return ;
	}
	if (get_channelByName(channel_name)->get_clientByname(guest_nick))
	{
		_sendResponse(ERROR_ALREADY_IN_CHANNEL(client_nick, channel_name), fd);
		return ;
	}
	if (channel->get_invitOnly() && !channel->get_adminByFd(fd))
	{
		_sendResponse(ERROR_NOT_CHANNEL_OP(channel_name), fd);
		return ;
	}
	if ((get_channelByName(channel_name)->get_userLimit()
		&& get_channelByName(channel_name)->get_userLimit() >= get_channelByName(channel_name)->get_userLimit()))
	{
		_sendResponse(ERROR_CHANNEL_FULL(client_nick, channel_name), fd);
		return ;
	}

	//5. Store invite in vector and send it to guest
	guest->addChannelInvitation(channel_name);

	_sendResponse(MSG_TO_INVITER(client_nick, guest_nick, channel_name), fd);
	_sendResponse(MSG_TO_INVITEE(guest->get_hostname(), guest_nick, channel_name), guest->get_fd());
}
