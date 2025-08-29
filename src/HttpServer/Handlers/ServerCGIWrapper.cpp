/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCGIWrapper.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/29 09:28:13 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/29 09:28:14 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/Handlers/Handlers.hpp"

namespace ServerCGIHandler {
uint16_t handleCGIRequest(WebServer *server, ClientRequest &req, Connection *conn) { return server->handleCGIRequest(req, conn); }
bool prepareCGIResponse(WebServer *server, CGI *cgi, Connection *conn) { return server->prepareCGIResponse(cgi, conn); }
void handleCGIOutput(WebServer *server, int fd) { server->handleCGIOutput(fd); }
bool isCGIFd(WebServer *server, int fd) { return server->isCGIFd(fd); }
}
