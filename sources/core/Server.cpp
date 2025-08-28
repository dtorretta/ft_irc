#include "../../includes/core/Server.hpp"


Server::Server(int port, std::string pass)
{
	this->_pass = pass;
	this->_port = port;
	this->_signalRecieved = false;
	this->_listeningSocket = -1;

	_registrationCommands["NICK"] = &Server::NICK;
	_registrationCommands["USER"] = &Server::USER;
	_registrationCommands["PASS"] = &Server::PASS;
	//_registrationCommands["QUIT"] = &Server::QUIT; //🚨 quitar comment out
	_channelCommands["JOIN"] = &Server::JOIN;
	_channelCommands["PART"] = &Server::PART;
	_channelCommands["PRIVMSG"] = &Server::PRIVMSG;
	_channelCommands["TOPIC"] = &Server::TOPIC;
	_channelCommands["INVITE"] = &Server::INVITE;
	_channelCommands["KICK"] = &Server::KICK;
	_channelCommands["MODE"] = &Server::MODE;
}

Server::Server(Server const &copy)
{
	this->_pass = copy._pass;
	this->_port = copy._port;
	this->_signalRecieved = copy._signalRecieved;
	this->_listeningSocket = copy._listeningSocket;
	this->_fds = copy._fds;
	this->_clients = copy._clients;
	this->_channels = copy._channels;
	this->_registrationCommands = copy._registrationCommands;
  	this->_channelCommands = copy._registrationCommands;
}

Server& Server::operator=(Server const &copy)
{
	if(this != &copy)
	{
		this->_pass = copy._pass;
		this->_port = copy._port;
		this->_signalRecieved = copy._signalRecieved;
		this->_listeningSocket = copy._listeningSocket;
		this->_fds = copy._fds;
		this->_clients = copy._clients;
		this->_channels = copy._channels;
		this->_registrationCommands = copy._registrationCommands;
  		this->_channelCommands = copy._registrationCommands;
	}
	return(*this);
}

Server::~Server()
{
	for(size_t i = 0; i < _clients.size(); i++)
		std::cout << YELLOW << "Client <" << _clients[i].get_fd()  << "> Disconnected" << RESET << std::endl;

	for (size_t i = 0; i < _fds.size(); i++)
		close(_fds[i].fd);

	_channels.clear();
	_clients.clear();
	_fds.clear();
	this->_listeningSocket = -1;
}


/******************/
/*     Methods    */
/******************/

/*
 * Initializes the server by creating the socket that will listen for incoming connections (socket)
 * Configures the socket to avoid problems when restarting and to make it non-blocking (setsockopt & fcntl)
 * Defines the address and port where the server will accept connections (sockaddr_in addr)
 * Binds the socket to that address and puts it into listening mode (bind & listen)
 * Finally, creates a new node, with the necessary configuration, to add it to _fds (pollfd listenPollFd)
	socket --> creates a new TCP IPv4 socket. Its return value is a fd. This socket is the entry point for clients
	setsockopt --> to avoid “Address already in use” error when quickly restarting the server
	fcntl --> changes the socket mode so that read/write operations do not block the process
	sockaddr_in addr --> struct used to indicate the IP address and port where the socket will listen
	bind --> associates the socket with the IP address and port set in the addr struct
	listen --> puts the socket into listening mode for incoming connections
*/
void Server::init()
{
	//1. Creates a new socket (fd) that uses the IPv4 address and the TCP protocol (to send/receive data reliably)
	this->_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listeningSocket < 0)
		throw(std::runtime_error("Failed to create socket"));

	int enable = 1; //1 = true
	if (setsockopt(_listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
		throw(std::runtime_error("Failed to set SO_REUSEADDR on listening socket"));

	//2. when accept() is called and there are no connections waiting, instead of blocking, it returns an error.
	if (fcntl(_listeningSocket, F_SETFL, O_NONBLOCK) < 0)
		throw(std::runtime_error("Failed to set non-blocking mode on listening socket"));

	//3. Create a new sockaddr_in structure element to specify which IP address and port this socket should be ‘bound’ to
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // Type of adress: IPv4
	addr.sin_port = htons(this->_port); //the port that the listening socket will use, converted to network byte order (big endian)
	addr.sin_addr.s_addr = INADDR_ANY; //Listen on all interfaces (all IPs of the server)
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero)); //Clears the padding bytes to avoid garbage in the structure

	//4. Bind _listeningSocket to the IP and port specified in addr
	if (bind(_listeningSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw(std::runtime_error("Failed to bind socket"));

	//5. Set the socket to listening mode for incoming connections
	if (listen(_listeningSocket, SOMAXCONN) < 0)
		throw(std::runtime_error("Listen failed"));

	//6. new node of the pollfd struct to add to the struct _fds. Here, it is configured how the poll function should behave with the assigned socket (the listening socket)
	struct pollfd listenPollFd;
	listenPollFd.fd = this->_listeningSocket; //The socket to monitor: the listening socket
	listenPollFd.events = POLLIN; //Events of interest: when there are new pending connections
	listenPollFd.revents = 0; //Occurred events: initialized to zero

	_fds.push_back(listenPollFd);
}

/*
 * In the main loop, you call poll(), which blocks until one of the sockets (included in _fds) has activity.
 * When poll() returns, you check the 'revents' field of each pollfd to know which sockets are “ready” to operate:
 *      - If it is the listening socket with POLLIN, there is a new client trying to connect.
 *      - If it is a client socket with POLLIN, that client has sent data you can read.
*/
void Server::execute()
{
	while (_signalRecieved == false)
	{
		if((poll(&_fds[0], _fds.size(), -1) < 0) && _signalRecieved == false) //timeout = -1 espera indefinidamente
			throw(std::runtime_error("poll failed"));

		if(_signalRecieved)
			break;

		for(size_t i = 0; i < _fds.size(); i++)
		{
			if(_fds[i].revents && POLLIN)
			{
				if(_fds[i].fd == _listeningSocket)
					NewClient();
				else
					NewData(_fds[i].fd);
			}
		}
	}
}

/*
 * Accepts a new incoming connection from the listening socket and prepares it to be handled by the server (accept)
 * Sets the socket to non-blocking mode (fcntl)
 * Creates and configures a new node of the pollfd struct to add it to _fds, so _fds can monitor this new client (pollfd newClientPollFd)

    accept --> Extracts the first pending connection from the listening socket's queue and returns a new socket file descriptor connected to the client.
    fcntl --> Sets the newly accepted client socket to non-blocking mode so that read/write operations will not block the server loop.
*/
void Server::NewClient()
{
	struct sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));
	socklen_t addrLen = sizeof(clientAddr);
	int clientSocket = accept(_listeningSocket, (struct sockaddr*)&clientAddr, &addrLen);
	if (clientSocket < 0)
		throw(std::runtime_error("Failed to accept a client"));

	//1. Set the client socket to non-blocking mode”
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
		throw(std::runtime_error("Failed to set non-blocking mode on client socket"));

	//2. new pollfd node to add to the _fds vector
	struct pollfd newClientPollFd;
	newClientPollFd.fd = clientSocket; //the socket to monitor: clientSocket
	newClientPollFd.events = POLLIN; //Events of interest: data sent by the client
	newClientPollFd.revents = 0; //Occurred events: initialized to zero.
	_fds.push_back(newClientPollFd);

	//3. new client node to add to the _clients vector
	Client newClient;
	newClient.set_fd(clientSocket);
	newClient.set_IPaddress(inet_ntoa((clientAddr.sin_addr))); //inet_ntoa --> Convert the binary IPv4 address (in_addr) into a readable string
	_clients.push_back(newClient);

	std::cout << YELLOW << "Client connected: fd " << clientSocket << RESET << std::endl;
}

/*
 * Basically, this will be the function that reads data from an existing socket and decides what to do with it.
 * The general logic of NewData() in a server using poll() is:
 *
 * - Receive data from the socket (recv() or read()).
 * - If the client closed the connection (recv returns 0) → close the socket and remove it from _fds.
 * - If there was an error → handle it the same way as a closure.
 * - If data was received → process it.
*/
void Server::NewData(int clientFd)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	size_t bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

	if (bytesReceived <= 0) //The client closed the connection or an error occurred
	{
		std::cerr << RED << "Connection closed or error on client's fd " << clientFd << RESET << std::endl;
		ft_close(clientFd);
		return;
	}
	buffer[bytesReceived] = '\0';

	Client* currentClient = this->get_client(clientFd);
	if (!currentClient)
		throw std::runtime_error("error client doesn't exist");

	//1. Accumulate the received data in the client’s private buffer, DO NOT overwrite
	currentClient->set_buffer(buffer);

	const std::string& accumulatedBuffer = currentClient->get_buffer();

	//2. Check if the accumulated buffer contains one or more complete commands ending with \r\n
	if (accumulatedBuffer.find("\r\n") == std::string::npos)
		return; //If not found, return to poll() to wait for more data; it is not the end of the IRC command.

	//3. Split all complete commands in the accumulated buffer
	std::vector<std::string> commands = split_receivedBuffer(accumulatedBuffer);
	currentClient->set_cmd(commands); //Each command is always delimited by \r\n

	//4. Parse each command
	for (size_t i = 0; i < currentClient->get_cmd().size(); i++)
		this->parser(currentClient->get_cmd()[i], clientFd);

	//5. Clean the client's buffer after parsing
	currentClient->clearBuffer();
}

void Server::parser(const std::string &command, int fd)
{
	std::string cmd = normalize_param(command, false);
	if(cmd.empty())
		return;

	std::vector<std::string> commands = split_cmd(cmd);

	//1. normalize letters from command token to capital letters
	for (size_t i = 0; i < commands[0].size(); i++)
		commands[0][i] = toupper(commands[0][i]);

	std::string cmdName = commands[0];

	//2. Check command registration
	std::map<std::string, CommandHandler>::iterator it = _registrationCommands.find(cmdName);
	if (it != _registrationCommands.end())
	{
		CommandHandler handler = it->second;
		(this->*handler)(cmd, fd);
		return;
	}

	//3. Check if the user is registered for channel commands and is logged in
	if (isregistered(fd) && get_client(fd)->get_logedIn())
	{
		std::map<std::string, CommandHandler>::iterator it2 = _channelCommands.find(cmdName);
		if (it2 != _channelCommands.end())
		{
			CommandHandler handler = it->second;
			(this->*handler)(cmd, fd);
		}
		else
			_sendResponse(ERROR_COMMAND_NOT_RECOGNIZED(get_client(fd)->get_nickname(), cmdName), fd);
	}
	else
		_sendResponse(ERROR_NOT_REGISTERED_YET(std::string("*")), fd);
}


// Method to split the buffer using the "\r\n" delimiter
std::vector<std::string> Server::split_receivedBuffer(std::string buffer) //no neesita ser & porque no vamos a modificar el buff, solo queremos leerlo
{
	std::vector<std::string> commands;
	std::string line;
	size_t start = 0;
	size_t end;

	//Search while there is "\r\n"
	while ((end = buffer.find("\r\n", start)) != std::string::npos)
	{
		line = normalize_param(buffer.substr(start, end - start), false);
		if (!line.empty())
			commands.push_back(line);
		start = end + 2; //skip "\r\n"
	}
	return commands;
}

void Server::addChannel(Channel newChannel){this->_channels.push_back(newChannel);}



/*****************/
/*    Getters    */
/*****************/

Client* Server::get_client(int fd)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i].get_fd() == fd) {
			return &_clients[i];
		}
	}
	return NULL;
}

Client *Server::get_clientNick(std::string nickname)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].get_nickname() == nickname)
			return &this->_clients[i];
	}
	return NULL;
}

Channel* Server::get_channelByName(const std::string& name)
{
	for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->get_name() == name)
			return &(*it);
	}
	return NULL;
}
