#pragma once

#include <iostream>
#include <vector>

//forward declaration
class Server;

class Client
{
	private:
		int _fd;
		std::string _IPaddress;
		std::string _nickname;
		std::string _username;
		std::string _buffer;
		std::vector<std::string> _channels;
		std::vector<std::string> _cmd;
		bool _logedIn; // Se usa???
		bool _passRegistered;
		bool _isQuitting;

		//bool isOperator; //borrar si al final no la usamos

	public:
		Client(); // Constructor
		Client(Client const &copy); // Copy constructor
		Client& operator=(Client const &copy); // Copy assignment operator
		~Client(); // Destructor

		/******************/
		/*     Getters    */
		/******************/
		std::string get_username() const;
		std::string get_nickname() const;
		std::string get_hostname() const;
		std::string get_IPaddress() const;
		int get_fd() const;
		const std::string& get_buffer() const; //& porque no queremos que devuelva una copia sino un pointer
		const std::vector<std::string>& get_cmd() const; //devuelve un pointer
		const std::vector<std::string>& get_channels() const;  //devuelve un pointer
		bool get_logedIn() const;
		bool get_passRegistered() const;
		bool get_channelInvitation(std::string &channel_name);
		bool get_isQuitting() const;


		/******************/
		/*     Setters    */
		/******************/
		void set_username(std::string username);
		void set_nickname(std::string nickname);
		void set_IPaddress(const std::string& address);
		void set_fd(int fd);
		void set_buffer(const std::string& chunk);
		void set_cmd(const std::vector<std::string>& cmds);
		void set_passRegistered(const bool value);
		void set_logedIn(const bool value);
		void set_isQuitting(const bool value);

		/******************/
		/*      Utils     */
		/******************/
		void clearBuffer();
		void addChannelInvitation(std::string channel_name);
		void removeChannelInvitation(std::string &channel_name);
};
