#include "../includes/Server.hpp"

void printBanner()
{
    std::cout   << "███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗" 
                << "██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗"
                << "███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝"
                << "╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗"
                << "███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║"
                << "╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝"<< std::endl;
}

/*
find, si no encuentra lo que buscas(es decir, todos SON digitos), devuelve std::string::npos
c_str() --> Es necesario para funciones antiguas de C, como atoi, que requieren cadenas en formato char* o const char*, no std::string.
*/
bool portValidation(std::string port)
{
    if(port.find_first_not_of("0123456789") == std::string::npos  //version simplicada del isdigit sin neesidad de un loop ni iterar
        && std::atoi(port.c_str()) >= 1024    // 0 a 1023 --> puertos rervados
        && std::atoi(port.c_str()) <= 65535)  // maximo
        return 1;

    return 0;
}
int main (int ac, char** av)
{
    if(ac != 3)
    {
        std::cerr << RED << "Correct usage: ./ircserv [port] [password]" << RESET << std::endl; return 1;
    }
    printBanner();
    Server newServer; //create the server object   // TO DO CONSTRUCTOR
    
    try
    {
        //agregar signals
        
        if (!portValidation(av[1]) || std::string(av[2]).empty()) //es necesario limitar el largo de la pass?
            std::cerr << "Error: Invalid Port or Password" << std::endl; return 1;
        
        newServer.init(std::atoi(av[1]), std::string(av[2]));
        
        std::cout << YELLOW << "Waiting for a client to get connected..." << RESET << std::endl;
        newServer.execute(); //aca esta el loop principal de poll
        
        newServer.ft_close(-42) //cierra todo al final de la ejecucion, pero no los esta removiendo
        newServer.closeFds(); //TO DO
    }
    catch(const std::exception& e) 
    {
        newServer.closeFds(); //TO DO
        std::cerr << RED << e.what() << RESET << std::endl;
    }  
    std::cout << "Server Closed!" << std::endl;
    return 0;
}