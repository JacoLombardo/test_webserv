/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: refactor                                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 00:00:00 by refactor          #+#    #+#             */
/*   Updated: 2025/08/28 00:00:00 by refactor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Webserv.hpp"
#include "src/ConfigParser/Structs/Struct.hpp"
#include "src/Logger/Logger.hpp"

/////////////////////////
// ServerConfig
////////

// GETTERS

const std::string &ServerConfig::getHost() const { 
    return host; 
}

int ServerConfig::getPort() const { 
    return port; 
}

int ServerConfig::getServerFD() const { 
    return server_fd; 
}

const std::string &ServerConfig::getPrefix() const { 
    return prefix_; 
}

void ServerConfig::setServerFD(int fd) { 
    server_fd = fd; 
}

void ServerConfig::setPrefix(const std::string& prefix) { 
    prefix_ = prefix; 
}

const std::map<uint16_t, std::string> &ServerConfig::getErrorPages() const { 
    return error_pages; 
}

bool ServerConfig::hasErrorPage(uint16_t status) const { 
    return error_pages.find(status) != error_pages.end();
}

std::vector<LocConfig> &ServerConfig::getLocations() { 
    return locations; 
}

std::string ServerConfig::getErrorPage(uint16_t status) const {
    std::map<uint16_t, std::string>::const_iterator it = error_pages.find(status);
    return (it != error_pages.end()) ? it->second : "";
}

// The default location
LocConfig *ServerConfig::defaultLocation() {
    for (std::vector<LocConfig>::iterator it = locations.begin(); it != locations.end(); ++it) {
        if (it->getPath() == "/") {
            return &(*it);
        }
    }
    return NULL;
}

// Find by server_fd
ServerConfig *ServerConfig::find(std::vector<ServerConfig> &servers, int server_fd) {
    for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->getServerFD() == server_fd) {
            return &(*it);
        }
    }
    return NULL;
}

// Find by host and port
ServerConfig *ServerConfig::find(std::vector<ServerConfig> &servers, const std::string &host,
                                 int port) {
    for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->host == host && it->port == port) {
            return &(*it);
        }
    }
    return NULL;
}

// Const versions for const containers
const ServerConfig *ServerConfig::find(const std::vector<ServerConfig> &servers, int server_fd) {
    for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end();
         ++it) {
        if (it->getServerFD() == server_fd) {
            return &(*it);
        }
    }
    return NULL;
}

const ServerConfig *ServerConfig::find(const std::vector<ServerConfig> &servers,
                                       const std::string &host, int port) {
    for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end();
         ++it) {
        if (it->host == host && it->port == port) {
            return &(*it);
        }
    }
    return NULL;
}

