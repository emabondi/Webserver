#include "../includes/Config.hpp"

uint16_t	Config::getListen(void) const {
	return _listen;
}

sVec		Config::getServerName(void) const {
	return _server_name;
}

std::string	Config::getRoot(void) const {
	return _root;
}

sVec		Config::getIndex(void) const {
	return _index;
}

iSMap		Config::getErrorPage(void) const {
	return _error_page;
}

std::string		Config::getErrorPage(int err) {
	return _error_page[err];
}

int			Config::getClientMaxBodySize(void) const {
	return _client_max_body_size;
}

sBMap		Config::getAllowedMethods(void) const {
	return _allowed_methods;
}

bool		Config::getAutoindex(void) const {
	return _autoindex;
}

sVec		Config::getTryFiles(void) const {
	return _try_files;
}

std::string	Config::getCgiPass(void) const {
	return _cgi_pass;
}

sVec		Config::getExtensionCgi(void) const {
	return _extension_cgi;
}

iSPair		Config::getReturn(void) const {
	return _return;
}

std::map<std::string, Config>	Config::getLocationMap(void) const {
	return _locationMap;
}