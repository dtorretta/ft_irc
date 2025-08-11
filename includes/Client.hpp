#pragma once

#include <iostream>
#include <vector>

class Client
{
    private:
        int _fd;
        std::string _IPaddress;
        std::string _nickname;
        std::string _username;
        std::vector<std::string> _channels; //Lista de canales o grupos a los que pertenece el cliente (en un chat, por ejemplo).


        //std::string buffer;
        //bool isOperator;
        //bool registered;
        //bool logedin;

    public:
        Client();
        ~Client();

        //getters

        //setters
        void set_IPaddress(std::string address);
        void set_fd(int fd);
};


