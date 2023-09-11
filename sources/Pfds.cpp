#include "../includes/Pfds.hpp"

Pfds::Pfds(void)
	: _socketArr(NULL), _size(5), _count(0) {

		_socketArr = (struct pollfd *) malloc (sizeof(struct pollfd) * (_size));
	}

Pfds::~Pfds() {
	// if (_socketArr != nullptr)
	// 	free(_socketArr);
}

void	Pfds::addToPfds(int new_fd){
	if (_count == _size){
		_size *= 2;
		_socketArr = (struct pollfd *) realloc (_socketArr, sizeof(struct pollfd) * (_size));
	}
	_socketArr[_count].fd = new_fd;
	_socketArr[_count].events = POLLIN;
	_count++;
	//std::cout << "nuovo socket allocato, grandezza:" << *fd_size << " count: " << *fd_count << std::endl;
}

void	Pfds::delFromPfds(int i){
	_socketArr[i] = _socketArr[_count - 1];
	_count--;
}

struct pollfd *Pfds::getSocketArr() const {
	return _socketArr;
}

size_t	Pfds::getSize() const {
	return _size;
}
size_t	Pfds::getCount() const {
	return _count;
}
