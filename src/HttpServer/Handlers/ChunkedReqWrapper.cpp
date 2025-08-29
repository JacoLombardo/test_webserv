/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkedReqWrapper.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/29 09:28:23 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/29 09:28:24 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/Handlers/Handlers.hpp"

namespace ChunkedReqHandler {
bool processChunkSize(WebServer *server, Connection *conn) { return server->processChunkSize(conn); }
bool processChunkData(WebServer *server, Connection *conn) { return server->processChunkData(conn); }
bool processTrailer(WebServer *server, Connection *conn) { return server->processTrailer(conn); }
void reconstructChunkedRequest(WebServer *server, Connection *conn) { server->reconstructChunkedRequest(conn); }
}
