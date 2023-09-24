#include "../includes/Server.hpp"

Server::Server(Config &config, sCMap &locationMap) : _config(config), _locationMap(locationMap) {
	struct sockaddr_in	addr;
	int					listener;

	listener = socket(AF_INET, SOCK_STREAM, 0);
	int flags = fcntl(listener, F_GETFL, 0);
	fcntl(listener, F_SETFL, flags | O_NONBLOCK); //questo é il non bloccante
	if (listener < 0)
		throw std::runtime_error("Unable to create socket");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(config.getListen()); //probabilmente da cambiare
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int yes = 1;
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		throw std::runtime_error("Unable to reuse port");
	if (bind(listener,(struct sockaddr *)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Unable to bind socket");
	if (listen(listener, 1000) < 0)
		throw std::runtime_error("Listen error");
	_pfds.addToPfds(listener);
	std::cout << "server = " << listener << std::endl;
}

Server::~Server(void) {

}

void	Server::startListening(void) {
	int 			poll_count;
	struct pollfd	*socketArr;

	socketArr = _pfds.getSocketArr();
	poll_count = poll(socketArr, _pfds.getCount(), 0);
	if (poll_count == -1)
		throw std::runtime_error("Poll error");
	if (poll_count == 0)
		return ;
	std::cout << socketArr[0].fd << " poll count " << poll_count << "\n";
	for(size_t i = 0; i < _pfds.getCount(); i++)
	{
		if (socketArr[i].revents & POLLIN)
		{
			if (i == 0){
				std::cout << "(handle server) nuovo socket!\n";
				this->handleServer();
				}
			else
				this->handleClient(i);
		}
	};
}


void	Server::handleServer(void) {
	int 					new_fd;
	struct sockaddr_storage client_addr;
	socklen_t				sin_size;
	struct pollfd			*socketArr;

	sin_size = sizeof(client_addr);
	socketArr = _pfds.getSocketArr();
	new_fd = accept(socketArr[0].fd, (struct sockaddr *)&client_addr, &sin_size);
	if (new_fd == -1)
		throw std::runtime_error("Accept error");
	_pfds.addToPfds(new_fd);
	std::cout << "new_fd:" << new_fd << std::endl;
}

void	Server::handleClient(int i) {
	unsigned char	buf;
	int				nbytes;
	std::string		request;
	struct pollfd	*socketArr;

	nbytes = 0;
	socketArr = _pfds.getSocketArr();
	while (1)
	{
		nbytes = recv(socketArr[i].fd, &buf, sizeof(unsigned char), 0);
		if (nbytes < 0)
		{
			std::cout << "Recv error\n";
			close(socketArr[i].fd);
			_pfds.delFromPfds(i);
			std::cout << "Connection from " << socketArr[i].fd << " closed.\n";
			break ;
		}
		if (nbytes == 0)
			break ;
		request += buf;
		if (request.find("\r\n\r\n") != std::string::npos)
			break ;
	}
	std::cout<<request<<std::endl;
	this->parseRequest(request);
	//if (_requestMap["URI"] != "/favicon.ico")
	//	std::cout << request << std::endl;
	Config location;
	if (this->checkRequest(socketArr[i].fd, location)) {
/*			std::cout<<"INIZIO "<<location._location_name<<std::endl;
			location.displayConfig();
			std::cout<<"FINE"<<std::endl;*/
		handleRequest(socketArr[i].fd, location);
	};
	_requestMap.clear();
	close(socketArr[i].fd);
	_pfds.delFromPfds(i);
}

void	Server::displayServerConfig(void) {
	std::cout << "Server" << std::endl;
	_config.displayConfig();
	for (sCMap::iterator it = _locationMap.begin(); it != _locationMap.end(); it++)
	{
		std::cout << "Name: " << it->first << std::endl;
		it->second.displayConfig();
	}
}

void	Server::default_error_answer(int err, int fd, Config &location) {
	std::ifstream file;
	if (err > 400) {
		std::string errpage = _config.getErrorPage(err);
		std::cout<<"getted err page "<< errpage << std::endl;
		if (errpage.size() && err != 500) {
			//std::stringstream temp;
			//temp << err;
			//std::string error = temp.str() + ".html";
			std::string	root = _config.getRoot();
			if (root.at(root.size() - 1) != '/')
				root.push_back('/');
			//for (iSMap::iterator it = errpages.begin(); it != errpages.end(); it++) {
			//	if ((*it) == error) {
					file.open((root + errpage).c_str());
					if (!file.is_open()) {
						file.close();
						//return default_error_answer(500, fd, location);
					}

		//		}
		//	}
		}
	}
	std::string tmpString;
		switch (err)
	{
		case 100: tmpString = "100 Continue"; break ;
		case 200: tmpString = "200 OK"; break ;
		case 201: tmpString = "201 Created"; break ;
		case 202: tmpString = "202 Accepted"; break ;
		case 203: tmpString = "203 Non-Authoritative Information"; break ;
		case 204: tmpString = "204 No content"; break ;
		case 205: tmpString = "205 Reset Content"; break ;
		case 206: tmpString = "206 Partial Content"; break ;
		case 400: tmpString = "400 Bad Request"; break ;
		case 401: tmpString = "401 Unauthorized"; break ;
		case 402: tmpString = "402 Payment Required"; break ;
		case 403: tmpString = "403 Forbidden"; break ;
		case 404: tmpString = "404 Not Found"; break ;
		case 405: tmpString = "405 Method Not Allowed"; break ;
		case 406: tmpString = "406 Not Acceptable"; break ;
		case 411: tmpString = "411 Length Required"; break ;
		case 413: tmpString = "413 Request Entity Too Large"; break ;
		case 500: tmpString = "500 Internal Server Error"; break ;
		case 501: tmpString = "501 Not Implemented"; break ;
		case 510: tmpString = "510 Not Extended"; break ;
		default: break ;
	}
	std::stringstream convert;
	std::string res = "HTTP/1.1 " + tmpString + "\r\n";
	if (err == 405) {
		res += "Allow: ";
		sBMap allowed = location.getAllowedMethods();
		for (sBMap::iterator it = allowed.begin(); it != allowed.end(); it++) {
			if (it->second == true)
				res += it->first + ", ";
		}
		res.resize(res.size() - 2);
		res += "\n\r";
	}
	if (err != 204)
		res += "Server: webserv1.0\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: ";
	else
		res += "\r\n";
	if (file.is_open()) {
		convert << file.rdbuf();
		file.close();
		std::string body = convert.str();
		convert.str(std::string());
		convert << body.size();
		res.append(convert.str() + "\r\n\r\n" + body);
	}
	else if (err != 100 && err != 204) {
		std::string tmpBody = "<html><head><title>" + tmpString + "</title></head><body><p>" + tmpString + "</p></body></html>";
		//std::string res = "HTTP/1.1 " + tmpString + "\r\nAllow: POST\r\nServer: webserv1.0\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 133";
		convert << tmpBody.length();
		res.append(convert.str() + "\r\n\r\n" + tmpBody);
	}
	std::cout<< res << std::endl;
	if (send(fd, res.c_str(), res.size(), 0) == -1)
		std::cout << "Send error!\n";
}