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
        std::vector<std::string> _channels; //old name: ChannelsInvite      //Lista de canales o grupos a los que pertenece el cliente (en un chat, por ejemplo).
        std::string _buffer; //old name: buffer
        std::vector<std::string> _cmd;

        // NO LAS DESCARTE, SOLO NO LLEGUE A IMPLEMENTARLAS
        //bool isOperator;
        //bool registered;
        //bool logedin;

    public:
        Client(); // Constructor
        ~Client(); // Destructor

        //Getters
        std::string get_username();  //old name: GetUserName
        std::string get_nickname();  //old name: GetNickName
        std::string ::get_hostname();
        int get_fd();  //old name: GetFd
        std::string& get_buffer();   //old name: getBuffer          //& porque no queremos que devuelva una copia sino un pointer
        std::vector<std::string>& get_cmd();  //old name: nueva          //devuelve un pointer

        //setters
        void set_IPaddress(const std::string& address); //old name: setIpAdd
        void set_fd(int fd); //old name: SetFd
        void set_buffer(const std::string& chunk); //old name: SetFd
        void set_cmd(const std::vector<std::string>& cmds); //old name: nueva

        //utils
        void clearBuffer();
        
};


