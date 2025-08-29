/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIRequest.cpp                                     :+:      :+:    :+:   */
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

uint16_t WebServer::handleCGIRequest(ClientRequest &req, Connection *conn) {
    Logger _lggr;

    CGI *cgi = NULL;
    uint16_t exit_code = CGIUtils::createCGI(cgi, req, conn->locConfig);
    if (exit_code)
        return (exit_code);

    _cgi_pool[cgi->getOutputFd()] = std::make_pair(cgi, conn);
    if (!epollManage(EPOLL_CTL_ADD, cgi->getOutputFd(), EPOLLIN)) {
        _lggr.error("EPollManage for CGI request failed.");
        return (502);
    }

    return (0);
}

