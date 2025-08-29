/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: recreated                                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/29 00:00:00 by recreated         #+#    #+#             */
/*   Updated: 2025/08/29 00:00:00 by recreated        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HANDLERS_HPP
#define HTTP_HANDLERS_HPP

#include "includes/Webserv.hpp"
#include "src/HttpServer/Structs/Connection.hpp"
#include "src/HttpServer/Structs/Response.hpp"
#include "src/HttpServer/Structs/WebServer.hpp"
#include "src/Utils/ServerUtils.hpp"

// Inline wrappers: namespace functions delegating to WebServer members

namespace EpollEventHandler {
    inline void processEpollEvents(WebServer *server, const struct epoll_event *events, int event_count) { server->processEpollEvents(events, event_count); }
    inline bool isListeningSocket(WebServer *server, int fd) { return server->isListeningSocket(fd); }
    inline void handleClientEvent(WebServer *server, int fd, uint32_t event_mask) { server->handleClientEvent(fd, event_mask); }
    inline void handleClientRecv(WebServer *server, Connection *conn) { server->handleClientRecv(conn); }
    inline ssize_t receiveData(WebServer *server, int client_fd, char *buffer, size_t buffer_size) { return server->receiveData(client_fd, buffer, buffer_size); }
    inline bool processReceivedData(WebServer *server, Connection *conn, const char *buffer, ssize_t bytes_read) { return server->processReceivedData(conn, buffer, bytes_read); }
}

namespace ConnectionHandler {
    inline void handleNewConnection(WebServer *server, ServerConfig *sc) { server->handleNewConnection(sc); }
    inline Connection *addConnection(WebServer *server, int client_fd, ServerConfig *sc) { return server->addConnection(client_fd, sc); }
    inline void cleanupExpiredConnections(WebServer *server) { server->cleanupExpiredConnections(); }
    inline void handleConnectionTimeout(WebServer *server, int client_fd) { server->handleConnectionTimeout(client_fd); }
    inline void closeConnection(WebServer *server, Connection *conn) { server->closeConnection(conn); }
}

namespace ResponseHandler {
    inline ssize_t prepareResponse(WebServer *server, Connection *conn, const Response &resp) { return server->prepareResponse(conn, resp); }
    inline bool sendResponse(WebServer *server, Connection *conn) { return server->sendResponse(conn); }
}

namespace StaticGetRespHandler {
    inline Response respDirectoryRequest(WebServer *server, Connection *conn, const std::string &fullDirPath) { return server->respDirectoryRequest(conn, fullDirPath); }
    inline Response generateDirectoryListing(WebServer *server, Connection *conn, const std::string &fullDirPath) { return server->generateDirectoryListing(conn, fullDirPath); }
    inline Response respFileRequest(WebServer *server, Connection *conn, const std::string &fullFilePath) { return server->respFileRequest(conn, fullFilePath); }
    inline Response respReturnDirective(WebServer *server, Connection *conn, uint16_t code, std::string target) { return server->respReturnDirective(conn, code, target); }
    inline void handleDirectoryRequest(WebServer *server, ClientRequest &req, Connection *conn, bool end_slash) { server->handleDirectoryRequest(req, conn, end_slash); }
    inline void handleFileRequest(WebServer *server, ClientRequest &req, Connection *conn, bool end_slash) { server->handleFileRequest(req, conn, end_slash); }
}

namespace RequestHandler {
    inline bool isHeadersComplete(WebServer *server, Connection *conn) { return server->isHeadersComplete(conn); }
    inline bool isRequestComplete(WebServer *server, Connection *conn) { return server->isRequestComplete(conn); }
    inline bool reconstructRequest(WebServer *server, Connection *conn) { return server->reconstructRequest(conn); }
    inline bool parseRequest(WebServer *server, Connection *conn, ClientRequest &req) { return server->parseRequest(conn, req); }
    inline void processRequest(WebServer *server, Connection *conn) { server->processRequest(conn); }
    inline void processValidRequest(WebServer *server, ClientRequest &req, Connection *conn) { server->processValidRequest(req, conn); }
    inline bool handleCompleteRequest(WebServer *server, Connection *conn) { return server->handleCompleteRequest(conn); }
}

namespace ReqValidationHandler {
    inline bool matchLocation(WebServer *server, ClientRequest &req, Connection *conn) { return server->matchLocation(req, conn); }
    inline bool normalizePath(WebServer *server, ClientRequest &req, Connection *conn) { return server->normalizePath(req, conn); }
    inline bool processValidRequestChecks(WebServer *server, ClientRequest &req, Connection *conn) { return server->processValidRequestChecks(req, conn); }
}

namespace ChunkedReqHandler {
    inline bool processChunkSize(WebServer *server, Connection *conn) { return server->processChunkSize(conn); }
    inline bool processChunkData(WebServer *server, Connection *conn) { return server->processChunkData(conn); }
    inline bool processTrailer(WebServer *server, Connection *conn) { return server->processTrailer(conn); }
    inline void reconstructChunkedRequest(WebServer *server, Connection *conn) { server->reconstructChunkedRequest(conn); }
}

namespace ServerCGIHandler {
    inline uint16_t handleCGIRequest(WebServer *server, ClientRequest &req, Connection *conn) { return server->handleCGIRequest(req, conn); }
    inline bool prepareCGIResponse(WebServer *server, CGI *cgi, Connection *conn) { return server->prepareCGIResponse(cgi, conn); }
    inline void handleCGIOutput(WebServer *server, int fd) { server->handleCGIOutput(fd); }
    inline bool isCGIFd(WebServer *server, int fd) { return server->isCGIFd(fd); }
}

#endif

