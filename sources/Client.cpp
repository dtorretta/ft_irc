#include "../includes/Client.hpp"

//setter
void Client::set_IPaddress(const std::string& address){_IPaddress = address;}

void Client::set_fd(int fd){_fd = fd;}

void Client::set_buffer(const std::string& chunk){_buffer += chunk;} //_buffer = chunk sobrrescribe, por eso +=

void Client::set_cmd(const std::vector<std::string>& cmds){_cmd = cmds;}   

//getters
int Client::get_fd(){return _fd;}

std::string& Client::get_buffer(){return _buffer;}

std::vector<std::string>& Client:get_cmd(){return _cmd;}


/*

setters:

- usamos & para evitar hace4r una copia extra cuando lo pasamos omo parametro. 
    De todos modos al hacer '_cmd = cmds' vamos a guardar una copia 
- usamos const que significa que dentro del setter no podés modificar el parámetro cmds. 
    Asignar (copiar) no es modificar el objeto original, es crear una copia interna.


    Tipos primitivos (int, char, bool, etc.) → pasarlos por valor normal. (nousamos const ni &)


*/