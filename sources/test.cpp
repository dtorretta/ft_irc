#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
//#include "../includes/Server.hpp"

void print(std::vector<std::string> commands)
{
    std::cout << "size:" << commands.size() << "\n";
    for(size_t i = 0; i < commands.size(); i++)
    {
        std::cout << commands[i] << "\n";
    }

}

// void test_split_receivedBuffer() {
//     Server server(6667, "pass");

//     //********test normal buffer *******
//     std::string buf = "NICK user\r\nUSER user 0 * :Real Name\r\n";
    
//     std::vector<std::string> commands = server.split_receivedBuffer(buf);
    
//     std::cout << YELLOW << "\ntest 0   -->  \"NICK user\\r\\nUSER user 0 * :Real Name\\r\\n\"" << RESET << std::endl;
//     print(commands);

//     assert(commands.size() == 2); //Si la condición es verdadera, no pasa nada y el programa sigue.
//     assert(commands[0] == "NICK user");
//     assert(commands[1] == "USER user 0 * :Real Name");

//     //********test empty buffer *******
//     std::string buf_empty = "";
//     std::vector<std::string> commands_empty = server.split_receivedBuffer(buf_empty);
//     assert(commands_empty.empty());

//     //******** Solo delimitador al final *******
//     std::string buf_single = "PING :server\r\n";
//     std::vector<std::string> commands_single = server.split_receivedBuffer(buf_single);
//     assert(commands_single.size() == 1 && commands_single[0] == "PING :server");

//     //******** Delimitador al inicio (comando vacío) *******
//     std::string buf_start = "\r\nUSER user 0 * :Nemo\r\n";
//     std::vector<std::string> commands_start = server.split_receivedBuffer(buf_start);
//     assert(commands_start.size() == 1 && commands_start[0] == "USER user 0 * :Nemo");
    
//     std::cout << YELLOW << "\ntest 1   -->  \"\\r\\nUSER user 0 * :Nemo\\r\\n\" " << RESET << std::endl;
//     print(commands_start);

//     //******** 4️⃣ Múltiples delimitadores seguidos *******
//     std::string buf_multiple = "NICK user\r\n\r\nUSER user 0 * :Name\r\n";
//     std::vector<std::string> commands_multiple = server.split_receivedBuffer(buf_multiple);
//     assert(commands_multiple.size() == 2);
//     assert(commands_multiple[0] == "NICK user");
//     assert(commands_multiple[1] == "USER user 0 * :Name");
//     std::cout << YELLOW << "\ntest 4   -->  \"\\r\\nUSER user 0 * :Nemo\\r\\n\" " << RESET << std::endl;
//     print(commands_multiple);

//     //******** 5️⃣ Sin delimitador final *******
//     std::string buf_no_end = "NICK user\r\nUSER user 0 * :Name"; // sin \r\n al final
//     std::vector<std::string> commands_no_end = server.split_receivedBuffer(buf_no_end);
//     assert(commands_no_end.size() == 1 && commands_no_end[0] == "NICK user");
//     std::cout << YELLOW << "\ntest 5" << RESET << std::endl;
//     print(commands_no_end);

//     //******** 6️⃣ Comando con solo espacios *******
//     std::cout << YELLOW << "\ntest 6" << RESET << std::endl;
//     std::string buf_spaces = "   \r\nNICK user\r\n";
//     std::vector<std::string> commands_spaces = server.split_receivedBuffer(buf_spaces);
//     print(commands_spaces);
//     assert(commands_spaces.size() == 1 && commands_spaces[0] == "NICK user");
    

//     std::cout << GREEN << "\nsplit_receivedBuffer test passed!\n" << RESET;
// }

// //Test de Client y acumulación
// void test_client_buffer() //hacer mas tests
// {
//     Client client;

//     // 1. Buffer vacío
//     assert(client.get_buffer().empty());

//     // 2. Un solo comando completo
//     client.set_buffer("only one\r\n");
//     std::string& buf = client.get_buffer();
//     assert(buf == "only one\r\n");
//     client.clearBuffer();
//     assert(client.get_buffer().empty());

//     // 3. Comando fragmentado: llega en dos partes
//     client.set_buffer("PART1 ");
//     client.set_buffer("PART2\r\nPART3\r\n");
//     assert(client.get_buffer() == "PART1 PART2\r\nPART3\r\n");
//     client.clearBuffer();
//     assert(client.get_buffer().empty());

//     // 4. Múltiples comandos completos en un solo set
//     client.set_buffer("CMD1\r\nCMD2\r\nCMD3\r\n");
//     assert(client.get_buffer() == "CMD1\r\nCMD2\r\nCMD3\r\n");
//     client.clearBuffer();
//     assert(client.get_buffer().empty());

//     // 5. Comando parcial seguido de comando completo
//     client.set_buffer("HELLO PART ");
//     client.set_buffer("WORLD\r\nBYE\r\n");
//     assert(client.get_buffer() == "HELLO PART WORLD\r\nBYE\r\n");
//     client.clearBuffer();
//     assert(client.get_buffer().empty());

//     // 6. Comando con solo espacios y delimitadores
//     client.set_buffer("   \r\n \r\n");
//     assert(client.get_buffer() == "   \r\n \r\n");
//     client.clearBuffer();
//     assert(client.get_buffer().empty());

//     // 7. Comando con caracteres especiales
//     client.set_buffer("NICK user_123!\r\n");
//     assert(client.get_buffer() == "NICK user_123!\r\n");
//     client.clearBuffer();
//     assert(client.get_buffer().empty());

//     // 8. Fragmentos con delimitadores intermedios
//     client.set_buffer("FRAG1\r");
//     client.set_buffer("\nFRAG2\r\n");
//     assert(client.get_buffer() == "FRAG1\r\nFRAG2\r\n");
//     client.clearBuffer();
//     assert(client.get_buffer().empty());
    
//     std::cout << GREEN << "Client buffer test passed!\n" << RESET ;
// }

// void test_command_accumulation() {
//     Client client;
//     Server server(6667, "pass");
    
//     // simula recibir dos chunks
//     client.set_buffer("NICK test");
//     client.set_buffer("\r\nUSER test 0 * :Test User\r\n");
    
//     std::string& buf = client.get_buffer();
    
//     if (buf.find("\r\n") != std::string::npos) {
//         std::vector<std::string> cmds = server.split_receivedBuffer(buf);
//         client.set_cmd(cmds);
//     }
    
//     assert(client.get_cmd().size() == 2);
//     assert(client.get_cmd()[0] == "NICK test");
//     assert(client.get_cmd()[1] == "USER test 0 * :Test User");
    
//     std::cout << GREEN << "Command accumulation test passed!\n" << RESET;
// }


std::vector<std::string> split_cmdo(std::string cmd)
{
    std::vector<std::string> commands;
    std::istringstream iss(cmd); //transforma el string en un stream de entrada para que pueda funcionar con >>
    std::string token; //el primer “token” de cada línea siempre se interpreta como el comando
    while(iss >> token) //El operador >> lee hasta el primer espacio en blanco y lo guarda en la variable token.
    {
        if(token[0] == ':')
        {
            //std::cout << token << std::endl;
            std::string rest;
            std::getline(iss, rest);
            token.erase(token.begin());
            token = token + rest;
        }
        commands.push_back(token);
        token.clear();
    }
    return commands;
}

int main()
{
    //test_split_receivedBuffer();
    //test_client_buffer();
    std::vector<std::string> commands = split_cmdo("USER daniela 0 * :Daniela Torretta");
    print(commands);
    return 0;
}

//g++ -std=c++98 -Wall -Wextra -Iincludes sources/Server.cpp sources/Client.cpp sources/test.cpp -o test_split
