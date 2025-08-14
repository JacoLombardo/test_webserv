/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollEventHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 14:06:48 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/08 14:17:02 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/Handlers/Handlers.hpp"

void Handlers::Epoll::processEpollEvents(WebServer *server, const struct epoll_event *events, int event_count) {
	for (int i = 0; i < event_count; ++i) {
		const uint32_t event_mask = events[i].events;
		const int fd = events[i].data.fd;

		server->getLogger().debug("Epoll event on fd=" + su::to_string(fd) + " (" +
		            describeEpollEvents(event_mask) + ")");

		if (isListeningSocket(server, fd)) {
			// TODO: NULL check
			ServerConfig *sc = ServerConfig::find(server->getConfigs(), fd);
			Handlers::Connection::handleNewConnection(server, sc);
		} else if (Handlers::CGIHandler::isCGIFd(server, fd)) {
			Handlers::CGIHandler::handleCGIOutput(server, fd);
		} else {
			handleClientEvent(server, fd, event_mask);
		}
	}
}

bool Handlers::Epoll::isListeningSocket(WebServer *server, int fd) {
	for (std::vector<ServerConfig>::const_iterator it = server->getConfigs().begin(); it != server->getConfigs().end(); ++it) {
		if (fd == it->getServerFd()) {
			return true;
		}
	}
	return false;
}

void Handlers::Epoll::handleClientEvent(WebServer *server, int fd, uint32_t event_mask) {
	std::map<int, ::Connection *>::iterator conn_it = server->getConnections().find(fd);
	if (conn_it != server->getConnections().end()) {
		::Connection *conn = conn_it->second;
		if (event_mask & EPOLLIN) {
			handleClientRecv(server, conn);
		}
		if (event_mask & EPOLLOUT) {
			if (conn->responseReady())
				Handlers::Response::sendResponse(server, conn);
			if (!conn->keepPersistentConnection())
				Handlers::Connection::closeConnection(server, conn);
		}
	} else {
		server->getLogger().debug("Ignoring event for unknown fd: " + su::to_string(fd));
	}
}

void Handlers::Epoll::handleClientRecv(WebServer *server, ::Connection *conn) {
	server->getLogger().debug("Updated last activity for FD " + su::to_string(conn->getFd()));
	conn->publicUpdateActivity();

	char buffer[4096 * 3];
	ssize_t total_bytes_read = 0;

	ssize_t bytes_read = receiveData(server, conn->getFd(), buffer, sizeof(buffer) - 1);

	if (bytes_read > 0) {
		total_bytes_read += bytes_read;

		if (!processReceivedData(server, conn, buffer, bytes_read, total_bytes_read)) {
			return;
		}
	} else if (bytes_read == 0) {
		server->getLogger().warn("Client (fd: " + su::to_string(conn->getFd()) + ") closed connection");
		conn->keepPersistentConnection() = false;
		Handlers::Connection::closeConnection(server, conn);
		return;
	} else if (bytes_read < 0) {
		server->getLogger().error("recv error for fd " + su::to_string(conn->getFd()) + ": " + strerror(errno));
		Handlers::Connection::closeConnection(server, conn);
		return;
	}
}

ssize_t Handlers::Epoll::receiveData(WebServer *server, int client_fd, char *buffer, size_t buffer_size) {
	errno = 0;
	ssize_t bytes_read = recv(client_fd, buffer, buffer_size, 0);

	server->getLogger().logWithPrefix(Logger::DEBUG, "recv", "Bytes received: " + su::to_string(bytes_read));
	if (bytes_read > 0) {
		buffer[bytes_read] = '\0';
	}

	server->getLogger().logWithPrefix(Logger::DEBUG, "recv", "Data: " + std::string(buffer));

	return bytes_read;
}

bool Handlers::Epoll::processReceivedData(WebServer *server, ::Connection *conn, const char *buffer, ssize_t bytes_read,
                                    ssize_t total_bytes_read) {
	conn->readBuffer() += std::string(buffer);

	server->getLogger().debug("Checking if request was completed");
	if (Handlers::Request::isRequestComplete(server, conn)) {
		if (!server->epollCtl(EPOLL_CTL_MOD, conn->getFd(), EPOLLIN | EPOLLOUT)) {
			return false;
		}
		server->getLogger().debug("Request was completed");
		if (conn->chunkedFlag() && conn->stateRef() == ::Connection::CONTINUE_SENT) {
			return true;
		}
		if (!conn->getServerConfig()->infiniteBodySize() &&
		    total_bytes_read > static_cast<ssize_t>(conn->getServerConfig()->getMaxBodySize())) {
			server->getLogger().debug("Request is too large");
			Handlers::Request::handleRequestTooLarge(server, conn, bytes_read);
			return false;
		}

		return Handlers::Request::handleCompleteRequest(server, conn);
	}

	return true;
}
