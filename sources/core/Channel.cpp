#include "../../includes/core/Channel.hpp"

Channel::Channel()
{
	this->_server = NULL;
	this->_inviteOnly = 0;
	this->_topic = 0;
	this->_key = 0;
	this->_limit = 0;
	this->_topicRestriction = false;
	this->_name = "";
	this->_topicName = "";
	char characters[] = {'i', 't', 'k', 'o', 'l'};
	for(size_t i = 0; i < sizeof(characters)/sizeof(characters[0]); i++)
		_modes.push_back(std::make_pair(characters[i], false));
	this->_createdAt = "";
}
Channel::~Channel(){}
Channel::Channel(Channel const &src){*this = src;}
Channel &Channel::operator=(Channel const &src){
	if (this != &src)
	{
		this->_server = src._server;
		this->_inviteOnly = src._inviteOnly;
		this->_topic = src._topic;
		this->_key = src._key;
		this->_limit = src._limit;
		this->_topicRestriction = src._topicRestriction;
		this->_name = src._name;
		this->_password = src._password;
		this->_createdAt = src._createdAt;
		this->_topicName = src._topicName;
		this->_clients = src._clients;
		this->_admins = src._admins;
		this->_modes = src._modes;
	}
	return *this;
}

/*****************/
/*    Setters    */
/*****************/
void Channel::set_server(Server *server){this->_server = server;}
void Channel::set_inviteOnly(int invitation){this->_inviteOnly = invitation;}
void Channel::set_topicMode(int topic){this->_topic = topic;}
void Channel::set_topicModificationTime(std::string time){this->_timeCreation = time;}
void Channel::set_keyMode(int key){this->_key = key;}
void Channel::set_userLimit(int limit){this->_limit = limit;}
void Channel::set_topicName(std::string topic_name){this->_topicName = topic_name;}
void Channel::set_topicCreator(std::string creator){this->_topicCreator = creator;}
void Channel::set_password(std::string password){this->_password = password;}
void Channel::set_name(std::string name){this->_name = name;}
void Channel::set_topicRestriction(bool value){this->_topicRestriction = value;}
void Channel::set_modeAtIndex(size_t index, bool mode){_modes[index].second = mode;}
void Channel::set_channelCreationTime(){this->_createdAt = Server::getCurrentTime();}

/*****************/
/*    Getters    */
/*****************/
int Channel::get_invitOnly(){return this->_inviteOnly;}
int Channel::get_topicMode(){return this->_topic;}
int Channel::get_keyMode(){return this->_key;}
int Channel::get_userLimit(){return this->_limit;}
int Channel::get_totalUsers(){return this->_clients.size() + this->_admins.size();}
bool Channel::get_topicRestriction() const{return this->_topicRestriction;}
bool Channel::get_ModeAtIndex(size_t index){return _modes[index].second;}
bool Channel::isClientInChannel(std::string &nick){return get_clientByname(nick) != NULL;}
std::string Channel::get_topicName(){return this->_topicName;}
std::string Channel::get_topicCreator() const {return this->_topicCreator;}
std::string Channel::get_password(){return this->_password;}
std::string Channel::get_name(){return this->_name;}
std::string Channel::get_topicModificationTime(){return this->_timeCreation;}
std::string Channel::get_channelCreationTime(){return _createdAt;}

/**
 * @brief Gets active channel modes formatted as a mode string.
 * @return std::string Formatted mode string (e.g., "+itk") or empty string if no modes
 * @note Excludes 'o' (operator) mode from the result as it's handled separately
 */
std::string Channel::get_activeModes()
{
	std::string result;
	for(size_t i = 0; i < _modes.size(); i++)
	{
		if(_modes[i].first != 'o' && _modes[i].second)
			result.push_back(_modes[i].first);
	}
	if(!result.empty())
		result.insert(result.begin(),'+');
	return (result);
}

/**
 * @brief Gets formatted list of channel members with operator prefixes.
 * @return std::string Space-separated list with "@" prefix for operators
 */
std::string Channel::get_memberList()
{
	std::ostringstream oss;

	// Add admins with @ prefix
	for (size_t i = 0; i < _admins.size(); i++)
	{
		if (i > 0)
			oss << " "; // Add space before (except first)
		oss << "@" << _admins[i].get_nickname(); // Add @username
	}

	// Add regular clients
	for(size_t i = 0; i < _clients.size(); i++)
	{
		if(i > 0 || !_admins.empty()) // Add space before each client (except first client)
			oss << " ";
		oss << _clients[i].get_nickname();
	}

	return oss.str();
}

Client *Channel::get_clientByFd(int fd)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->get_fd() == fd)
			return &(*it);
	}
	return NULL;
}

Client *Channel::get_adminByFd(int fd)
{
	for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it){
		if (it->get_fd() == fd)
			return &(*it);
	}
	return NULL;
}

/**
 * @brief Finds client by nickname in both regular members and operators.
 * @param name Nickname to search for
 * @return Client* Pointer to client if found, NULL otherwise
 */
Client* Channel::get_clientByname(std::string name)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->get_nickname() == name)
			return &(*it);
	}
	for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
	{
		if (it->get_nickname() == name)
			return &(*it);
	}
	return NULL;
}


/*****************/
/*    Methods    */
/*****************/
void Channel::add_client(Client newClient){_clients.push_back(newClient);}
void Channel::add_admin(Client newClient){_admins.push_back(newClient);}
void Channel::remove_client(int fd)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->get_fd() == fd)
		{
			_clients.erase(it);
			break;
		}
	}
}

void Channel::remove_admin(int fd)
{
	for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
	{
		if (it->get_fd() == fd)
		{
			_admins.erase(it);
			break;
		}
	}
}

/**
 * @brief Moves a client between two vectors (e.g., member to operator).
 * @param from Source vector to remove client from
 * @param to Destination vector to add client to
 * @param nick Nickname of client to move
 * @return bool True if client was found and moved, false otherwise
 */
bool Channel::moveClientBetween(std::vector<Client> &from,
	std::vector<Client> &to, const std::string &nick)
{
	for(std::vector<Client>::iterator it = from.begin(); it != from.end(); ++it)
	{
		if(it->get_nickname() == nick)
		{
			to.push_back(*it); // Copy to destination
			from.erase(it); // Remove from source
			return (true);
		}
	}
	return (false);
}

/**
 * @brief Promotes a regular member to operator status.
 */
bool Channel::change_clientToAdmin(std::string& nick)
	{return moveClientBetween(_clients, _admins, nick);}

/**
 * @brief Demotes an operator to regular member status.
 */
bool Channel::change_adminToClient(std::string& nick)
	{return moveClientBetween(_admins, _clients, nick);}

/**
 * @brief Sends message to all channel members (operators and regular members).
 */
void Channel::broadcast_message(std::string reply)
{
	for(size_t i = 0; i <_admins.size(); i++)
		_server->_sendResponse(reply, _admins[i].get_fd());

	for(size_t i = 0; i < _clients.size(); i++)
		_server->_sendResponse(reply, _clients[i].get_fd());
}

void Channel::broadcast_message(std::string reply, std::set<int>& notified_fds)
{
	for(size_t i = 0; i <_admins.size(); i++)
	{
		if(notified_fds.find(_admins[i].get_fd()) == notified_fds.end())
		{
			_server->_sendResponse(reply, _admins[i].get_fd());
			notified_fds.insert(_admins[i].get_fd());
		}
	}
	for(size_t i = 0; i < _clients.size(); i++)
	{
		if(notified_fds.find(_clients[i].get_fd()) == notified_fds.end())
		{
			_server->_sendResponse(reply, _clients[i].get_fd());
			notified_fds.insert(_clients[i].get_fd());
		}
	}
}

/**
 * @brief Sends message to all channel members except specified file descriptor.
 * @param fd File descriptor to exclude from broadcast
 */
void Channel::broadcast_messageExcept(std::string reply, int fd)
{
	for(size_t i = 0; i < _admins.size(); i++)
	{
		if(_admins[i].get_fd() != fd)
			_server->_sendResponse(reply, _admins[i].get_fd());
	}
	for(size_t i = 0; i < _clients.size(); i++)
	{
		if(_clients[i].get_fd() != fd)
			_server->_sendResponse(reply, _clients[i].get_fd());
	}
}

void Channel::broadcast_messageExcept(std::string reply, int fd, std::set<int>& notified_fds)
{
	for(size_t i = 0; i < _admins.size(); i++)
	{
		if(_admins[i].get_fd() != fd)
		{
			if(notified_fds.find(_admins[i].get_fd()) == notified_fds.end())
			{
				_server->_sendResponse(reply, _admins[i].get_fd());
				notified_fds.insert(_admins[i].get_fd());
			}
		}
	}
	for(size_t i = 0; i < _clients.size(); i++)
	{
		if(_clients[i].get_fd() != fd)
		{
			if(notified_fds.find(_clients[i].get_fd()) == notified_fds.end())
			{
				_server->_sendResponse(reply, _clients[i].get_fd());
				notified_fds.insert(_clients[i].get_fd());
			}
		}
	}
}
