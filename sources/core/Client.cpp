#include "../../includes/core/Client.hpp"

Client::Client()
{
        this->_fd = -1;
        //this->bool isOperator = false; //borrar si al final nunca lo usamos
        this->_passRegistered = false;
        this->_logedIn = false;
}

Client::Client(Client const &copy)
{
    this->_fd = copy._fd;
    this->_IPaddress = copy._IPaddress;
    this->_nickname = copy._nickname;
    this->_username = copy._username;
    this->_buffer = copy._buffer;
    this->_channels = copy._channels;
    this->_cmd = copy._cmd;
    this->_logedIn = copy._logedIn;
    this->_passRegistered = copy._passRegistered;
}

Client& Client::operator=(Client const &copy)
{
	if(this != &copy)
	{
        this->_fd = copy._fd;
        this->_IPaddress = copy._IPaddress;
        this->_nickname = copy._nickname;
        this->_username = copy._username;
        this->_buffer = copy._buffer;
        this->_channels = copy._channels;
        this->_cmd = copy._cmd;
        this->_logedIn = copy._logedIn;
        this->_passRegistered = copy._passRegistered;
	}
	return(*this);
}


Client::~Client(){}

/*****************/
/*    Setters    */
/*****************/
void Client::set_username(std::string username){_username = username;}
void Client::set_nickname(std::string nickname){_nickname = nickname;}
void Client::set_IPaddress(const std::string& address){_IPaddress = address;}
void Client::set_fd(int fd){_fd = fd;}
void Client::set_buffer(const std::string& chunk){_buffer += chunk;}
void Client::set_cmd(const std::vector<std::string>& cmds){_cmd = cmds;}
void Client::set_passRegistered(const bool value){_passRegistered = value;}
void Client::set_logedIn(const bool value){_logedIn = value;}


/*****************/
/*    Getters    */
/*****************/
std::string Client::get_username() const {return _username;}
std::string Client::get_nickname() const {return _nickname;}
std::string Client::get_IPaddress() const {return _IPaddress;}
int Client::get_fd() const {return _fd;}
const std::string& Client::get_buffer() const {return _buffer;}
const std::vector<std::string>& Client::get_cmd() const {return _cmd;}
const std::vector<std::string>& Client::get_channels() const {return _channels;}
bool Client::get_logedIn() const {return this->_logedIn;}
bool Client::get_passRegistered() const {return this->_passRegistered;}
std::string Client::get_hostname() const 
{
	std::string hostname = this->get_nickname() + "!" + this->get_username();
	return hostname;
}


/******************/
/*      Utils     */
/******************/

void Client::clearBuffer()
{
    _buffer.clear();
}
