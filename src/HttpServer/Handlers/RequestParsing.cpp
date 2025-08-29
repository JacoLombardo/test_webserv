/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParsing.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: refactor                                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/29 00:00:00 by refactor           #+#    #+#             */
/*   Updated: 2025/08/29 00:00:00 by refactor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/HttpServer.hpp"
#include "src/HttpServer/Structs/Connection.hpp"
#include "src/HttpServer/Structs/Response.hpp"
#include "src/HttpServer/Structs/WebServer.hpp"
#include "src/Utils/ServerUtils.hpp"

bool WebServer::isHeadersComplete(Connection *conn) {
    _lggr.debug("isHeadersComplete");
    std::string temp = conn->read_buffer;
    size_t header_end = conn->read_buffer.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        _lggr.debug("[HEADER CHECK] INCOMPLETE returning false");
        return false;
    }

    // Headers are complete
    std::string headers = conn->read_buffer.substr(0, header_end + 4);
    std::string remaining_data = conn->read_buffer.substr(header_end + 4);

    // Header request for early headers error detection
    ClientRequest req;
    req.clfd = conn->fd;

    // On error: REQUEST_COMPLETE, Prepare Response
    uint16_t error_code = RequestParsingUtils::parseRequestHeaders(headers, req, _lggr);
    _lggr.debug("[HEADER CHECK] Status post header request parsing : " + su::to_string(error_code));
    if (error_code != 0) {
        _lggr.logWithPrefix(Logger::ERROR, "BAD REQUEST", "Malformed or invalid headers");
        prepareResponse(conn, Response(error_code, conn));
        conn->state = Connection::REQUEST_COMPLETE;
        conn->should_close = true;
        return true;
    }

    // Match location block, Normalize URI + Check traversal
    if (!matchLocation(req, conn) || !normalizePath(req, conn)) {
        conn->state = Connection::REQUEST_COMPLETE;
        conn->should_close = true;
        return true;
    }
    // Return, Method, Max body
    if (!processValidRequestChecks(req, conn)) {
        conn->state = Connection::REQUEST_COMPLETE;
        conn->should_close = true;
        return true;
    }
    // Valid request headers - store parsed headers in connection
    conn->headers_buffer = headers;
    conn->parsed_request = req;
    conn->chunked = req.chunked_encoding;
    conn->content_length = req.content_length;

    if (!conn->chunked) { // Store remaining data as binary body data for Content-Length requests

        if (!remaining_data.empty() && conn->content_length > 0) {
            conn->body_data.insert(conn->body_data.end(),
                                   reinterpret_cast<const unsigned char *>(remaining_data.data()),
                                   reinterpret_cast<const unsigned char *>(remaining_data.data() +
                                                                           remaining_data.size()));
            conn->body_bytes_read = conn->body_data.size();
        }
        _lggr.debug("Request POST HEADER content length: " + su::to_string(conn->content_length));
        _lggr.debug("Request POST HEADER remaining data size: " +
                    su::to_string(remaining_data.size()));

        // ERROR handling if Body present when it should not
        if (conn->content_length <= 0 && conn->body_bytes_read != 0) {
            _lggr.error("Body present when it should not (400)");
            prepareResponse(conn, Response(400, conn));
            conn->should_close = true;
            conn->state = Connection::REQUEST_COMPLETE;
            return true;
        }

        // Case : content_length 0 or no content length)
        if (conn->content_length <= 0) {
            conn->state = Connection::REQUEST_COMPLETE;
            return true;
        }
        // Case : content_length specified
        else { // if (conn->content_length > 0)
            conn->state = Connection::READING_BODY;
            // check if full body
            if (static_cast<ssize_t>(conn->body_data.size()) == conn->content_length) {
                conn->state = Connection::REQUEST_COMPLETE;
                // req.body = reconstructRequest(conn);
                _lggr.debug("1 req.body" + req.body);
                return true;
            }
            if (static_cast<ssize_t>(conn->body_data.size()) > conn->content_length) {
                conn->state = Connection::REQUEST_COMPLETE;
                prepareResponse(conn, Response(400));
                _lggr.error("Content length mismatch: " + req.body);
                conn->should_close = true;
                return true;
            }
            // clear read_buffer since body data is in body_data vector
            conn->read_buffer.clear();
            return false;
        }
    }

    else { // CHUNKED - store remaining data as string
        conn->state = Connection::READING_CHUNK_SIZE;
        conn->read_buffer = remaining_data; // Keep any data after headers for chunk processing
        conn->chunk_size = 0;
        conn->chunk_bytes_read = 0;
        conn->chunk_data.clear();
        return processChunkSize(conn);
    }
    // Default
    _lggr.logWithPrefix(Logger::ERROR, "BAD REQUEST", "Impossible request");
    conn->state = Connection::REQUEST_COMPLETE;
    return true;
}

bool WebServer::reconstructRequest(Connection *conn) {
    std::string reconstructed_request;

    if (conn->headers_buffer.empty()) {
        _lggr.warn("Cannot reconstruct request: headers not available");
        return false;
    }

    reconstructed_request = conn->headers_buffer;

    if (conn->content_length > 0) {
        size_t body_size =
            std::min(static_cast<size_t>(conn->content_length), conn->body_data.size());

        reconstructed_request.append(reinterpret_cast<const char *>(&conn->body_data[0]),
                                     body_size);

        _lggr.debug("Reconstructed request with " + su::to_string(body_size) +
                    " bytes of body data");
    }

    conn->read_buffer = reconstructed_request;

    size_t headers_end = conn->headers_buffer.size();
    std::string debug_output =
        "Reconstructed request headers:\n" + conn->read_buffer.substr(0, headers_end);
    if (conn->content_length > 0) {
        debug_output += "\n[Binary body data: " + su::to_string(conn->body_data.size()) + " bytes]";
    }

    return true;
}

// Deprecated
bool WebServer::parseRequest(Connection *conn, ClientRequest &req) {
    _lggr.debug("Parsing request: " + conn->read_buffer);
    uint16_t error_code = RequestParsingUtils::parseRequest(conn->read_buffer, req, _lggr);
    _lggr.debug("Error code post request parsing : " + su::to_string(error_code));
    if (error_code != 0) {
        _lggr.error("Parsing of the request failed.");
        prepareResponse(conn, Response(error_code, conn));
        // closeConnection(conn);
        return false;
    }
    return true;
}

