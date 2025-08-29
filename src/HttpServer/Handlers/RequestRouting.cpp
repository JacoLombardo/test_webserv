/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestRouting.cpp                                 :+:      :+:    :+:   */
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

void WebServer::processRequest(Connection *conn) {
    _lggr.info("Processing request from fd: " + su::to_string(conn->fd));

    ClientRequest req = conn->parsed_request;

    // Handle body extraction differently for chunked vs non-chunked
    if (req.chunked_encoding) {
        // For chunked requests, use the reconstructed chunk data
        req.body = conn->chunk_data;
        _lggr.debug("Using chunked body data: " + su::to_string(req.body.length()) + " bytes");
    } else if (!req.chunked_encoding && conn->headers_buffer.size() <= conn->read_buffer.size()) {
        req.body = conn->read_buffer.substr(conn->headers_buffer.size());
    } else {
        _lggr.debug("No body data or headers not properly parsed");
        req.body = "";
    }

    _lggr.debug("req.body: " + req.body);
    _lggr.debug("req.headers: " + conn->headers_buffer);
    _lggr.debug("req.uri: " + req.uri);

    // For chunked requests: use of chunk_data length for content verification
    size_t actual_body_size = req.chunked_encoding ? conn->chunk_data.size() : req.body.size();

    _lggr.debug("[Resp] Payload vs content size: " + su::to_string(req.content_length) +
                ", payload size: " + su::to_string(actual_body_size));

    // Only verify content-length for non-chunked requests
    if (!req.chunked_encoding && req.content_length >= 0 &&
        static_cast<ssize_t>(actual_body_size) != req.content_length) {
        _lggr.error("[Resp] Payload mismatch, content size: " + su::to_string(req.content_length) +
                    ", payload size: " + su::to_string(actual_body_size));
        prepareResponse(conn, Response::contentTooLarge(conn));
        return;
    }

    _lggr.debug("FD " + su::to_string(req.clfd) + " ClientRequest {" + req.toString() + "}");
    // process the request
    processValidRequest(req, conn);
}

void WebServer::processValidRequest(ClientRequest &req, Connection *conn) {

    const std::string &full_path = conn->locConfig->getFullPath();
    _lggr.debug("[Resp] The matched location is an exact match: " +
                su::to_string(conn->locConfig->is_exact_()));

    // File system check
    FileType file_type = checkFileType(full_path);
    _lggr.debug("[Resp] checkFileType for " + full_path + " is " + fileTypeToString(file_type));

    if (file_type == NOT_FOUND_404 && su::back(full_path) == '/') {
        std::string pathWithoutSlash = full_path.substr(0, full_path.length() - 1);
        FileType fileTypeWithoutSlash = checkFileType(pathWithoutSlash);
        if (fileTypeWithoutSlash == ISREG) {
            file_type = ISREG;
        }
    }

    // File system errors
    if (!handleFileSystemErrors(file_type, full_path, conn))
        return;

    // we redirect if uri is missing the / (and vice versa), not the resolved path
    bool end_slash = (!req.uri.empty() && su::back(req.uri) == '/');
    // Route based on file type and request format
    if (file_type == ISDIR) {
        handleDirectoryRequest(req, conn, end_slash);
    } else if (file_type == ISREG) {
        handleFileRequest(req, conn, end_slash);
    } else {
        _lggr.error("Unexpected file type for: " + full_path);
        prepareResponse(conn, Response::internalServerError(conn));
    }
}

