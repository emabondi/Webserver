#include "../includes/Server.hpp"

void	Server::handlePOST(int fd, Config &location) {
	if (_requestMap["Transfer-Encoding"] == "chunked") {
		std::string content = getChunks(fd);
		std::string response_body;
		std::string filepath = _requestMap["URI"];
		filepath.replace(filepath.find(location._location_name), location._location_name.size(), location.getRoot());
		size_t pos = filepath.find("//");
		if (pos != std::string::npos)
			filepath.erase(pos, 1);
		std::cout<<"filepath:"<<filepath<<std::endl;
		std::ofstream file(filepath.c_str(), std::ios::out | std::ios::trunc);
		if (file.is_open()){
			response_body = executeCGI(location, content);
			if (response_body.find("Status") != std::string::npos)
				response_body.erase(0, response_body.find("\r\n\r\n") + 4);
			std::cout<<"final respone body size:"<<response_body.size()<<std::endl;
			file << response_body;
			file.close();
			std::string response = "HTTP/1.1 201 Created\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: " + std::to_string(response_body.size()) + "\r\n\r\n";
			if (response_body != "")
				response += response_body;
			std::cout<<"response body size"<<response_body.size() <<std::endl;
			if (send(fd, response.c_str(), response.size(), MSG_NOSIGNAL) == -1)
				std::cout << "Send error!\n";
			return ;
		}
		file.close();
		// manca pezzo per $uri/
		default_error_answer(404, fd, location);
	}
}

std::string	Server::executeCGI(Config &location, std::string &content){
	pid_t	pid;
	int		std_cpy[2] = { dup(0), dup(1) };
	char	**env = getEnvCgi(location);
	std::string _retBody = "";

	// UNCOMMENT TO PRINT ENV
	// int i = 0;
	// while (env[i])
	//		printf("%s\n", env[i++]);

	// use tmpFile() instead of pipe() to handle big amount of data
	FILE*	in = tmpfile();
	FILE*	out = tmpfile();
	int		fd_in = fileno(in);
	int		fd_out = fileno(out);

	write(fd_in, content.c_str(), content.size());
	lseek(fd_in, 0, SEEK_SET);

	pid = fork();

	if (pid == -1) {
		std::cout << "Fork failed" << std::endl;
		return ("Status: 500\r\n\r\n");
	}
	else if (!pid) {
		std::string uri = _requestMap["URI"];
		uri.replace(uri.find(location._location_name), location._location_name.size(), location.getRoot());
		size_t pos = uri.find("//");
		if (pos != std::string::npos)
			uri.erase(pos, 1);
		std::cout<<"uri:"<<uri<<std::endl;
		const char *filepath = uri.c_str();
		char *const args[3] = {strdup(location.getCgiPass().c_str()), strdup(filepath), NULL};
		dup2(fd_in, 0);
		dup2(fd_out, 1);
		std::cout<<"args0"<<args[0]<<std::endl;
		std::cout<<"args1"<<args[1]<<std::endl;
		std::cout<<"args2"<<args[2]<<std::endl;
		execve(args[0], args, env);
		std::cout << "exxxecve failed" << std::endl;
		write(1, "Status: 500\r\n\r\n", 15);
		exit (0);
	}
	else {
		waitpid(-1, NULL, 0);

		lseek(fd_out, 0, SEEK_SET);

		char	buffer[1024];
		size_t  dataRead = 0;
		while ((dataRead = read(fd_out, buffer, sizeof buffer)) > 0) {
			for (size_t i = 0; i < dataRead; i++)
				_retBody.push_back(buffer[i]);
			memset(buffer, 0, sizeof buffer);
		}
	}
	fclose(in);
	fclose(out);
	close(fd_in);
	close(fd_out);

	dup2(std_cpy[0], 0);
	dup2(std_cpy[1], 1);

	for (short i = 0; env[i] != NULL; i++)
		delete[] env[i];
	delete[] env;
	return _retBody;
}

char	**Server::getEnvCgi(Config &location) {
	sSMap envMap;

	envMap.insert(std::make_pair("AUTH_TYPE", ""));
	envMap.insert(std::make_pair("CONTENT_LENGTH", ""));
	envMap.insert(std::make_pair("CONTENT_TYPE", "application/x-www-form-urlencoded"));
	envMap.insert(std::make_pair("GATEWAY_INTERFACE", "CGI/1.1"));
	envMap.insert(std::make_pair("PATH_INFO", _requestMap["URI"]));
	envMap.insert(std::make_pair("PATH_TRANSLATED", _requestMap["URI_TRANSLATED"]));
	envMap.insert(std::make_pair("QUERY_STRING", ""));
	envMap.insert(std::make_pair("REMOTE_ADDR", _requestMap["Host"]));
	envMap.insert(std::make_pair("REMOTE_HOST", ""));
	envMap.insert(std::make_pair("REMOTE_IDENT", ""));
	envMap.insert(std::make_pair("REMOTE_USER", ""));
	envMap.insert(std::make_pair("REQUEST_METHOD", _requestMap["HTTP_method"]));
	envMap.insert(std::make_pair("REQUEST_URI", _requestMap["URI"]));
	envMap.insert(std::make_pair("SCRIPT_NAME", location.getCgiPass().substr(location.getCgiPass().find_last_of("/") + 1)));
	envMap.insert(std::make_pair("SERVER_NAME", "http://" + _requestMap["Host"]));
	envMap.insert(std::make_pair("SERVER_PORT", std::to_string(_config.getListen())));
	envMap.insert(std::make_pair("SERVER_PROTOCOL", "HTTP/1.1"));
	envMap.insert(std::make_pair("SERVER_SOFTWARE", "Webserv/1.0"));
	envMap.insert(std::make_pair("REDIRECT_STATUS", "200"));
	if (_requestMap["X-Secret-Header-For-Test"] != ""){
		std::cout<<"secret head:"<< _requestMap["X-Secret-Header-For-Test"]<< std::endl;
		envMap.insert(std::make_pair("HTTP_X_SECRET_HEADER_FOR_TEST", _requestMap["X-Secret-Header-For-Test"]));}
	char	**env = new char*[envMap.size() + 1];
	int	j = 0;
	for (sSMap::const_iterator i = envMap.begin(); i != envMap.end(); i++) {
		std::string	element = i->first + "=" + i->second;
		env[j] = new char[element.size() + 1];
		env[j] = strcpy(env[j], (const char*)element.c_str());
		//std::cout<<"riga env:"<<env[j]<<std::endl;
		j++;
	}
	env[j] = NULL;
	return env;
}