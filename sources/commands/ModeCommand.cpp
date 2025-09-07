#include "../../includes/core/Server.hpp"

/**
 * @brief Deactivates a specific channel mode and updates channel settings.
 * @param mode The mode character to deactivate ('i', 't', 'k', 'o', 'l')
 * @param parameter Additional parameter required for certain modes (password for 'k', username for 'o')
 * @param channel Pointer to the channel object to modify
 * @return bool True if mode was successfully deactivated, false otherwise
 *
 * @details Handles the deactivation of various IRC channel modes:
 * - **'i' (invite-only)**: Removes invite-only restriction, allows anyone to join
 * - **'t' (topic restriction)**: Removes topic restriction, allows anyone to change topic
 * - **'k' (key/password)**: Removes channel password if provided parameter matches current password
 * - **'o' (operator)**: Demotes specified user from operator to regular member
 * - **'l' (user limit)**: Removes user limit restriction, sets limit to 0 (unlimited)
 *
 * @note For 'k' mode: parameter must match current channel password for successful removal
 * @note For 'o' mode: specified user must exist and be an operator for successful demotion
 * @see activateMode() for the corresponding mode activation function
 */
bool	Server::deactivateMode(Client *client, char mode, std::string parameter, Channel *channel)
{
	switch(mode)
	{
		case 'i':
			channel->set_inviteOnly(false); channel->set_modeAtIndex(0, false);
			return (true);
		case 't':
			channel->set_topicRestriction(false); channel->set_modeAtIndex(1, false);
			return (true);
		case 'k':
			if (parameter == channel->get_password())
			{
				channel->set_password("");
				channel->set_modeAtIndex(2, false);
				return (true);
			}
			return (false);
		case 'o':
			if (channel->change_adminToClient(parameter))
			{
				channel->set_modeAtIndex(3, false);
				return (true);
			}
			return (false);
		case 'l':
			channel->set_userLimit(0); channel->set_modeAtIndex(4, false);
			return (true);
		default:
			_sendResponse(ERROR_UNRECOGNIZED_MODE(client->get_nickname(), channel->get_name(), mode), client->get_fd());
			return (false); // invalid mode
	}
}

/**
 * @brief Activates a specific channel mode and updates channel settings.
 * @param mode The mode character to activate ('i', 't', 'k', 'o', 'l')
 * @param parameter Additional parameter required for certain modes (password for 'k', username for 'o', limit for 'l')
 * @param channel Pointer to the channel object to modify
 * @return bool True if mode was successfully activated, false otherwise
 *
 * @details Handles the activation of various IRC channel modes:
 * - **'i' (invite-only)**: Sets channel to invite-only, requires invitation to join
 * - **'t' (topic restriction)**: Restricts topic changes to operators only
 * - **'k' (key/password)**: Sets channel password using provided parameter
 * - **'o' (operator)**: Promotes specified user from regular member to operator
 * - **'l' (user limit)**: Sets maximum user limit using provided numeric parameter
 *
 * @note For 'k' mode: parameter becomes the new channel password
 * @note For 'o' mode: specified user must exist and be a regular member for successful promotion
 * @note For 'l' mode: parameter is converted to integer using atoi()
 * @see deactivateMode() for the corresponding mode deactivation function
 */
bool	Server::activateMode(Client *client, char mode, std::string parameter, Channel *channel)
{
	switch(mode)
	{
		case 'i':
			channel->set_inviteOnly(true); channel->set_modeAtIndex(0, true);
			return (true);
		case 't':
			channel->set_topicRestriction(true); channel->set_modeAtIndex(1, true);
			return (true);
		case 'k':
			channel->set_password(parameter); channel->set_modeAtIndex(2, true);
			return (true);
		case 'o':
			if (channel->change_clientToAdmin(parameter))
			{
				channel->set_modeAtIndex(3, true);
				return (true);
			}
			return (false);
		case 'l':
			channel->set_userLimit(atoi(parameter.c_str())); channel->set_modeAtIndex(4, true);
			return (true);
		default:
			_sendResponse(ERROR_UNRECOGNIZED_MODE(client->get_nickname(), channel->get_name(), mode), client->get_fd());
			return (false); // invalid mode
	}
}

/**
 * @brief Validates channel existence, membership, and operator privileges for mode operations.
 * @param channel Pointer to the channel object to validate (may be NULL)
 * @param channel_string The channel name string for error reporting
 * @param client_nick The nickname of the client requesting mode changes
 * @param fd File descriptor of the client for sending error responses
 * @return bool True if all validations pass, false if any validation fails
 *
 * @details Performs comprehensive validation checks required for mode operations:
 * - **Channel existence**: Verifies the channel exists on the server
 * - **Membership verification**: Confirms the client is a member of the channel
 * - **Operator privileges**: Ensures the client has operator/admin status
 * - Sends appropriate error responses for each type of validation failure
 * - Used as a prerequisite check before allowing any mode modifications
 *
 * @note Sends ERROR_CHANNEL_NOT_EXISTS if channel doesn't exist
 * @note Sends ERROR_NOT_IN_CHANNEL if client is not a channel member
 * @note Sends ERROR_NOT_CHANNEL_OP if client lacks operator privileges
 * @see MODE() command for usage context
 */
bool	Server::isChannelValid(Channel *channel, std::string channel_string, std::string client_nick, int fd)
{
	if (!channel)
	{
		_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client_nick, channel_string), fd);
		return (false);
	}
	else if (!channel->get_clientByFd(fd) && !channel->get_adminByFd(fd))
	{
		_sendResponse(ERROR_NOT_IN_CHANNEL(client_nick, channel->get_name()), fd);
		return (false);
	}
	else if (!channel->get_adminByFd(fd))
	{
		_sendResponse(ERROR_NOT_CHANNEL_OP(channel->get_name()), fd);
		return (false);
	}
	return (true);
}

/**
 * @brief Determines if a specific mode requires an additional parameter.
 * @param mode The mode character to check ('i', 't', 'k', 'o', 'l')
 * @param operation The operation type ('+' for activation, '-' for deactivation)
 * @return bool True if the mode requires a parameter, false otherwise
 *
 * @details Determines parameter requirements for different IRC channel modes:
 * - **'k' (key/password)**: Always requires parameter (password) for both + and -
 * - **'o' (operator)**: Always requires parameter (username) for both + and -
 * - **'l' (user limit)**: Only requires parameter for '+' operation (limit number)
 * - **'i' (invite-only)**: Never requires parameter
 * - **'t' (topic restriction)**: Never requires parameter
 *
 * @note Used during mode string parsing to determine parameter consumption
 * @note Critical for proper command validation and error handling
 * @see processModeString() for usage in mode parsing logic
 */
bool	needsParameter(char mode, char operation)
{
	if (mode == 'k' || mode == 'o')
		return (true);
	if (mode == 'l' && operation == '+')
		return (true);
	else
		return (false);
}

/**
 * @brief Processes a mode string and associates parameters with modes that require them.
 * @param modeString The mode string containing operations and modes (e.g., "+oi-t+k")
 * @param parameters Reference to vector of parameters for modes that require them
 * @return std::vector<std::string> Vector of processed mode operations with parameters
 *
 * @details Parses complex mode strings and creates individual mode operations:
 * - Tracks current operation sign ('+' or '-') as it processes characters
 * - For each mode character, creates an operation string with current sign
 * - Consumes parameters from the parameter vector for modes that require them
 * - Associates parameters with their corresponding mode operations
 * - Returns empty vector if insufficient parameters are provided
 *
 * **Processing Logic**:
 * - '+' or '-' characters update the current operation sign
 * - Mode characters create operation strings like "+o", "-k", "+l"
 * - Parameter-requiring modes consume next available parameter
 * - Final operations include parameters: "+o alice", "+k password", "+l 50"
 *
 * **Example**:
 * - Input: "+oi-t+k", parameters: ["alice", "password"]
 * - Output: ["+o alice", "+i", "-t", "+k password"]
 *
 * @note Parameter index automatically advances for modes requiring parameters
 * @note Returns empty vector if not enough parameters for parameter-requiring modes
 * @see needsParameter() to determine which modes require parameters
 */
std::vector<std::string> processModeString(const std::string &modeString,
	std::vector<std::string> &parameters)
{
	std::vector<std::string> result;
	size_t paramIndex = 0;
	char currOperation = '+'; // default in case user sends 'i'

	for (size_t i = 0; i < modeString.size(); i++)
	{
		char c = modeString[i];

		if (c == '+' || c == '-')
			currOperation = c; // Update current sign
		else
		{
			std::string mode_operation = std::string(1, currOperation) + c; // "+o" or "-k"
			if (needsParameter(c, currOperation))
			{
				if (paramIndex < parameters.size())
				{
					mode_operation += " " + parameters[paramIndex++]; // "+l 50"
					result.push_back(mode_operation);
				}
				else
					return (std::vector<std::string>());
			}
			else
				result.push_back(mode_operation);
		}
	}
	return (result);
}

/**
 * @brief Parses MODE command parameters to extract channel, mode string, and parameters.
 * @param cmd The complete MODE command string received from the client
 * @return std::vector<std::string> Vector containing [channel, mode_string, param1, param2, ...]
 *
 * @details Parses the MODE command syntax which supports complex mode operations:
 * - Splits the command by spaces to extract basic components
 * - Validates minimum parameter count (requires at least channel name)
 * - Handles mode viewing (channel only) and mode setting operations
 * - Separates mode strings (starting with '+' or '-') from parameters
 * - Consolidates multiple mode strings into a single mode string
 * - Preserves parameter order for proper association with modes
 *
 * **Return Structure**:
 * - **View mode**: ["#channel", ""] - for viewing current modes
 * - **Set mode**: ["#channel", "+oi-t+k", "alice", "password"] - for setting modes
 *
 * **Examples**:
 * - "MODE #general" returns ["#general", ""]
 * - "MODE #general +o alice" returns ["#general", "+o", "alice"]
 * - "MODE #general +oi alice +k password" returns ["#general", "+oi+k", "alice", "password"]
 *
 * @note Mode strings are concatenated if multiple are provided
 * @note Parameters maintain their original order for proper mode association
 * @see RFC 2812 Section 3.2.3 for MODE command syntax specifications
 */
std::vector<std::string>	Server::SplitMODE(std::string cmd)
{
	std::vector<std::string> args = split_cmd(cmd); //Output: ["MODE", "#chan1", "+o", "alice", "-o", "bob", "+l", "50"]
	if (args.size() < 2)
		return (std::vector<std::string>());

	std::vector<std::string> result;
	result.push_back(args[1]); // [0] = channel
	if (args.size() == 2)
		return (result);

	std::string modeStrings = "";
	std::vector<std::string> params;
	for (size_t i = 2; i < args.size(); i++)
	{
		if (args[i][0] == '+' || args[i][0] == '-')
			modeStrings.append(args[i]);
		else
			params.push_back(args[i]);
	}
	result.push_back(modeStrings);
	for (size_t i = 0; i < params.size(); i++)
		result.push_back(params[i]);

	return (result); // ["#chan1", "+o-o+l", "alice", "bob", "50"]
}

/**
 * @brief Handles the IRC MODE command for viewing or modifying channel modes.
 * @param cmd The complete MODE command string received from the client
 * @param fd File descriptor of the client who sent the command
 * @return void
 *
 * @details Processes the IRC MODE command which supports both mode viewing and modification:
 * - Verifies the client is registered and authenticated on the server
 * - Retrieves the client object associated with the file descriptor
 * - Parses command parameters using SplitMODE() to extract channel and mode operations
 * - Validates command format and parameter availability
 * - Operates in two distinct modes based on parameter count:
 *
 * **Mode VIEWING** (channel only):
 *   - Validates channel existence and user permissions
 *   - Sends current active modes to the requesting client
 *   - Includes channel creation timestamp information
 *
 * **Mode MODIFICATION** (channel + mode string + parameters):
 *   - Processes complex mode strings using processModeString()
 *   - Validates channel permissions (requires operator status)
 *   - Applies each mode operation using activateMode()/deactivateMode()
 *   - Tracks successful operations for broadcasting
 *   - Builds result strings for successful mode changes and parameters
 *   - Broadcasts mode changes to all channel members
 *   - Handles parameter association for modes requiring additional data
 *
 * **Supported Modes**:
 * - **i**: Invite-only channel
 * - **t**: Topic restriction (ops only)
 * - **k**: Channel key/password
 * - **o**: Operator privileges
 * - **l**: User limit
 *
 * @note Only successful mode changes are included in broadcast messages
 * @note Failed operations are silently ignored to continue processing remaining modes
 * @note Operator privileges are required for all mode modifications
 * @see RFC 2812 Section 3.2.3 for complete MODE command specifications
 */
void	Server::MODE(std::string cmd, int fd)
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

	//3. Parse and validate parameters
	std::vector<std::string> token = SplitMODE(cmd);
	if (token.size() == 0)
	{
		_sendResponse(ERROR_INSUFFICIENT_PARAMS(client->get_nickname()), fd);
		return ;
	}
	std::string channel_string = token[0];
	//4. Display Mode
	Channel *channel = get_channelByName(channel_string);
	if (token.size() == 1)
	{
		if (!isChannelValid(channel, channel_string, client_nick, fd))
		{
			//_sendResponse(ERROR_CHANNEL_NOT_EXISTS(client_nick, channel_string), fd);
			return ;
		}
		std::string modes = channel->get_activeModes();
		if (modes.empty())
			modes = "+";
		_sendResponse(MSG_CHANNEL_MODES(client_nick, channel_string, modes), fd);
		_sendResponse(MSG_CREATION_TIME(client_nick, channel_string, channel->get_channelCreationTime()), fd);
	}
	//5. Handle Modes
	else
	{
		// Parse mode operations
		std::string modeString = token[1]; // "+k+o+l"
		if (!modeString.empty())
		{
			std::vector<std::string> params(token.begin() + 2, token.end()); // ["mypass", "alice", "50"]
			std::vector<std::string> operations = processModeString(modeString, params);
			if (operations.empty() && !modeString.empty())
			{
				_sendResponse(ERROR_INSUFFICIENT_PARAMS(client->get_nickname()), fd);
				return;
			}
			if (!isChannelValid(channel, channel_string, client_nick, fd))
				return ;

			// Apply Modes changes and track success for broadcasting
			std::string successfulModes = ""; // Will build "+o-t+k"
			std::string modeParams = ""; // Will build "alice password"
			for (size_t i = 0; i < operations.size(); i++)
			{
				char sign = operations[i][0]; // '+' or '-'
				char mode = operations[i][1]; // 'i', 't', 'k', 'o', 'l'
				std::string parameter = "";
				size_t SpacePos = operations[i].find(' ');
				if (SpacePos != std::string::npos)
					parameter = operations[i].substr(SpacePos + 1);

				bool success = false;
				if (operations[i][0] == '+')
					success = activateMode(client, mode, parameter, channel);
				else
					success = deactivateMode(client, mode, parameter, channel);

				if (success)
				{
					successfulModes += sign;
					successfulModes += mode;

					if (!parameter.empty() && needsParameter(mode, sign))
					{
						if (!modeParams.empty())
							modeParams += " ";
						modeParams += parameter;
					}
				}
			}
			//Broadcast to all channel members
			if (!successfulModes.empty())
				channel->broadcast_message(MSG_MODE_CHANGE(client_nick, client->get_username(),
					channel_string, successfulModes, modeParams));
		}
	}
}
