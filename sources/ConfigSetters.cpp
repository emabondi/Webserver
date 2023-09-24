#include "../includes/Config.hpp"

void	Config::setListen(std::string &configStr) {
	size_t		pos;
	size_t		end;
	int			myInt;
	std::string	line;

	pos = configStr.find("listen");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" ", pos + 6);
	if (pos >= end)
		throw badConfigFile();
	line = configStr.substr(pos, end - pos);
	if(line.find_first_not_of("1234567890") != std::string::npos)
		throw badConfigFile();
	std::istringstream tmp(line.substr(0,10));
	if(!(tmp >> myInt))
		throw badConfigFile();
	if (myInt > static_cast<int>(UINT16_MAX) || myInt <= 0)
    	throw badConfigFile();
	_listen = static_cast<uint16_t>(myInt);
}

void	Config::setServerName(std::string &configStr) {
	size_t	pos;
	size_t	end;
	std::string	line;

	pos = configStr.find("server_name");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 11);
	if (pos >= end)
		throw badConfigFile();
	line = configStr.substr(pos, end - pos);
	for (pos = 0; pos != std::string::npos; pos = line.find_first_not_of(" \t", end))
	{
		end = line.find_first_of(" \t", pos);
		_server_name.push_back(line.substr(pos, end - pos));
	}
}

void	Config::setRoot(std::string &configStr) {
	size_t	pos;
	size_t	end;

	pos = configStr.find("root");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 4);
	if (pos >= end)
		throw badConfigFile();
	_root = configStr.substr(pos, end - pos);
	if (_root.find_first_of(" \t") != std::string::npos)
		throw badConfigFile();
}

void	Config::setIndex(std::string &configStr) {
	size_t	pos;
	size_t	end;
	std::string	line;

	pos = configStr.find("index");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 5);
	if (pos >= end)
		throw badConfigFile();
	line = configStr.substr(pos, end - pos);
	for (pos = 0; pos != std::string::npos; pos = line.find_first_not_of(" \t", end))
	{
		end = line.find_first_of(" \t", pos);
		_index.push_back(line.substr(pos, end - pos));
	}
}

void	Config::setErrorPage(std::string &configStr) {
	size_t	pos;
	size_t	end;
	int		error_code;

	pos = configStr.find("error_page");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 10);
	if (pos >= end)
		throw badConfigFile();
	configStr = configStr.substr(pos, end - pos);
	end = configStr.find_first_of(" \t");
	try {
		std::stringstream ss(configStr.substr(0, end));
		ss >> error_code;
	}
	catch(std::exception &e) {
		throw badConfigFile();
	}
	end = configStr.find_first_not_of(" \t", end);
	configStr.erase(0, end);
	_error_page.insert(std::make_pair(error_code, configStr));
}

void	Config::setClientMaxBodySize(std::string &configStr) {
	size_t	pos;
	size_t	end;
	std::string	line;

	pos = configStr.find("client_max_body_size");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 20);
	if (pos >= end)
		throw badConfigFile();
	line = configStr.substr(pos, end - pos);
	pos = line.find_first_of("Kk");
	if (pos != std::string::npos)
		line.replace(pos, 1, 3, '0');
	if (line.find_first_not_of("0123456789Kk") != std::string::npos)
		throw badConfigFile();
	std::istringstream tmp(line.substr(0,10));
	if(!(tmp >> _client_max_body_size))
		throw badConfigFile();
	if (_client_max_body_size < 0)
		throw badConfigFile();
}

void	Config::setAllowedMethods(std::string &configStr) {
	size_t		pos;
	size_t		end;
	std::string	line;

	_allowed_methods.clear();
	_allowed_methods.insert(std::pair<std::string, bool>("GET", false));
	_allowed_methods.insert(std::pair<std::string, bool>("POST", false));
	_allowed_methods.insert(std::pair<std::string, bool>("HEAD", false));
	_allowed_methods.insert(std::pair<std::string, bool>("PUT", false));
	_allowed_methods.insert(std::pair<std::string, bool>("DELETE", false));
	pos = configStr.find("allowed_methods");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 15);
	if (pos >= end)
		throw badConfigFile();
	line = configStr.substr(pos, end - pos);
	pos = 0;
	while (end != std::string::npos)
	{
		end = line.find_first_of(" \t", pos);
		if (_allowed_methods.find(line.substr(pos, end)) != _allowed_methods.end())
			_allowed_methods[line.substr(pos, end)] = true;
		else
			throw badConfigFile();
		line.erase(pos, end + 1);
	}
}

void	Config::setAutoindex(std::string &configStr) {
	size_t		pos;
	size_t		end;
	std::string	line;
	std::string	mode[2] = {"on", "off"};
	int			i;

	pos = configStr.find("autoindex");
	end = configStr.find(";", pos);
	if (end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 9);
	if (pos >= end)
		throw badConfigFile();
	line = configStr.substr(pos, end - pos);
	for (i = 0; i < 2; i++)
		if (line == mode[i])
			break ;
	switch(i) {
		case 0 :
			_autoindex = true;
			break ;
		case 1 :
			_autoindex = false;
			break ;
		default :
			throw badConfigFile();
	};
}

void	Config::setTryFiles(std::string &configStr) {
	size_t	pos;
	size_t	end;
	std::string	line;

	pos = configStr.find("try_files");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 9);
	if (pos >= end)
		throw badConfigFile();
	line = configStr.substr(pos, end - pos);
	for (pos = 0; pos != std::string::npos; pos = line.find_first_not_of(" \t", end))
	{
		end = line.find_first_of(" \t", pos);
		_try_files.push_back(line.substr(pos, end - pos));
	}
}

void	Config::setCgiPass(std::string &configStr) {
	size_t	pos;
	size_t	end;

	pos = configStr.find("cgi_pass");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 8);
	if (pos >= end)
		throw badConfigFile();
	_cgi_pass = configStr.substr(pos, end - pos);
	if (_cgi_pass.find_first_of(" \t") != std::string::npos)
		throw badConfigFile();
}

void	Config::setExtensionCgi(std::string &configStr) {
	size_t	pos;
	size_t	end;
	std::string	line;

	pos = configStr.find("extension_cgi");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 13);
	if (pos >= end)
		throw badConfigFile();
	line = configStr.substr(pos, end - pos);
	for (pos = 0; pos != std::string::npos; pos = line.find_first_not_of(" \t", end))
	{
		end = line.find_first_of(" \t", pos);
		_extension_cgi.push_back(line.substr(pos, end - pos));
	}
}

void	Config::setReturn(std::string &configStr) {
	size_t		pos;
	size_t		end;
	std::string	line;
	int			status_code;
	std::string	url;

	pos = configStr.find("return");
	end = configStr.find(";", pos);
	if (pos == std::string::npos || end == std::string::npos || configStr.at(end + 1) != '\n')
		throw badConfigFile();
	pos = configStr.find_first_not_of(" \t", pos + 6);
	if (pos >= end)
		throw badConfigFile();
	line = configStr.substr(pos, end - pos);
	end = line.find_first_of(" \t");
	if (line.substr(0, end).find_first_not_of("0123456789") != std::string::npos)
		throw badConfigFile();
	std::istringstream tmp(line.substr(0,end));
	if(!(tmp >> status_code))
		throw badConfigFile();
	line.erase(0, end);
	pos = line.find_first_not_of(" \t\n");
	if (pos == std::string::npos)
		_return = std::make_pair(status_code, url);
	else
	{
		end = line.find_first_of(" \t\n", pos);
		url = line.substr(pos, end);
		_return = (std::make_pair(status_code, url));
	}
}
