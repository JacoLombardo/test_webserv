/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestWrapper.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/29 09:28:01 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/29 09:28:02 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/Handlers/Handlers.hpp"

namespace RequestHandler {
bool isHeadersComplete(WebServer *server, Connection *conn) { return server->isHeadersComplete(conn); }
bool isRequestComplete(WebServer *server, Connection *conn) { return server->isRequestComplete(conn); }
bool reconstructRequest(WebServer *server, Connection *conn) { return server->reconstructRequest(conn); }
bool parseRequest(WebServer *server, Connection *conn, ClientRequest &req) { return server->parseRequest(conn, req); }
void processRequest(WebServer *server, Connection *conn) { server->processRequest(conn); }
void processValidRequest(WebServer *server, ClientRequest &req, Connection *conn) { server->processValidRequest(req, conn); }
}



// EOF
namespace ReqValidationHandler {
bool matchLocation(WebServer *server, ClientRequest &req, Connection *conn) { return server->matchLocation(req, conn); }
bool normalizePath(WebServer *server, ClientRequest &req, Connection *conn) { return server->normalizePath(req, conn); }
bool processValidRequestChecks(WebServer *server, ClientRequest &req, Connection *conn) { return server->processValidRequestChecks(req, conn); }
}
