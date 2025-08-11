#include "../includes/Server.hpp"
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
void Server::init(int port, std::string pass)
{
    this->_pass = pass;
    this->_port = port;
    this->_signalRecieved = false;

    _listeningSocket = socket(AF_INET, SOCK_STREAM, 0); //Crea un nuevo socket (fd) que usa la direccion IPv4 y el protocolo TCP (enviar/recibir datos de manera confiable)
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
    struct sockaddr_in addr
    addr.sin_family = AF_INET; // Type of adress: IPv4
    addr.sin_port = htons(_port); //es el puerto que el socket de escucha usara, convertido a red (big endian)
    addr.sin_addr.s_addr = INADDR_ANY; //escucha en todas las interfaces (todas las IPs del servidor).
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero)); //limpia los bytes de relleno para evitar basura en la estructura.

    if (bind(_listeningSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) //enlaza _listeningSocket con la IP y puerto de addr
        throw(std::runtime_error("Failed to bind socket"));

    if (listen(_listeningSocket, SOMAXCONN) < 0) //Pone el socket en modo escucha para conexiones entrantes.
        throw(std::runtime_error("Listen failed"));

    //new node of the pollfd struct to add to the struct _fds.  en ella se configura como debe comportarse la funcion poll con el socket asignado (el de escucha)
    struct pollfd listenPollFd;
    listenPollFd.fd = _listeningSocket; //el socket que debe vigilar: el listening socket
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
    * Accepts a new incoming connection from the listening socket and prepares it to be handled by the server (accept)
    * Configura el socket que no bloquee el programa (fcntl)
    * Creates and configures a new node of the pollfd struct to add it to _fds, therefore _fds can monitor this new client (pollfd newClientPollFd)
    
    accept --> Extracts the first pending connection from the listening socket's queue and returns a new socket file descriptor connected to the client.
    fcntl --> Sets the newly accepted client socket to non-blocking mode so that read/write operations will not block the server loop.
*/
void Server::NewClient()
{
    //⚠️ memset(&cliadd, 0, sizeof(cliadd));   //VEEEEEEEEEEEEEEEEEEEEEEEEEEEER

    //al crear un cliente se necesita crear un nuevo elemento de la estructura sockaddr_in para indicar a dónde debe enviar datos.
    struct sockaddr_in _clientAddr; //Ver si no la uso de forma global, entonces la puedo definir aca
    socklen_t addrLen = sizeof(_clientAddr);
    int clientSocket = accept(_listeningSocket, (struct sockaddr*)&_clientAddr, &addrLen);
    if (clientSocket < 0)
        throw(std::runtime_error("Failed to accept a client"));

    if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) //poner el socket cliente en modo no bloqueante
        throw(std::runtime_error("Failed to set non-blocking mode on client socket"));

    //new node of the pollfd struct to add to the _fds 
    struct pollfd newClientPollFd;
    newClientPollFd.fd = clientSocket; //el socket que debe vigilar: clientSocket
    newClientPollFd.events = POLLIN; //eventos que te interesan: datos que envíe el cliente
    newClientPollFd.revents = 0; // eventos que ocurrieron: se inicializa a cero.

    _fds.push_back(newClientPollFd); //añades este fd a tu vector _fds

    Client newClient; //create the Client object   ⚠️⚠️⚠️⚠️⚠️ NO DEBERIA SER POINTER???
    newClient.set_fd(clientSocket);
	newClient.set_IPaddress(inet_ntoa((_clientAddr.sin_addr))); //inet_ntoa --> convierte la dirección IPv4 binaria (in_addr) en una cadena legible 

    _clients.push_back(newClient); //agrega objeto a vector _clients
    
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
    int bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0)
    {
        if (bytesReceived == 0)
            std::cout << YELLOW << "Client disconnected: fd " << clientFd << RESET << std::endl;
        else
            std::cerr << RED << "Error receiving data from fd " << clientFd << RESET << std::endl;
        
        ft_close(clientFd);  // --> removeClient   RemoveFds    close(fd)
        // RmChannels(fd);

        return;
    }
    buffer[bytesReceived] = '\0';
    
    //⚠️ else...........
}





/*
 *  Cuando queremos eliminar y cerrar todo, el Fd es -42 y la secuencia correcta es:
 *      - Cerrar cada file descriptor individualmente (con close(fd)) para liberar el recurso del sistema operativo.
 *      - Luego, hacer clear() en el vector que almacena esos fds, para eliminar todos los elementos del vector y dejarlo vacío.
 *  cuando se quiera eliminar un Fd en particular, el parametro sera algun valor > 0
*/
void Server::ft_close(int Fd)
{
    if (Fd < 0) // Cerrar y limpiar todo (agregar otras cosas como channels o buffers)
    {
        for(size_t i = 0; i < _clients.size(); i++)
        {
            std::cout << YELLOW << "Client <" << _clients[i].fd  << "> Disconnected" << RESET << std::endl; //ver si tengo que hacer un getter del fd para el client
        }

        for (size_t i = 0; i < _fds.size(); i++) //incluye el _listeningSocket 
        {
            close(_fds[i].fd);
        }

        _clients.clear(); //esto me ahorra de hacer un remove elemento por elemento
        _fds.clear(); //esto me ahorra de hacer un remove elemento por elemento
        _listeningSocket = -1;
    }

    else //remover 1 en particular
    {
        RemoveClient(Fd); //si nunca los voy a llamar por fuera de esta funcion, puedo quitarlols del header y agregarlos a utils
        RemoveFd(Fd);  //si nunca los voy a llamar por fuera de esta funcion, puedo quitarlols del header y agregarlos a utils
        close(Fd)
    }
}



void Server::RemoveClient(int clientFd)
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i].fd == clientFd)   //ver si tengo que hacer un getter del fd para el client
        {
            _clients.erase(_clients.begin() + i);
            break;
        }
    }
}

void Server::RemoveFd(int Fd)
{
    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].fd == Fd)
        {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }
}