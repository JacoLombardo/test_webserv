#include "src/HttpServer/Handlers/Handlers.hpp"

namespace EpollEventHandler {
void processEpollEvents(WebServer *server, const struct epoll_event *events, int event_count) {
	server->processEpollEvents(events, event_count);
}

// EOF


bool isListeningSocket(WebServer *server, int fd) {
	return server->isListeningSocket(fd);
}

void handleClientEvent(WebServer *server, int fd, uint32_t event_mask) {
	server->handleClientEvent(fd, event_mask);
}

void handleClientRecv(WebServer *server, Connection *conn) {
	server->handleClientRecv(conn);
}

ssize_t receiveData(WebServer *server, int client_fd, char *buffer, size_t buffer_size) {
	return server->receiveData(client_fd, buffer, buffer_size);
}

bool processReceivedData(WebServer *server, Connection *conn, const char *buffer, ssize_t bytes_read) {
	return server->processReceivedData(conn, buffer, bytes_read);
}
}
