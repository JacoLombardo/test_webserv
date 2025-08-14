/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 14:08:41 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/08 14:17:10 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/Handlers/Handlers.hpp"
#include "src/HttpServer/Structs/WebServer.hpp"
#include "src/HttpServer/Structs/Connection.hpp"
#include "src/HttpServer/Structs/Response.hpp"
#include "src/HttpServer/HttpServer.hpp"

ssize_t Handlers::Response::prepareResponse(WebServer *server, ::Connection *conn, const ::Response &resp) {
	// TODO: some checks if the arguments are fine to work with
	// TODO: make sure that Response has all required headers set up correctly (e.g. Content-Type,
	// Content-Length, etc).
	if (conn->responseReady()) {
		server->getLogger().error(
		    "Trying to prepare a response for a connection that is ready to sent another one");
		server->getLogger().error("Current response: " + conn->responseObj().toShortString());
		server->getLogger().error("Trying to prepare response: " + resp.toShortString());
		return -1;
	}
	server->getLogger().debug("Saving a response [" + su::to_string(resp.status_code) + "] for fd " +
	            su::to_string(conn->getFd()));
	conn->responseObj() = resp;
	conn->responseReady() = true;
	return conn->responseObj().toString().size();
	// return send(clfd, raw_response.c_str(), raw_response.length(), 0);
}

bool Handlers::Response::sendResponse(WebServer *server, ::Connection *conn) {
	if (!conn->responseReady()) {
		server->getLogger().error("Response is not ready to be sent back to the client");
		server->getLogger().debug("Error for clinet " + conn->debugString());
		return false;
	}
	server->getLogger().debug("Sending response [" + conn->responseObj().toShortString() +
	            "] back to fd: " + su::to_string(conn->getFd()));
	std::string raw_response = conn->responseObj().toString();
	server->epollCtl(EPOLL_CTL_MOD, conn->getFd(), EPOLLIN);
	conn->responseObj().reset();
	conn->responseReady() = false;
	return send(conn->getFd(), raw_response.c_str(), raw_response.size(), MSG_NOSIGNAL) != -1;
}
