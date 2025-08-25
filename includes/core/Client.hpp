#pragma once

#include <iostream>
#include <vector>

class Client
{
    private:
        int _fd; //old name: fd
        std::string _IPaddress; //old name: ipadd
        std::string _nickname; //old name: nickname
        std::string _username; //old name: username
        std::string _buffer; //old name: buffer
        std::vector<std::string> _channels; //old name: ChannelsInvite      //Lista de canales o grupos a los que pertenece el cliente (en un chat, por ejemplo).
        std::vector<std::string> _cmd;
        bool _logedin; //old name: logedin   //USER aca lo uso

        // NO LAS DESCARTE, SOLO NO LLEGUE A IMPLEMENTARLAS
        //bool isOperator;
        //bool registered;


    public:
        Client(); // Constructor
        ~Client(); // Destructor
        //copy y operaator = //todoooo ⚠️

		/******************/
		/*     Getters    */
		/******************/
        std::string get_username() const;  //old name: GetUserName
        std::string get_nickname() const;  //old name: GetNickName
        std::string get_hostname() const;
        std::string get_IPaddress() const;
        int get_fd() const;  //old name: GetFd
        const std::string& get_buffer() const;   //old name: getBuffer          //& porque no queremos que devuelva una copia sino un pointer
        const std::vector<std::string>& get_cmd() const;  //old name: nueva          //devuelve un pointer
        const std::vector<std::string>& get_channels() const;  //devuelve un pointer
        bool get_logedIn() const;

        /******************/
		/*     Setters    */
		/******************/
        void set_nickname(std::string nickname);
        void set_IPaddress(const std::string& address); //old name: setIpAdd
        void set_fd(int fd); //old name: SetFd
        void set_buffer(const std::string& chunk); //old name: SetFd
        void set_cmd(const std::vector<std::string>& cmds); //old name: nueva

        /******************/
		/*      Utils     */
		/******************/
        void clearBuffer();

};


