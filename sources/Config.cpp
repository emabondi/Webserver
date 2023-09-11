#include "../includes/Config.hpp"

Config::Config()
	: _listen(8080), _client_max_body_size(0),
		_autoindex(false) {

	_allowed_methods.insert(std::pair<std::string, bool>("GET", false));
	_allowed_methods.insert(std::pair<std::string, bool>("POST", false));
	_allowed_methods.insert(std::pair<std::string, bool>("HEAD", false));
	_allowed_methods.insert(std::pair<std::string, bool>("PUT", false));
	_allowed_methods.insert(std::pair<std::string, bool>("DELETE", false));
}

Config::Config(std::string &serverBody)
	: _listen(8080), _client_max_body_size(0) {
	std::string	line;
	std::string	directive;
	size_t		pos;
	std::string	locationName;

	_allowed_methods.insert(std::pair<std::string, bool>("GET", true));
	_allowed_methods.insert(std::pair<std::string, bool>("POST", false));
	_allowed_methods.insert(std::pair<std::string, bool>("HEAD", true));
	_allowed_methods.insert(std::pair<std::string, bool>("PUT", false));
	_allowed_methods.insert(std::pair<std::string, bool>("DELETE", false));

	while ((pos = serverBody.find_first_of('\n')) != std::string::npos)
	{
		line = serverBody.substr(serverBody.find_first_not_of(" \t\n"), pos);
		if (this->doDirective(line))
			this->addLocation(serverBody);
		else
			serverBody.erase(0, pos + 1);
	}
}

Config::Config(std::string &locationBody, Config &mainConfig) {
	*this = mainConfig;

	std::string	line;
	std::string	directive;
	size_t		pos;
	std::string	locationName;

	while ((pos = locationBody.find_first_of('\n')) != std::string::npos)
	{
		line = locationBody.substr(locationBody.find_first_not_of(" \t\n"), pos);
		this->doDirective(line);
		locationBody.erase(0, pos + 1);
	}
}

Config::~Config(void) {
	
}

Config	&Config::operator=(Config &rhs) {
	if (this == &rhs)
		return *this;
	
	this->_listen = rhs.getListen();
	this->_server_name = rhs.getServerName();
	this->_root = rhs.getRoot();
	this->_index = rhs.getIndex();
	this->_error_page = rhs.getErrorPage();
	this->_client_max_body_size = rhs.getClientMaxBodySize();
	this->_allowed_methods = rhs.getAllowedMethods();
	this->_autoindex = rhs.getAutoindex();
	this->_try_files = rhs.getTryFiles();
	this->_cgi_pass = rhs.getCgiPass();
	this->_extension_cgi = rhs.getExtensionCgi();
	this->_return = rhs.getReturn();
	this->_locationMap = rhs.getLocationMap();

	return *this;
}

int	Config::doDirective(std::string &line) {
	std::string		directive;
	int				i;
	std::string		directiveAr[] = {"listen", "server_name", "root", "index", "error_page", 
		"client_max_body_size", "allowed_methods", "location", "autoindex", "try_files",
			"cgi_pass", "extension_cgi", "return"};

	directive = line.substr(0, line.find_first_of(" \t\n"));
	i = 0;
	while (i < 13)
	{
		if (directive == directiveAr[i])
			break ;
		i++;
	}
	switch(i)
	{
		case LISTEN :
			this->setListen(line);
			break ;
		case SERVER_NAME :
			this->setServerName(line);
			break ;
		case ROOT :
			this->setRoot(line);
			break ;
		case INDEX :
			this->setIndex(line);
			break ;
		case ERROR_PAGE :
			this->setErrorPage(line);
			break ;
		case CLIENT_MAX_BODY_SIZE :
			this->setClientMaxBodySize(line);
			break ;
		case ALLOWED_METHODS :
			this->setAllowedMethods(line);
			break ;
		case LOCATION :
			return 1;
		case AUTOINDEX :
			this->setAutoindex(line);
			break ;
		case TRY_FILES :
			this->setTryFiles(line);
			break ;
		case CGI_PASS :
			this->setCgiPass(line);
			break ;
		case EXTENSION_CGI :
			this->setExtensionCgi(line);
			break ;
		case RETURN :
			this->setReturn(line);
			break ;
		default :
			if (directive.find_first_not_of("{}\n") != std::string::npos)
				throw badConfigFile();
	}
	return 0;
}

void	Config::addLocation(std::string &serverBody) {
	size_t		pos;
	size_t		end;
	std::string	locationName;
	std::string	locationBody;

	pos = serverBody.find("location");
	serverBody.erase(0, pos + 8);
	pos = serverBody.find_first_not_of(" \t\n");
	end = serverBody.find("{");
	if (pos == end || pos == std::string::npos || end == std::string::npos)
		throw badConfigFile();
	end -= 1;
	while (std::isspace(serverBody.at(end)))
		end--;
	locationName = serverBody.substr(pos, end);
	pos = end + 1;
	end = serverBody.find_first_of("}");
	locationBody = serverBody.substr(pos, end);
	Config locationConfig(locationBody, *this);
	_locationMap.insert(std::pair<std::string, Config>(locationName, locationConfig));
	serverBody.erase(0, end);
}

void	Config::displayConfig(void) const {
	sVec	vec;
	std::map<std::string, bool> map;
	iSPair	pair;
	iSMap	errorMap;

	std::cout << "listen: " << this->getListen() << std::endl;
	std::cout << "server_name: ";
	vec = this->getServerName();
	for(sVec::iterator it = vec.begin(); it != vec.end(); it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	std::cout << "root: " << this->getRoot() << std::endl;
	std::cout << "index: ";
	vec = this->getIndex();
	for(sVec::iterator it = vec.begin(); it != vec.end(); it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	std::cout << "error_page: ";
	errorMap = this->getErrorPage();
	for(iSMap::iterator it = errorMap.begin(); it != errorMap.end(); it++)
		std::cout << it->first << "=" << it->second << ", ";
	std::cout << std::endl;
	std::cout << "client_max_body_size: " << this->getClientMaxBodySize() << std::endl;
	std::cout << "allowed_methods: ";
	map = getAllowedMethods();
	std::cout << std::boolalpha;
	for(std::map<std::string, bool>::iterator it = map.begin(); it != map.end(); it++)
		std::cout << it->first << '=' << it->second << ", ";
	std::cout << std::endl;
	std::cout << "autoindex: " << this->getAutoindex() << std::endl;
	std::cout << "try_files: ";
	vec = this->getTryFiles();
	for(sVec::iterator it = vec.begin(); it != vec.end(); it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	std::cout << "cgi_pass: " << this->getCgiPass() << std::endl;
	std::cout << "extension_cgi: ";
	vec = this->getExtensionCgi();
	for(sVec::iterator it = vec.begin(); it != vec.end(); it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	pair = this->getReturn();
	std::cout << "return: " << pair.first << ' ' << pair.second << std::endl;
	std::cout << std::endl;
}

const char *Config::badConfigFile::what() const throw() {
	return "Bad config file";
}
