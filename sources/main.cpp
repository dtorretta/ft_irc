#include "../includes/core/Server.hpp"

//Initialize the static global variable
bool Server::_signalRecieved = false;

void printBanner()
{
    std::cout   << "███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗\n" 
                << "██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗\n"
                << "███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝\n"
                << "╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗\n"
                << "███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║\n"
                << "╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝"<< std::endl;
    std::cout << "                -- initiated --\n\n";
}

bool portValidation(std::string port)
{
    if(port.find_first_not_of("0123456789") == std::string::npos
        && std::atoi(port.c_str()) >= 1024    // 0 to 1023 → reserved ports
        && std::atoi(port.c_str()) <= 65535)  // max
        return 1;

    return 0;
}
int main (int ac, char** av)
{
    if(ac != 3)
    {
        std::cerr << RED << "Correct usage: ./ircserv [port] [password]" << RESET << std::endl; 
        return 1;
    }
    
    try
    { 
        if (!portValidation(av[1]) || std::string(av[2]).empty()) //es necesario limitar el largo de la pass?
            throw std::runtime_error("Error: Invalid Port or Password");
        
        printBanner();
        
        Server newServer(std::atoi(av[1]), std::string(av[2]));
         
        //Signals
        std::signal(SIGINT, Server::signalHandler); // Ctrl+C
        std::signal(SIGTERM, Server::signalHandler); //kill -TERM <pid>
        std::signal(SIGQUIT, SIG_IGN); // ignore Ctrl + back slash  
        //std::signal(SIGTSTP, SIG_IGN); // ignore Ctrl + Z   

        newServer.init();        
        std::cout << YELLOW << "Waiting for a client to get connected..." << RESET << std::endl;
        newServer.execute();
    }
    catch(const std::exception& e) 
    {
        std::cerr << RED << e.what() << RESET << std::endl;
        return 1;
    }  
    std::cout << "\nServer Closed!" << std::endl;
    return 0;
}