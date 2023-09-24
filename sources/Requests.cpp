#include "../includes/Server.hpp"

bool	Server::checkRequest(int fd, Config &location) {
	location = _locationMap["/"];
	location._location_name = "/";
	std::string temp_request = _requestMap["URI"];
	if (_locationMap.count(temp_request) == 0) {
		for (sCMap::iterator it = _locationMap.begin(); it != _locationMap.end(); it++) {
			if (it->first == "/")
				continue ;
			if (temp_request.find(it->first.c_str(), 0, it->first.length()) == 0) {
				location = _locationMap[it->first];
				location._location_name = it->first;
			}
		}
		if (location._location_name == "/") {
			for (sCMap::iterator it = _locationMap.begin(); it != _locationMap.end(); it++) {
				if (it->first.at(0) == '~' && it->first.size() >= 3 && it->first.at(1) == ' ' && it->first.find_first_not_of(' ', 1) != std::string::npos) {
					std::string end = it->first.substr(it->first.find_first_not_of(' ', 1));
					if (this->checkExtensionCgi(end, location) && temp_request.size() >= end.size() && temp_request.substr(temp_request.size() - end.size() - 1) == end) {
						location = _locationMap[it->first];
						location._location_name = it->first;
						break ;
					}
				}
			}
		}
	}
		/*size_t pos = temp_request.size();
		while ((pos = temp_request.rfind('/', pos)) != std::string::npos){
			temp_request.resize(pos);
			if (_locationMap.count(temp_request) > 0) {
				location = _locationMap[temp_request];
				location._location_name = temp_request;
				break ;
			}
		}
	}*/
	else {
		location = _locationMap[temp_request];
		location._location_name = temp_request;
	}
	sBMap alllowed_methods = location.getAllowedMethods();
	if (alllowed_methods[_requestMap["HTTP_method"]] == false) {
		std::cout<< "esco qua"<<std::endl;
		default_error_answer(405, fd, location);
		return false;
	}
	return true;
}

void	Server::parseRequest(std::string request) {
	std::size_t first = 0;
	std::size_t i = 0;

	const char *prova = request.c_str();
	std::string line;

	while (prova[i] != '\0') {
		if ((prova[i] == '\r' && prova[i + 1] == '\n') || prova[i] == 4){
			line = request.substr(first, i - first);
			std::size_t space = line.find(' ', 0);
			std::size_t space2 = line.find(' ', space + 1);
			_requestMap.insert(std::make_pair("HTTP_method", line.substr(0, space)));
			_requestMap.insert(std::make_pair("URI", line.substr(space + 1, space2 - space - 1)));
			_requestMap.insert(std::make_pair("protocol_version", line.substr(space2 + 1, line.length())));
			if (prova[i] != 4)
				i++;
			first = i + 1;
			break ;
		}
		i++;
	}
	size_t fine = request.rfind("\r\n\r\n");
	if (fine != std::string::npos) {
		_requestMap.insert(std::make_pair("Last", request.substr(fine+4)));
	}
	while (prova[i] != '\0') {
		if ((prova[i] == '\r' && prova[i + 1] == '\n') || prova[i] == 4){
			
			line = request.substr(first, i - first);
			if (prova[i] != 4)
				i++;
			first = i + 1;
		std::size_t mid = line.find(':', 0);
		if (mid != std::string::npos && line[mid] == ':' && line[mid + 1] == ' ')
			_requestMap.insert(std::make_pair(line.substr(0, mid), line.substr(mid + 2 , line.length())));
		else
			_requestMap.insert(std::make_pair(line.substr(0, line.length()), ""));
		}
		i++;
	}
}

void Server::handleRequest(int fd, Config &location) {
	std::string	methods[5] = {"GET", "POST", "DELETE", "HEAD", "PUT"};
	int	i;

	for (i = 0; i < 5; i++)
		if (methods[i] == _requestMap["HTTP_method"])
			break ;
	switch(i)
	{
		case GET:
			this->handleGET(fd, location);
			break ;
		case PUT:
			this->handlePUT(fd, location);
			break ;
		case POST:
			this->handlePOST(fd, location);
			break ;
		case DELETE:
			this->handleDELETE(fd);
			break;
		default :	
			return this->default_error_answer(405, fd, location);
	}
}

void	Server::handleDELETE(int fd) {
	std::string uri = _requestMap["URI"];
	Config		toDeleteLocation = _locationMap[uri];

	(void)fd;
}

void	Server::handleGET(int fd, Config &location) {
	std::ifstream body;
	int status;
	std::ostringstream oss;

	oss << "HTTP/1.1 200 OK\r\n";
	std::cout<< "handleGET "<< _requestMap["URI"] << std::endl;
	if (_requestMap["URI"] == "/favicon.ico") {
		if(!getIcon(body))
			return default_error_answer(404, fd, location);
		oss << "Content-Type: image/png\r\n";
	}
	else { 
		if ((status = getBody(body, location)) != 0)
			return default_error_answer(status, fd, location);
    	oss << "Content-Type: text/html\r\n";
	}
	std::string b( (std::istreambuf_iterator<char>(body) ),
                       (std::istreambuf_iterator<char>()    ) );
    oss << "Content-Length: " << b.size() << "\r\n";
    // oss << "Connection: keep-alive\r\n";
    oss << "\r\n";
    oss << b;
	//oss << "\r\n\n\r"; //forse non va messo, sembra che il tester dia un errore
	std::string response(oss.str());
	if (send(fd, response.c_str(), response.size(), MSG_NOSIGNAL) == -1)
		std::cout << "Send error!\n";
}

void	Server::handlePUT(int fd, Config &location) {
	std::cout<<fd<<"handle PUT "<<location._location_name<<std::endl<<"LAST:"<<_requestMap["Last"]<<std::endl;
	if (_requestMap["Transfer-Encoding"] == "chunked")
		handlePUTChunked(fd, location);

}

void Server::handlePUTChunked(int fd, Config &location) {
	std::string body = getChunks(fd, location.getClientMaxBodySize());
	if (body == "413")
		return default_error_answer(413, fd, location);
		//std::cout << "size "<<chunkSize<<std::endl<< "body:"<<body<<std::endl<<" body size "<<body.size()<<std::endl;
	std::string filepath(location.getRoot());
	std::string line = _requestMap["URI"];
	filepath += line.substr(line.find(location._location_name) + location._location_name.size());
	if (checkTryFiles("$uri", location)) {
		std::ofstream file(filepath.c_str(), std::ios::out | std::ios::trunc);
		//std::cout<< "tentativo prima: " << filepath << std::endl;
		if (file.is_open()) {
			file << body;
			file.close();
			return default_error_answer(201, fd, location);
		}
		file.close();
	}
	if (checkTryFiles("$uri/", location)){
		if (filepath.at(filepath.size() - 1) != '/')
			filepath.push_back('/');
		sVec indexes = location.getIndex();
		for (sVec::iterator it = indexes.begin(); it != indexes.end(); it++)
		{
			std::ofstream file(filepath.c_str(), std::ios::out | std::ios::trunc) ;
			//std::cout<< "tentativo: " << filepath + *it << " it:"<<*it<< std::endl;
			file.open((filepath + *it).c_str());
			if (file.is_open() == true){
				file << body;
				file.close();
				return default_error_answer(201, fd, location);
			}
			file.close();
		}
	}
	std::cout<<"error"<<std::endl;
	default_error_answer(404, fd, location);
}

std::string Server::getChunks(int fd, size_t max_body_size){
	char c;
	size_t size;
	std::string buf;
	std::string body;

	std::cout<<"wait..."<<std::endl;
	std::cout<< "max body:"<<max_body_size<<std::endl;
	while(body.size() < max_body_size || max_body_size == 0)
	{
		while (buf.find("\r\n") == std::string::npos)
		{
			if (recv(fd, &c, 1, 0) == 0) //condizione un po' a caso
				return body;
			buf += c;

		}
		std::stringstream conv(buf.substr(0, buf.find("\r\n")));
		size = 0;
		conv >> std::hex >> size;
		if(size == 0)
			break ;
		buf.clear();
		while (buf.size() < size + 2)
		{
			size_t s = size + 3 - buf.size();
			char buff[s];
			// std::cout<<"i:"<<i<<std::endl;
			//if (recv(fd, &c, 1, 0) == 0) //condizione un po' a caso
			//	return body;
			size_t nbytes = recv(fd, buff, s - 1, 0);
			buff[nbytes] = '\0';
			buf += buff;
			//std::cout<<"buf2 size:"<<buf.size()<<std::endl;
		}
		buf.erase(buf.size() - 2);
		//std::cout<<"buf2 size:"<<buf.size()<<std::endl;
		body += buf;
		//std::cout<<"size body:"<<body.size()<<std::endl;
		buf.clear();
		if (max_body_size != 0 && body.size() > max_body_size)
			return ("413");
	}
	std::cout<<"vado dopo pene "<<body.size()<<std::endl;
	return body;
}

int Server::getBody(std::ifstream &body, Config &location) {
	std::string resource_path = _requestMap["URI"];
	std::string root = location.getRoot();

	/*if (root.at(root.size() - 1) != '/')
		root += "/";*/
	if (location._location_name == "/") {
		resource_path = root;
	}
	else {
		std::cout<<"resource_path prima:"<<resource_path<<std::endl;
		size_t pos = resource_path.find(location._location_name);

		resource_path.replace(pos, location._location_name.size(), root);
		pos = resource_path.find("//");
		std::cout << "find pos:"<< pos<<" resource.size:"<< resource_path.size()<<std::endl;
		if (pos != std::string::npos)
			resource_path.erase(pos, 1);
		std::cout<<"resource_path at size -1:"<<resource_path.at(resource_path.size() - 1) <<std::endl;
	}
	//std::cout<<"check try files $uri "<< this->checkTryFiles("$uri", location)<<std::endl;
	if (!isDirectory(resource_path) && this->checkTryFiles("$uri", location)) {
		body.open(resource_path.c_str());
		if (body.is_open() == true){std::cout<<"no dir open file"<<std::endl;
		_requestMap.insert(std::make_pair("URI_TRANSLATED", resource_path));
				return 0;}
		else
			body.close();
	}
	if (this->checkTryFiles("$uri/", location)) {
		if (resource_path.at(resource_path.size() - 1) != '/')
			resource_path.push_back('/');
		sVec	indexes = location.getIndex();
		for (sVec::iterator it = indexes.begin(); it != indexes.end(); it++)
		{
			body.open((resource_path + *it).c_str());
			if (body.is_open() == true) {
				_requestMap.insert(std::make_pair("URI_TRANSLATED", resource_path + *it));
				return 0;}
			body.close();
		}
	}
	return 404;
}

bool Server::getIcon(std::ifstream &body) {
	body.open("fake_site/choco.png");
	if (body.is_open())
		return true;
	body.close();
	return false;
}

bool isDirectory(const std::string& path) {
	struct stat fileStat;
	if (stat(path.c_str(), &fileStat) == 0)
		return S_ISDIR(fileStat.st_mode);
	return false;
}

std::string toString(int num) {
	std::stringstream ss;
	ss << num;
	return ss.str();
}

bool	Server::checkTryFiles(std::string check, Config &location) {
	sVec try_files = location.getTryFiles();
	for (std::vector<std::string>::iterator it = try_files.begin(); it != try_files.end(); it++) {
		if ((*it) == check)
			return true;
	}
	return false;
}

bool Server::checkExtensionCgi(std::string end, Config &location) {
	sVec exts_cgi = location.getExtensionCgi();
	std::cout<<"entro check extension cgi, size vector: "<< exts_cgi.size() <<std::endl;
	for (sVec::iterator it = exts_cgi.begin(); it != exts_cgi.end(); it++) {
		std::cout<<"ext: "<< *it << "end: " << end<<std::endl;
		if (*it == end)
			return true;
	}
	std::cout<<"ritorno false"<<std::endl;
	return false;
}