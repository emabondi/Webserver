#include "../includes/Server.hpp"

void	Server::handlePOST(int fd, Config &location) {
	if (_requestMap["Transfer-Encoding"] == "chunked") {
		std::string line = _requestMap["Last"];
		if (line == "") {
			char buf[256];
			std::cout<<"pre recv0"<<std::endl;
			short nbytes = recv(fd, buf, 255, 0);
			std::cout<<"post recv0"<<std::endl;
			buf[nbytes] = '\0';
			line += buf;
		}
		std::stringstream content;
		while (getChunk(line, fd) == true) {
			if (content.str().size() + line.size() > location.getClientMaxBodySize() && location.getClientMaxBodySize() != 0)
				return default_error_answer(413, fd, location);
			if (line.rfind("\r\n")){
				std::cout<< "trovato carriage in line:"<<line<<std::endl;
				line.pop_back();
				line.pop_back();
			}
			content << line;
			line.clear();
			char buf[256];
			std::cout<<"pre recv"<<std::endl;
			short nbytes = recv(fd, buf, 255, 0);
			std::cout<<"post recv"<<std::endl;
			if (nbytes == 0) {
				std::cout<<"recv mi da 0 esco"<<std::endl;
				break ;
			}
			buf[nbytes] = '\0';
			line += buf;
			std::cout<<"size "<<line.size() <<" line da mandare a getChunk"<<std::endl;
		}
		char buf[256];
			std::cout<<"pre recv2"<<std::endl;
			short nbytes = recv(fd, buf, 255, 0);
			std::cout<<"post recv2"<<std::endl;
			buf[nbytes] = '\0';
			std::cout<< "altro? nbytes:"<<nbytes<<" "<<buf<<std::endl; 
		std::string response_body;
		std::cout<<" content size:"<<content.str().size()<<std::endl;
		if (content.str().size() != 0 && location.getCgiPass() != "")
			response_body = executeCGI(location, content);
		std::cout<<"response body "<<response_body<<std::endl;
		std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(response_body.size()) + "\r\n\r\n";
		if (response_body != "")
			response += response_body + "\r\n";
		std::cout<<"response "<<response<<std::endl;
		if (send(fd, response.c_str(), response.size(), MSG_NOSIGNAL) == -1)
			std::cout << "Send error!\n";
	}
}

std::string	Server::executeCGI(Config &location, std::stringstream &content){
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

	write(fd_in, content.rdbuf(), content.str().size());
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
		const char *filepath = uri.c_str();
		char *const args[3] = {strdup(location.getCgiPass().c_str()), strdup(filepath), NULL};
		dup2(fd_in, 0);
		dup2(fd_out, 1);
/*		std::cout<<"cgipass "<<location.getCgiPass()<<std::endl;
		std::cout<<"args0"<<args[0]<<std::endl;
		std::cout<<"args1"<<args[1]<<std::endl;
		std::cout<<"args2"<<args[2]<<std::endl;*/
		execve(location.getCgiPass().c_str(), args, env);
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
			std::ofstream file("prova.txt", std::ios::out | std::ios::trunc) ;
			file << _retBody.c_str();
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
	//if (conn.headers.find("X-Secret") != conn.headers.npos)
	//    envMap.insert(std::make_pair("HTTP_X_SECRET_HEADER_FOR_TEST", conn.headers.substr(conn.headers.find("X-Secret") + 26, conn.headers.find_first_of("\r\n"))));
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