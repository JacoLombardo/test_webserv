/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 14:10:22 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/08 14:17:07 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/Handlers/Handlers.hpp"
#include "src/HttpServer/Structs/WebServer.hpp"
#include "src/HttpServer/Structs/Connection.hpp"
#include "src/HttpServer/Structs/Response.hpp"
#include "src/HttpServer/HttpServer.hpp"

/* Request handlers */

void Handlers::Request::handleRequestTooLarge(WebServer *server, ::Connection *conn, ssize_t bytes_read) {
	server->getLogger().info("Reached max content length for fd: " + su::to_string(conn->getFd()) + ", " +
	           su::to_string(bytes_read) + "/" +
	           su::to_string(conn->getServerConfig()->getMaxBodySize()));
	Handlers::Response::prepareResponse(server, conn, ::Response(413, conn));
	// closeConnection(conn);
}

bool Handlers::Request::handleCompleteRequest(WebServer *server, ::Connection *conn) {
	processRequest(server, conn);

	server->getLogger().debug("Request was processed. Read buffer will be cleaned");
	conn->readBuffer().clear();
	conn->requestCount()++;
	conn->publicUpdateActivity();
	return true; // Continue processing
}

bool Handlers::Request::handleCGIRequest(WebServer *server, ClientRequest &req, ::Connection *conn) {
	Logger _lggr;

	CGI *cgi = CGIUtils::createCGI(req, conn->locationConfig());
	if (!cgi)
		return (false);
	server->getCgiPool()[cgi->getOutputFd()] = std::make_pair(cgi, conn);
	if (!server->epollCtl(EPOLL_CTL_ADD, cgi->getOutputFd(), EPOLLIN)) {
		server->getLogger().error("EPollManage for CGI request failed.");
		return (false);
	}
	return (true);
}

/* Request processing */

bool Handlers::Request::isHeadersComplete(WebServer *server, ::Connection *conn) {
	size_t header_end = conn->readBuffer().find("\r\n\r\n");
	if (header_end == std::string::npos) {
		return false;
	}

	// Headers are complete, check if this is a chunked request
	std::string headers = conn->readBuffer().substr(0, header_end + 4);

	std::string headers_lower = su::to_lower(headers);

	if (headers_lower.find("transfer-encoding: chunked") != std::string::npos) {
		conn->chunkedFlag() = true;
		// conn->state = Connection::READING_CHUNK_SIZE;
		conn->headersBuffer() = headers;

		if (headers_lower.find("expect: 100-continue") != std::string::npos) {
			Handlers::Response::prepareResponse(server, conn, ::Response::continue_());

			conn->stateRef() = ::Connection::CONTINUE_SENT;

			conn->readBuffer().clear();

			conn->chunkSize() = 0;
			conn->chunkBytesRead() = 0;
			conn->chunkData().clear();

			return true;
		} else {
			conn->stateRef() = ::Connection::READING_CHUNK_SIZE;

			conn->readBuffer() = conn->readBuffer().substr(header_end + 4);

			conn->chunkSize() = 0;
			conn->chunkBytesRead() = 0;
			conn->chunkData().clear();

			return Handlers::Chunked::processChunkSize(server, conn);
		}
	} else {
		conn->chunkedFlag() = false;
		conn->stateRef() = ::Connection::REQUEST_COMPLETE;
		return true;
	}
}

bool Handlers::Request::isRequestComplete(WebServer *server, ::Connection *conn) {
	switch (conn->stateRef()) {
	case ::Connection::READING_HEADERS:
		server->getLogger().debug("isRequestComplete->READING_HEADERS");
		return isHeadersComplete(server, conn);

	case ::Connection::CONTINUE_SENT:
		server->getLogger().debug("isRequestComplete->CONTINUE_SENT");
		conn->stateRef() = ::Connection::READING_CHUNK_SIZE;
		return Handlers::Chunked::processChunkSize(server, conn);

	case ::Connection::READING_CHUNK_SIZE:
		server->getLogger().debug("isRequestComplete->READING_CHUNK_SIZE");
		return Handlers::Chunked::processChunkSize(server, conn);

	case ::Connection::READING_CHUNK_DATA:
		server->getLogger().debug("isRequestComplete->READING_CHUNK_DATA");
		return Handlers::Chunked::processChunkData(server, conn);

	case ::Connection::READING_TRAILER:
		server->getLogger().debug("isRequestComplete->READING_TRAILER");
		return Handlers::Chunked::processTrailer(server, conn);

	case ::Connection::REQUEST_COMPLETE:
	case ::Connection::CHUNK_COMPLETE:
		server->getLogger().debug("isRequestComplete->REQUEST_COMPLETE");
		return true;

	default:
		server->getLogger().debug("isRequestComplete->default");
		return false;
	}
}

void Handlers::Request::processRequest(WebServer *server, ::Connection *conn) {
	server->getLogger().info("Processing request from fd: " + su::to_string(conn->getFd()));

	ClientRequest req;
	req.clfd = conn->getFd();

	if (!parseRequest(server, conn, req))
		return;
	server->getLogger().debug("Request parsed sucsessfully");

	// RFC 2068 Section 8.1 -- presistent connection unless client or server sets connection header
	// to 'close' -- indicating that the socket for this connection may be closed
	if (req.headers.find("connection") != req.headers.end()) {
		if (req.headers["connection"] == "close") {
			conn->keepPersistentConnection() = false;
		}
	}

	if (req.chunked_encoding && conn->stateRef() == ::Connection::READING_HEADERS) {
		// Accept chunked requests sequence
		server->getLogger().debug("Accepting a chunked request");
		conn->stateRef() = ::Connection::READING_CHUNK_SIZE;
		conn->chunkedFlag() = true;
		Handlers::Response::prepareResponse(server, conn, ::Response::continue_());
		return;
	}

	// TODO: this part breaks the req struct for some reason
	//       can't debug on my own :(
	if (req.chunked_encoding && conn->stateRef() == ::Connection::CHUNK_COMPLETE) {
		server->getLogger().debug("Chunked request completed!");
		server->getLogger().debug("Parsing complete chunked request");
		if (!parseRequest(server, conn, req))
			return;
		server->getLogger().debug("Chunked request parsed sucsessfully");
		server->getLogger().debug(conn->debugString());
		server->getLogger().debug(req.toString());
	}

	server->getLogger().debug("FD " + su::to_string(req.clfd) + " ClientRequest {" + req.toString() + "}");
	std::string response;
	// initialize the correct locConfig // default "/"
	LocConfig *match = findBestMatch(req.uri, conn->getServerConfig()->getLocations());
	if (!match) {
		server->getLogger().error("[Resp] No matched location for : " + req.uri);
		Handlers::Response::prepareResponse(server, conn, ::Response::internalServerError(conn));
		return;
	}
	conn->locationConfig() = match; // Set location context
	server->getLogger().debug("[Resp] Matched location : " + match->getPath());

	// check if RETURN directive in the matched location
	if (conn->locationConfig()->hasReturn()) {
		server->getLogger().debug("[Resp] The matched location has a return directive.");
		uint16_t code = conn->locationConfig()->getReturnCode();
		std::string target = conn->locationConfig()->getReturnTarget();
		Handlers::Response::prepareResponse(server, conn, Handlers::Directory::handleReturnDirective(server, conn, code, target));
		return;
	}

	// Is the method allowed?
	if (!conn->locationConfig()->hasMethod(req.method)) {
		server->getLogger().warn("Method " + req.method + " is not allowed for location " +
	           conn->locationConfig()->getPath());
		Handlers::Response::prepareResponse(server, conn, ::Response::methodNotAllowed(conn));
		return;
	}

	std::string fullPath = server->buildLocationFullPath(req.path, conn->locationConfig());
	conn->locationConfig()->setFullPath(fullPath);
	// security check
	// TODO : normalize the path
	if (fullPath.find("..") != std::string::npos) {
		server->getLogger().warn("Uri " + req.uri + " is not safe.");
		Handlers::Response::prepareResponse(server, conn, ::Response::forbidden(conn));
		return;
	}

	FileType ftype = server->getFileType(fullPath);

	// Error checking
	if (ftype == NOT_FOUND_404) {
		server->getLogger().debug("Could not open : " + fullPath);
		Handlers::Response::prepareResponse(server, conn, ::Response::notFound(conn));
		return;
	}
	if (ftype == PERMISSION_DENIED_403) {
		server->getLogger().debug("Permission denied : " + fullPath);
		Handlers::Response::prepareResponse(server, conn, ::Response::forbidden(conn));
		return;
	}
	if (ftype == FILE_SYSTEM_ERROR_500) {
		server->getLogger().debug("Other file access problem : " + fullPath);
		Handlers::Response::prepareResponse(server, conn, ::Response::internalServerError(conn));
		return;
	}

	// uri request ends with '/'
	bool endSlash = (!req.uri.empty() && req.uri[req.uri.length() - 1] == '/');

	// Directory requests
	if (ftype == ISDIR) {
		server->getLogger().debug("Directory request: " + fullPath);
		if (!endSlash) {
			server->getLogger().debug("Directory request without trailing slash, redirecting: " + req.uri);
			std::string redirectPath = req.uri + "/";
			Handlers::Response::prepareResponse(server, conn, Handlers::Directory::handleReturnDirective(server, conn, 302, redirectPath));
			return;
		} else {
			Handlers::Response::prepareResponse(server, conn, Handlers::Directory::handleDirectoryRequest(server, conn, fullPath));
			return;
		}
	}

	// Handle file requests with trailing /
	server->getLogger().debug("File request: " + fullPath);
	if (ftype == ISREG && endSlash) {
		server->getLogger().debug("File request with trailing slash, redirecting: " + req.uri);
		std::string redirectPath = req.uri.substr(0, req.uri.length() - 1);
		Handlers::Response::prepareResponse(server, conn, Handlers::Directory::handleReturnDirective(server, conn, 302, redirectPath));
		return;
	}

	// if we arrive here, this should be the only possible case
	if (ftype == ISREG && !endSlash) {

		server->getLogger().debug("File request with following extension: " + Handlers::Directory::getExtension(req.uri));

		// check if it is a script with a language supported by the location
		if (conn->locationConfig()->acceptExtension(Handlers::Directory::getExtension(req.path))) {
			std::string extPath = conn->locationConfig()->getExtensionPath(Handlers::Directory::getExtension(req.path));
			server->getLogger().debug("Extension path is : " + extPath);
			req.extension = Handlers::Directory::getExtension(req.path);
			if (!handleCGIRequest(server, req, conn)) {
				server->getLogger().error("Handling the CGI request failed.");
				Handlers::Response::prepareResponse(server, conn, ::Response::badRequest());
				// closeConnection(conn);
				return;
			}
			return;
		}

		if (req.method != "GET") {
			server->getLogger().debug("POST or DELETE request not handled by CGI -> not implemented response.");
			Handlers::Response::prepareResponse(server, conn, ::Response::notImplemented(conn));
			return;
		} else {
			Handlers::Response::prepareResponse(server, conn, Handlers::Directory::handleFileRequest(server, conn, fullPath));
			return;
		}

		server->getLogger().debug("Should never be reached");
		Handlers::Response::prepareResponse(server, conn, ::Response::internalServerError(conn));
		return;
	}
}

bool Handlers::Request::parseRequest(WebServer *server, ::Connection *conn, ClientRequest &req) {
	server->getLogger().debug("Parsing request: " + conn->debugString());
	if (!RequestParsingUtils::parseRequest(conn->readBuffer(), req)) {
		server->getLogger().error("Parsing of the request failed.");
		server->getLogger().debug("FD " + su::to_string(conn->getFd()) + " " + conn->debugString());
		Handlers::Response::prepareResponse(server, conn, ::Response::badRequest(conn));
		// closeConnection(conn);
		return false;
	}
	return true;
}
