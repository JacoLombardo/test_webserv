/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 14:09:35 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/08 14:19:08 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/Handlers/Handlers.hpp"

void Handlers::Connection::handleNewConnection(WebServer *server, ServerConfig *sc) {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(sc->getServerFd(), (struct sockaddr *)&client_addr, &client_len);
	if (client_fd == -1) {
		// TODO: cannot accept a connection with the client
	}

	if (!server->setFdNonBlocking(client_fd)) {
		close(client_fd);
		return;
	}

	// TODO: error checks
	::Connection *conn = addConnection(server, client_fd, sc);

	if (!server->epollCtl(EPOLL_CTL_ADD, client_fd, EPOLLIN)) {
		closeConnection(server, conn);
	}

	server->getLogger().info("New connection from " + std::string(inet_ntoa(client_addr.sin_addr)) + ":" +
	           su::to_string<unsigned short>(ntohs(client_addr.sin_port)) +
	           " (fd: " + su::to_string<int>(client_fd) + ")");
}

::Connection *Handlers::Connection::addConnection(WebServer *server, int client_fd, ServerConfig *sc) {
	::Connection *conn = new ::Connection(client_fd);
	conn->locationConfig() = sc->defaultLocation();
	server->getConnections()[client_fd] = conn;

	server->getLogger().debug("Added connection tracking for fd: " + su::to_string(client_fd));
	return conn;
}

void Handlers::Connection::cleanupExpiredConnections(WebServer *server) {
	time_t current_time = server->now();

	if (current_time - server->getLastCleanup() < server->getCleanupInterval()) {
		return;
	}

	server->setLastCleanup(current_time);

	std::vector< ::Connection * > expired;

	// Collect expired connections
	for (std::map<int, ::Connection *>::iterator it = server->getConnections().begin(); it != server->getConnections().end();
	     ++it) {

		::Connection *conn = it->second;
		if (conn->publicIsExpired(time(NULL), server->getConnectionTimeout())) {
			conn->keepPersistentConnection() = false;
			expired.push_back(conn);
			server->getLogger().info("Connection expired for fd: " + su::to_string(conn->getFd()));
		}
	}

	for (size_t i = 0; i < expired.size(); ++i) {
		closeConnection(server, expired[i]);
	}
	expired.clear();
}

void Handlers::Connection::handleConnectionTimeout(WebServer *server, int client_fd) {
	std::map<int, ::Connection *>::iterator it = server->getConnections().find(client_fd);
	if (it != server->getConnections().end()) {
		::Connection *conn = it->second;

		Handlers::Response::prepareResponse(server, conn, ::Response(408, conn));

		server->getLogger().info("Connection timed out for fd: " + su::to_string(client_fd) + " (idle for " +
	           su::to_string(server->now() - conn->lastActivity()) + " seconds)");
		closeConnection(server, conn);
	}
}

void Handlers::Connection::closeConnection(WebServer *server, ::Connection *conn) {
	if (!conn)
		return;

	// TODO: redundant check may be removed
	if (conn->keepPersistentConnection()) {
		server->getLogger().debug("Ignoring connection close request for fd: " + su::to_string(conn->getFd()));
		return;
	}
	server->getLogger().debug("Closing connection for fd: " + su::to_string(conn->getFd()));

	epoll_ctl(server->getEpollFd(), EPOLL_CTL_DEL, conn->getFd(), NULL);
	close(conn->getFd());

	std::map<int, ::Connection *>::iterator it = server->getConnections().find(conn->getFd());
	if (it != server->getConnections().end()) {
		server->getConnections().erase(conn->getFd());
	}
	server->getLogger().debug("Connection cleanup completed for fd: " + su::to_string(conn->getFd()));
	delete conn;
}
