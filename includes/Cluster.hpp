#ifndef	CLUSTER_HPP
# define CLUSTER_HPP

# include <iostream>
# include <fstream>
# include <vector>
# include <sstream>
# include "Config.hpp"
# include "Server.hpp"
# include "Pfds.hpp"

typedef	std::map<std::string, Config> sCMap;

class Cluster {
	private:
		std::vector<Server>	_serverVec;
		
		Cluster(void);

	public:
		Cluster(const char *filePath);
		~Cluster(void);

		sVec								divideByServer(std::string &fileContent);
		void								displayServerConfig(Config &config) const;

		struct	wrongFilePath : public std::exception {
			virtual const char *what() const throw();
		};

		struct badConfigFile : public std::exception {
			virtual const char *what() const throw();
		};
};

#endif
