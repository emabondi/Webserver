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
				std::cout<<"try_location: "<< it->first << std::endl;
				if (it->first.at(0) == '~' && it->first.size() >= 3 && it->first.at(1) == ' ' && it->first.find_first_not_of(' ', 1) != std::string::npos) {
					std::string end = it->first.substr(it->first.find_first_not_of(' ', 1));
					std::cout<<"end: "<< end<<std::endl;
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
		std::cout<<"ultima "<< request.substr(fine+4)<<std::endl;
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
	std::cout << methods[i] << std::endl;
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
	std::cout<< "location name:" << location._location_name << std::endl;
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
	std::cout<< "non va in errore\n";
	std::string b( (std::istreambuf_iterator<char>(body) ),
                       (std::istreambuf_iterator<char>()    ) );
    oss << "Content-Length: " << b.size() << "\r\n";
    // oss << "Connection: keep-alive\r\n";
    oss << "\r\n";
    oss << b;
	//oss << "\r\n\n\r"; //forse non va messo, sembra che il tester dia un errore
	std::string response(oss.str());
	//std::cout << response << std::endl;
	if (send(fd, response.c_str(), response.size(), MSG_NOSIGNAL) == -1)
		std::cout << "Send error!\n";
	std::cout<<"esco da handle get"<< std::endl;
}

void	Server::handlePUT(int fd, Config &location) {
	std::cout<<fd<<"handle PUT "<<location._location_name<<std::endl<<"LAST:"<<_requestMap["Last"]<<std::endl;
	if (_requestMap["Transfer-Encoding"] == "chunked")
		handlePUTChunked(fd, location);

}

void Server::handlePUTChunked(int fd, Config &location) {
	std::string line = _requestMap["Last"];
	if (line == "") {
		char buf[256];
		short nbytes = recv(fd, buf, 255, 0);
		buf[nbytes] = '\0';
		line += buf;
	}
	std::stringstream content;
	while (getChunk(line, fd) == true) {
		if (content.str().size() + line.size() > location.getClientMaxBodySize() && location.getClientMaxBodySize() != 0)
			return default_error_answer(413, fd, location);
		if (line.rfind("\r\n")){
			line.pop_back();
			line.pop_back();
		}
		content << line;
		line.clear();
		char buf[256];
		short nbytes = recv(fd, buf, 255, 0);
		if (nbytes == 0) {
			std::cout<<"recv mi da 0 esco"<<std::endl;
			return ;
		}
		buf[nbytes] = '\0';
		line += buf;
		std::cout<<"size "<<line.size() <<" line da mandare a getChunk"<<std::endl;
	}
		//std::cout << "size "<<chunkSize<<std::endl<< "content:"<<content.str()<<std::endl<<" content size "<<content.str().size()<<std::endl;
	std::string filepath(location.getRoot());
	line = _requestMap["URI"];
	filepath += line.substr(line.find(location._location_name) + location._location_name.size());
	if (checkTryFiles("$uri", location)) {
		std::ofstream file(filepath.c_str(), std::ios::out | std::ios::trunc);
		//std::cout<< "tentativo prima: " << filepath << std::endl;
		if (file.is_open()) {
			file << content.rdbuf();
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
				file << content.rdbuf();
				file.close();
				return default_error_answer(201, fd, location);
			}
			file.close();
		}
	}
	std::cout<<"error"<<std::endl;
	default_error_answer(404, fd, location);
}

bool Server::getChunk(std::string &line, int fd) {
	std::cout<<"ENTRO line:"<<line<<std::endl;
	std::size_t pos = line.find("\r\n");
	std::stringstream c(line.substr(0, pos));
	std::size_t chunkSize = 0;
	c >> std::hex >> chunkSize;
	if (chunkSize == 0) {
		std::cout<<"esco per size 0 "<<std::endl;
		return false;
	}
	std::cout<<"line before "<<line<<std::endl;
	line.erase(0, pos + 2);
	std::cout<<"line after  "<<line<<" chunk size "<<chunkSize<<std::endl;
	while (line.size() < chunkSize) {
		size_t bufsize = chunkSize - line.size() + 3;
		std::cout<<"bufsize "<<bufsize<< " chunkSize "<< chunkSize << " content size "<< line.size()<<std::endl;
		char buf[bufsize];
		short nbytes = recv(fd, buf, bufsize - 1, 0);
		buf[nbytes] = '\0';
		std::cout<<"line size before "<<line.size()<<std::endl;
		line += buf;
		std::cout<<"line size after "<<line.size()<<std::endl;
	}
	return true;
}

//short Server::getRequestContent(int fd, std::string line, std::stringstream &content, int maxBodySize) {
//	std::size_t pos = line.find("\r\n");
//	std::stringstream c(line.substr(0, pos));
//	std::size_t chunkSize = 0;
//	c >> std::hex >> chunkSize;
//	content << line.substr(pos + 2);
//
//	while (content.str().size() < chunkSize) {
//		size_t bufsize = chunkSize - content.str().size() + 3;
//		std::cout<<"bufsize "<<bufsize<< " chunkSize "<< chunkSize << " content size "<< content.str().size()<<std::endl;
//		char buf[bufsize];
//		short nbytes = recv(fd, buf, bufsize - 1, 0);
//		buf[nbytes] = '\0';
//		content << buf;
//			while (chunkSize > content.str().size() ) {
//				size_t bsize = chunkSize - content.str().size() + 3;
//				std::cout<<"bsize "<<bsize<< " chunkSize "<< chunkSize << " content size "<< content.str().size()<<std::endl;
//				char b[bsize];
//				short n = recv(fd, b, bsize - 1, 0);
//				b[n] = '\0';
//				content << b;
//			}
//		std::cout<<"received size: "<<content.str().size()<<std::endl;
//			/*std::string temp(content.str().substr(content.str().size() - 3));*/
//		std::string temp(buf);
//		/*if (temp.find("\r\n") == std::string::npos)
//			break;
//		std::cout<<"ha trovato \\r\\n "<<std::endl;
//		temp.clear();*/
//		while (temp.find("\r\n") == std::string::npos) {
//			unsigned char c;
//			if (recv(fd, &c, sizeof(unsigned char), 0) <= 0){
//				std::cout<<"recv non ha niente"<<std::endl;
//				return 204;
//			}
//			temp += c;
//			std::cout<<"new hex chunk "<< temp<<std::endl;
//		}
//		temp.pop_back();
//		temp.pop_back();
//		std::cout<<"new hex chunk popped "<< temp<<std::endl;
//		std::stringstream converter(temp);
//		std::size_t newChunk;
//		converter >> std::hex >> newChunk;
//		std::cout<<"new chunk converted "<< newChunk<<std::endl;
//		if (newChunk == 0)
//			break ;
//		chunkSize += newChunk;
//		if (chunkSize > maxBodySize && maxBodySize != 0)
//			return 413;
//		default_error_answer(100, fd, _config);
//	}
//	return 0;
//}

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
	std::cout<<"check try files $uri "<< this->checkTryFiles("$uri", location)<<std::endl;
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
		std::cout<<"resource path:"<<resource_path<<std::endl;
		sVec	indexes = location.getIndex();
		for (sVec::iterator it = indexes.begin(); it != indexes.end(); it++)
		{
			std::cout<< "tentativo: " << resource_path + *it << std::endl;
			body.open((resource_path + *it).c_str());
			if (body.is_open() == true) {
				_requestMap.insert(std::make_pair("URI_TRANSLATED", resource_path + *it));
				return 0;}
			body.close();
		}
	}
	std::cout<<"final return"<< std::endl;
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

bool	Server::checkTryFiles(std::string check, Config &location) {
	sVec try_files = location.getTryFiles();
	std::cout<<"check:"<<check<<"size"<<try_files.size()<<std::endl;
	for (std::vector<std::string>::iterator it = try_files.begin(); it != try_files.end(); it++) {
		std::cout<<"it:"<<(*it)<<std::endl;
		if ((*it) == check)
			return true;
	}
	std::cout<<"esco:"<<check<<std::endl;
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