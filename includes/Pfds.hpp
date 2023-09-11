#ifndef	PFDS_HPP
# define PFDS_HPP

#include <cstdlib>
#include <poll.h>

class Pfds {
	private:
		struct pollfd	*_socketArr;
		size_t			_size;
		size_t			_count;
	public:
		Pfds();
		~Pfds();

		void			addToPfds(int new_fd);
		void			delFromPfds(int i);

		struct pollfd	*getSocketArr() const;
		size_t			getSize() const;
		size_t			getCount() const;
};

#endif