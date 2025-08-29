/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestLifecycle.cpp                               :+:      :+:    :+:   */
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

bool WebServer::handleCompleteRequest(Connection *conn) {
    processRequest(conn);

    _lggr.debug("Request was processed. Read buffer will be cleaned");
    conn->read_buffer.clear();
    conn->request_count++;
    conn->updateActivity();
    return true;
}

bool WebServer::isRequestComplete(Connection *conn) {

    switch (conn->state) {

    case Connection::READING_HEADERS:
        _lggr.debug("isRequestComplete->READING_HEADERS");
        return isHeadersComplete(conn);

    case Connection::READING_BODY:
        _lggr.debug("isRequestComplete->READING_BODY");
        _lggr.debug(
            su::to_string(conn->content_length - static_cast<ssize_t>(conn->body_data.size())) +
            " bytes left to receive");

        if (static_cast<ssize_t>(conn->body_data.size()) == conn->content_length) {
            _lggr.debug("Read full content-length: " + su::to_string(conn->body_data.size()) +
                        " bytes received");
            conn->state = Connection::REQUEST_COMPLETE;
            reconstructRequest(conn);
            return true;
        }
        return false;

    case Connection::READING_CHUNK_SIZE:
        _lggr.debug("isRequestComplete->READING_CHUNK_SIZE");
        return processChunkSize(conn);

    case Connection::READING_CHUNK_DATA:
        _lggr.debug("isRequestComplete->READING_CHUNK_DATA");
        return processChunkData(conn);

    case Connection::READING_TRAILER:
        _lggr.debug("isRequestComplete->READING_TRAILER");
        return processTrailer(conn);

    case Connection::REQUEST_COMPLETE:
    case Connection::CHUNK_COMPLETE:
        _lggr.debug("isRequestComplete->REQUEST_COMPLETE");
        return true;

    default:
        _lggr.debug("isRequestComplete->default");
        return false;
    }
}

