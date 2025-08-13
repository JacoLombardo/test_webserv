/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 11:47:14 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/08 13:12:21 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "includes/Webserv.hpp"

class WebServer;
class Connection;
class Response;
class CGI;
class ServerConfig;
struct ClientRequest;

namespace Handlers {

namespace Epoll {
void processEpollEvents(WebServer &server, const struct epoll_event *events, int event_count);
bool isListeningSocket(const WebServer &server, int fd);
void handleClientEvent(WebServer &server, int fd, uint32_t event_mask);
void handleClientRecv(WebServer &server, Connection *conn);
ssize_t receiveData(WebServer &server, int client_fd, char *buffer, size_t buffer_size);
bool processReceivedData(WebServer &server, Connection *conn, const char *buffer,
                         ssize_t bytes_read, ssize_t total_bytes_read);
}

namespace Conn {
void handleNewConnection(WebServer &server, ServerConfig *sc);
Connection *addConnection(WebServer &server, int client_fd, ServerConfig *sc);
void cleanupExpiredConnections(WebServer &server);
void handleConnectionTimeout(WebServer &server, int client_fd);
void closeConnection(WebServer &server, Connection *conn);
}

namespace Dir {
Response handleDirectoryRequest(WebServer &server, Connection *conn, const std::string &fullDirPath);
Response generateDirectoryListing(WebServer &server, Connection *conn, const std::string &fullDirPath);
Response handleFileRequest(WebServer &server, Connection *conn, const std::string &fullDirPath);
Response handleReturnDirective(WebServer &server, Connection *conn, uint16_t code, std::string target);
std::string getExtension(const std::string &path);
std::string detectContentType(const std::string &path);
}

namespace Chunked {
bool processChunkSize(WebServer &server, Connection *conn);
bool processChunkData(WebServer &server, Connection *conn);
bool processTrailer(WebServer &server, Connection *conn);
void reconstructChunkedRequest(WebServer &server, Connection *conn);
}

namespace Req {
bool handleCGIRequest(WebServer &server, ClientRequest &req, Connection *conn);
void handleRequestTooLarge(WebServer &server, Connection *conn, ssize_t bytes_read);
bool handleCompleteRequest(WebServer &server, Connection *conn);
bool isHeadersComplete(WebServer &server, Connection *conn);
bool isRequestComplete(WebServer &server, Connection *conn);
void processRequest(WebServer &server, Connection *conn);
bool parseRequest(WebServer &server, Connection *conn, ClientRequest &req);
}

namespace Resp {
ssize_t prepareResponse(WebServer &server, Connection *conn, const Response &resp);
bool sendResponse(WebServer &server, Connection *conn);
}

namespace CGIHandlers {
void sendCGIResponse(WebServer &server, std::string &cgi_output, CGI *cgi, Connection *conn);
void normalResponse(WebServer &server, CGI *cgi, Connection *conn);
void chunkedResponse(WebServer &server, CGI *cgi, Connection *conn);
void handleCGIOutput(WebServer &server, int fd);
bool isCGIFd(const WebServer &server, int fd);
}

} // namespace Handlers

#endif
