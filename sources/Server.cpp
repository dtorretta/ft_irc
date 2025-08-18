#include "../includes/Server.hpp"

Server::Server(int port, std::string pass)
{
        this->_pass = pass;
        this->_port = port;
        this->_signalRecieved = false;
        this->_listeningSocket = -1; 
}

Server::Server(Server const &copy)
{
    this->_pass = copy._pass;
    this->_port = copy._port;
    this->_signalRecieved = copy._signalRecieved;
    this->_listeningSocket = copy._listeningSocket; 
    this->_fds = copy._fds; 
    this->_clients = copy._clients;
    //this->_channels = copy._channels;
}

Server& Server::operator=(Server const &copy)
{
	if(this != &copy)
    {
        this->_pass = copy._pass;
        this->_port = copy._port;
        this->_signalRecieved = copy._signalRecieved;
        this->_listeningSocket = copy._listeningSocket; 
        this->_fds = copy._fds; 
        this->_clients = copy._clients;
        //this->_channels = copy._channels;
    }
    return(*this);
}

Server::~Server()
{
    for(size_t i = 0; i < _clients.size(); i++)
        std::cout << YELLOW << "Client <" << _clients[i].get_fd()  << "> Disconnected" << RESET << std::endl; //ver si tengo que hacer un getter del fd para el client

    for (size_t i = 0; i < _fds.size(); i++) //incluye el _listeningSocket 
        close(_fds[i].fd);

    //necesito hacer algo especifico de remove channel?

    _channels.clear();
    _clients.clear(); //esto me ahorra de hacer un remove elemento por elemento
    _fds.clear(); //esto me ahorra de hacer un remove elemento por elemento
    this->_listeningSocket = -1;
}

/*
 * Inicializa el servidor creando el socket que escuchará las conexiones entrantes (socket)
 * Configura el socket para evitar problemas al reiniciar y para que no bloquee el programa (setsockopt & fcntl)
 * Define la dirección y puerto donde el servidor aceptará conexiones (sockaddr_in addr)
 * Enlaza el socket a esa dirección y lo pone a escuchar nuevas conexiones (bind & listen)
 * Finalmente, crea un nuevo nodo, con la configuracion necesaria, para agregarlo  a _fds (pollfd listenPollFd)

    socket --> creates as new TCP IPv4 socket. Its eturn value is a fd. this socket is la puerta de entrada para clientes
    setsockopt --> to avoid “Address already in use” error when quickly restarting the server
    fcntl --> Cambia el modo del socket para que las operaciones de lectura/escritura no bloqueen el proceso.
    sockaddr_in addr --> estruct to indicar la dirección IP y puerto donde el socket escuchará
    bind --> It associets the socket with the IP adress and port setted in the struct addr
    listen --> Pone el socket en modo escucha para conexiones entrantes.
*/
void Server::init()
{
    // this->_pass = pass;
    // this->_port = port;
    // this->_signalRecieved = false;

    this->_listeningSocket = socket(AF_INET, SOCK_STREAM, 0); //Crea un nuevo socket (fd) que usa la direccion IPv4 y el protocolo TCP (enviar/recibir datos de manera confiable)
    if (_listeningSocket < 0)
        throw(std::runtime_error("Failed to create socket"));
    
    int enable = 1; //1 = true   
    if (setsockopt(_listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
        throw(std::runtime_error("Failed to set SO_REUSEADDR on listening socket"));

    if (fcntl(_listeningSocket, F_SETFL, O_NONBLOCK) < 0) // si llamas a accept() y no hay conexiones esperando, en vez de quedarse esperando, retorna error.
        throw(std::runtime_error("Failed to set non-blocking mode on listening socket"));

    //al crear un socket se necesita crear un nuevo elemento de la estructura sockaddr_in para indicar a qué dirección IP y puerto se debe “atar” ese socket 
    //solo en estos casos, definimos tambien los elementos de addr, necesarios para usar 'bind' (en el caso de clientes nuevos omitimos esto)
    //Esta estructura es la manera que el sistema operativo usa para representar direcciones de red IPv4 en C/C++
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr)); //por buena practica
    addr.sin_family = AF_INET; // Type of adress: IPv4
    addr.sin_port = htons(this->_port); //es el puerto que el socket de escucha usara, convertido a red (big endian)
    addr.sin_addr.s_addr = INADDR_ANY; //escucha en todas las interfaces (todas las IPs del servidor).
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero)); //limpia los bytes de relleno para evitar basura en la estructura.

    if (bind(_listeningSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) //enlaza _listeningSocket con la IP y puerto de addr
        throw(std::runtime_error("Failed to bind socket"));

    if (listen(_listeningSocket, SOMAXCONN) < 0) //Pone el socket en modo escucha para conexiones entrantes.
        throw(std::runtime_error("Listen failed"));

    //new node of the pollfd struct to add to the struct _fds.  en ella se configura como debe comportarse la funcion poll con el socket asignado (el de escucha)
    struct pollfd listenPollFd;
    listenPollFd.fd = this->_listeningSocket; //el socket que debe vigilar: el listening socket
    listenPollFd.events = POLLIN; //eventos que te interesan: cuando hay nuevas conexiones pendientes
    listenPollFd.revents = 0; // eventos que ocurrieron: se inicializa a cero.
    
    _fds.push_back(listenPollFd); //añades este descriptor a tu vector _fds
}

/*
 * En el bucle principal, llamas a poll() que bloquea hasta que alguno de esos sockets (incluidos en _fds) tenga actividad.
 * Cuando poll() retorna, revisas el campo 'revents' de cada pollfd para saber qué sockets están “listos” para operar:
        - Si es el listening socket con POLLIN, hay un cliente nuevo que quiere conectar.
        - Si es un socket cliente con POLLIN, ese cliente envió datos que puedes leer.
*/    
void Server::execute()
{
    while (_signalRecieved == false)
    {
        if((poll(&_fds[0], _fds.size(), -1) < 0) && _signalRecieved == false) //timeout = -1 espera indefinidamente
            throw(std::runtime_error("poll failed"));
        for(size_t i = 0; i < _fds.size(); i++)
        {
            if(_fds[i].revents && POLLIN)
            {
                if(_fds[i].fd == _listeningSocket)
                    NewClient(); //hay una nueva conexión pendiente
                else
                    NewData(_fds[i].fd); //hay datos para leer
            }
        }
    }
}
/*
Cuando recv() no leyó todo el mensaje porque el buffer es limitado (ejemplo 1024 bytes) y quedaron datos sin leer, esos datos ya están en el buffer interno del sistema operativo (SO) para esa conexión TCP.

Entonces:

Aunque el cliente no haya enviado más datos nuevos,
El socket sigue teniendo datos pendientes en su cola interna (buffer del SO) que todavía no leíste,
Por eso, en la siguiente llamada a poll(), el descriptor del socket sigue marcándose con POLLIN, indicando que hay datos para leer,
Y al llamar a recv() de nuevo, obtendrás el siguiente fragmento del mensaje.


Cliente envía datos → llegan al buffer interno del socket en el servidor.
poll() → detecta que hay datos (POLLIN).
recv() → lees N bytes (por ejemplo 1024), y esos N bytes se eliminan del buffer interno del SO.
Si quedó más en el buffer, poll volverá a marcar POLLIN en el siguiente ciclo, incluso si el cliente ya no manda nada más.
Cuando lees todo (el buffer interno queda vacío),
    poll ya no detecta datos → no se ejecuta NewData() hasta que lleguen má

*/



/*
    * Accepts a new incoming connection from the listening socket and prepares it to be handled by the server (accept)
    * Configura el socket que no bloquee el programa (fcntl)
    * Creates and configures a new node of the pollfd struct to add it to _fds, therefore _fds can monitor this new client (pollfd newClientPollFd)
    
    accept --> Extracts the first pending connection from the listening socket's queue and returns a new socket file descriptor connected to the client.
    fcntl --> Sets the newly accepted client socket to non-blocking mode so that read/write operations will not block the server loop.
*/
void Server::NewClient()
{
    //al crear un cliente se necesita crear un nuevo elemento de la estructura sockaddr_in para indicar a dónde debe enviar datos.
    struct sockaddr_in clientAddr; //Ver si no la uso de forma global, entonces la puedo definir aca
    memset(&clientAddr, 0, sizeof(clientAddr)); //limmpia la memoria, PSRECE QUE NO ES ESTRICTAMENTE NECESARIO
    socklen_t addrLen = sizeof(clientAddr);
    int clientSocket = accept(_listeningSocket, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientSocket < 0)
        throw(std::runtime_error("Failed to accept a client"));

    if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) //poner el socket cliente en modo no bloqueante
        throw(std::runtime_error("Failed to set non-blocking mode on client socket"));

    //2. new pollfd node to add to the _fds vector
    struct pollfd newClientPollFd;
    newClientPollFd.fd = clientSocket; //el socket que debe vigilar: clientSocket
    newClientPollFd.events = POLLIN; //eventos que te interesan: datos que envíe el cliente
    newClientPollFd.revents = 0; // eventos que ocurrieron: se inicializa a cero.
    _fds.push_back(newClientPollFd);

    //3. new client node to add to the _clients vector
    Client newClient;
    newClient.set_fd(clientSocket);
	newClient.set_IPaddress(inet_ntoa((clientAddr.sin_addr))); //inet_ntoa --> convierte la dirección IPv4 binaria (in_addr) en una cadena legible 
    _clients.push_back(newClient);
    
    std::cout << YELLOW << "Client connected: fd " << clientSocket << RESET << std::endl;
}

/*
    básicamente será la función que lee datos de un socket ya existente, y decide qué hacer con ellos.
    La lógica general de NewData() en un servidor con poll() es:

    -Recibir datos del socket (recv() o read()).
    -Si el cliente cerró la conexión (recv devuelve 0) → cerrar el socket y quitarlo de _fds.
    -Si hubo error → manejarlo igual que un cierre.
    -Si hay datos recibidos → procesarlos (por ahora, imprimirlos o guardarlos).
*/
void Server::NewData(int clientFd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));  // Buena práctica: limpiar buffer TEMPORAL
    size_t bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived <= 0) // el cliente cerró o hubo error
    {
        std::cerr << RED << "Connection closed or error on client's fd " << clientFd << RESET << std::endl; //NO USAMOS THROW porque el servidor debería seguir funcionando para otros clientes.
        ft_close(clientFd); // RemoveClient, RemoveFds, close(fd)
        // RmChannels(fd); //⚠️ TO DO!!
        return;
    }
    buffer[bytesReceived] = '\0';

    Client* currentClient = this->get_client(clientFd); // Devuelve puntero al cliente encontrado
    if (!currentClient) 
        throw std::runtime_error("error client doesn't exist"); 

    // Acumular los datos recibidos en el buffer privado del cliente, NO sobrescribir
    currentClient->set_buffer(buffer); //usamos '->' porque es un puntero
        
    std::string& accumulatedBuffer = currentClient->get_buffer();

    // Revisar si el buffer acumulado contiene uno o más comandos completos terminados en \r\n
    if (accumulatedBuffer.find("\r\n") == std::string::npos)
        return; //si NO encuentra, vuelve a poll() para esperar más datos, no es el fin del comando IRC (todo el resto no se ejecuta, no se limpia el buffer del cliente)
     
    // Separar todos los comandos completos en el buffer acumulado
    std::vector<std::string> commands = split_receivedBuffer(accumulatedBuffer);
    currentClient->set_cmd(commands); //cada comando siempre esta delimitado por \r\n

    // Parsear cada comando
    for (size_t i = 0; i < currentClient->get_cmd().size(); i++)
        this->parser(currentClient->get_cmd()[i], clientFd);

    //clean the CLIENT buffer after the parser
    currentClient->clearBuffer();
}

void Server::parser(std::string &cmd, int fd)
{
    cmd = trim(cmd);
    if(cmd.empty())
        return;

    std::vector<std::string> commands = split_cmd(cmd);
    
    //normalize letters from command token to capital letters 
    for (size_t i = 0; i < commands[0].size(); i++)
        commands[0][i] = toupper(commands[0][i]);

    static CommandMap handleCommands []= {
	{"NICK", &Server::NICK}, //⚠️ TO DO!!
	{"USER", &Server::USER}, //⚠️ TO DO!!
	{"PASS", &Server::PASS}, //⚠️ TO DO!!
	{"QUIT", &Server::QUIT},
	};

    for (int i = 0; i < 4; i++)
	{
		if(!commands.empty() && commands[0] == handleCommands[i]._name)
            this->handleCommands[i]._handler(cmd, fd);
	}

    static CommandMap handleCommands2 []= {
	{"JOIN", &Server::JOIN},
	{"PRIVMSG", &Server::PRIVMSG},
	{"KICK", &Server::KICK},
	{"INVITE", &Server::INVITE},
    {"TOPIC", &Server::TOPIC},
    {"MODE", &Server::MODE},
    {"PART", &Server::PART},
	};

    if(isregistered(fd))
        for (int i = 0; i < 7; i++)
        {
            if(!commands.empty() && commands[0] == handleCommands2[i]._name)
                this->handleCommands2[i]._handler(cmd, fd);
            else
                _sendResponse(ERROR_COMMAND_NOT_RECOGNIZED(get_client(fd)->get_nickname(), commands[0]), fd);
        }
    else
        _sendResponse(ERROR_NOT_REGISTERED_YET(std::string("*")), fd);
}

// Función para separar el buffer por delimitador "\r\n"
std::vector<std::string> Server::split_receivedBuffer(std::string buffer) //no neesita ser & porque no vamos a modificar el buff, solo queremos leerlo
{
    std::vector<std::string> commands;
    std::string line;
    size_t start = 0;
    size_t end;

    // Buscar mientras haya "\r\n"
    while ((end = buffer.find("\r\n", start)) != std::string::npos) 
    {
        line = trim(buffer.substr(start, end - start));
        if (!line.empty()) 
            commands.push_back(line); // Guardar el comando
        start = end + 2; // Saltar "\r\n"
    }
    return commands;
}

//Getters
Client* Server::get_client(int fd) //con esta funcion accedemos al puntero cliente
{
    for (size_t i = 0; i < _clients.size(); i++) 
    {
        if (_clients[i].get_fd() == fd) {
            return &_clients[i]; // Devuelve puntero al cliente encontrado
        }
    }
    return NULL; // No encontrado
}



/*

_clients[i].get_fd() 

_clients es un vector de clientes que esta dentro de eserver, en la posicion [i] hace referencia a un nodo especifico de la clase Client. 
para acceder a ingotmacion de ese nodo especifico tenemos que usar un getter porque es otra clase

*/

