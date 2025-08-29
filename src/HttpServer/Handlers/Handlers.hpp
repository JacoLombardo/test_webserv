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
    void processEpollEvents(WebServer *server, const struct epoll_event *events, int event_count);
    bool isListeningSocket(WebServer *server, int fd);
    void handleClientEvent(WebServer *server, int fd, uint32_t event_mask);
    void handleClientRecv(WebServer *server, Connection *conn);
    ssize_t receiveData(WebServer *server, int client_fd, char *buffer, size_t buffer_size);
    bool processReceivedData(WebServer *server, Connection *conn, const char *buffer, ssize_t bytes_read);
}

namespace ConnectionHandler {
    void handleNewConnection(WebServer *server, ServerConfig *sc);
    Connection *addConnection(WebServer *server, int client_fd, ServerConfig *sc);
    void cleanupExpiredConnections(WebServer *server);
    void handleConnectionTimeout(WebServer *server, int client_fd);
    void closeConnection(WebServer *server, Connection *conn);
}

namespace ResponseHandler {
    ssize_t prepareResponse(WebServer *server, Connection *conn, const Response &resp);
    bool sendResponse(WebServer *server, Connection *conn);
}

namespace StaticGetRespHandler {
    Response respDirectoryRequest(WebServer *server, Connection *conn, const std::string &fullDirPath);
    Response generateDirectoryListing(WebServer *server, Connection *conn, const std::string &fullDirPath);
    Response respFileRequest(WebServer *server, Connection *conn, const std::string &fullFilePath);
    Response respReturnDirective(WebServer *server, Connection *conn, uint16_t code, std::string target);
    void handleDirectoryRequest(WebServer *server, ClientRequest &req, Connection *conn, bool end_slash);
    void handleFileRequest(WebServer *server, ClientRequest &req, Connection *conn, bool end_slash);
}

namespace RequestHandler {
    bool isHeadersComplete(WebServer *server, Connection *conn);
    bool isRequestComplete(WebServer *server, Connection *conn);
    bool reconstructRequest(WebServer *server, Connection *conn);
    bool parseRequest(WebServer *server, Connection *conn, ClientRequest &req);
    void processRequest(WebServer *server, Connection *conn);
    void processValidRequest(WebServer *server, ClientRequest &req, Connection *conn);
    bool handleCompleteRequest(WebServer *server, Connection *conn);
}

namespace ReqValidationHandler {
    bool matchLocation(WebServer *server, ClientRequest &req, Connection *conn);
    bool normalizePath(WebServer *server, ClientRequest &req, Connection *conn);
    bool processValidRequestChecks(WebServer *server, ClientRequest &req, Connection *conn);
}

namespace ChunkedReqHandler {
    bool processChunkSize(WebServer *server, Connection *conn);
    bool processChunkData(WebServer *server, Connection *conn);
    bool processTrailer(WebServer *server, Connection *conn);
    void reconstructChunkedRequest(WebServer *server, Connection *conn);
}

namespace ServerCGIHandler {
    uint16_t handleCGIRequest(WebServer *server, ClientRequest &req, Connection *conn);
    bool prepareCGIResponse(WebServer *server, CGI *cgi, Connection *conn);
    void handleCGIOutput(WebServer *server, int fd);
    bool isCGIFd(WebServer *server, int fd);
}

#endif

