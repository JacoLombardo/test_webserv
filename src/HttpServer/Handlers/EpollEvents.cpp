/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollEvents.cpp                                    :+:      :+:    :+:   */
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

void WebServer::processEpollEvents(const struct epoll_event *events, int event_count) {
    for (int i = 0; i < event_count; ++i) {
        const uint32_t event_mask = events[i].events;
        const int fd = events[i].data.fd;

        if (isListeningSocket(fd)) {
            ServerConfig *sc = ServerConfig::find(_confs, fd);
            handleNewConnection(sc);
        } else if (isCGIFd(fd)) {
            handleCGIOutput(fd);
        } else {
            handleClientEvent(fd, event_mask);
        }
    }
}

bool WebServer::isListeningSocket(int fd) const {
    for (std::vector<ServerConfig>::const_iterator it = _confs.begin(); it != _confs.end(); ++it) {
        if (fd == it->getServerFD()) {
            return true;
        }
    }
    return false;
}

