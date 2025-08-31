#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Server.hpp"
#include <utility>
#include <ctime>


//forward declaration
class Client;
class Server;

class Channel
{
	private:
	Server* _server;
	int _inviteOnly;
	int _topic;
	int _key;
	int _limit;
	bool _topicRestriction;
	std::string _name;
	std::string _timeCreation;
	std::string _password;
	std::string _createdAt;
	std::string _topicName;
	std::string _topicCreator;
	std::vector<Client> _clients;
	std::vector<Client> _admins;
	std::vector<std::pair<char, bool> > _modes;

	public:
	Channel();
	~Channel();
	Channel(Channel const &src);
	Channel &operator=(Channel const &src);

	/*****************/
	/*    Setters    */
	/*****************/
	void set_server(Server *server);
	void set_inviteOnly(int invitation);
	void set_topicMode(int topic);
	void set_topicModificationTime(std::string time);
	void set_keyMode(int key);
	void set_userLimit(int limit);
	void set_topicName(std::string topic_name);
	void set_topicCreator(std::string creator);
	void set_password(std::string password);
	void set_name(std::string name);
	void set_topicRestriction(bool value);
	void set_modeAtIndex(size_t index, bool mode);
	void set_channelCreationTime();

	/*****************/
	/*    Getters    */
	/*****************/
	int get_invitOnly();
	int get_topicMode();
	int get_keyMode();
	int get_userLimit(); // GetLimit
	int get_totalUsers(); //antes GetClientsNumber
	bool get_topicRestriction() const;
	bool get_ModeAtIndex(size_t index);
	bool isClientInChannel(std::string &nick);
	std::string get_topicName();
	std::string get_topicCreator() const;
	std::string get_password();
	std::string get_name();
	std::string get_topicModificationTime();
	std::string get_channelCreationTime();
	std::string get_memberList(); // clientChannel_list
    std::string get_activeModes();
	Client *get_clientByFd(int fd);
	Client *get_adminByFd(int fd);
	Client* get_clientByname(std::string name);

	/*****************/
	/*    Methods    */
	/*****************/
	void add_client(Client newClient);
	void add_admin(Client newClient);
	void remove_client(int fd);
	void remove_admin(int fd);
	bool moveClientBetween(std::vector<Client> &from,
		std::vector<Client> &to, const std::string &nick);
	bool change_clientToAdmin(std::string& nick);
	bool change_adminToClient(std::string& nick);
	void broadcast_message(std::string reply);
	void broadcast_messageExcept(std::string reply, int fd);
};

#endif
