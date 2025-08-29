/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocConfig.cpp                                      :+:      :+:    :+:   */
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
// LOCCONFIG
////////

std::string LocConfig::getPath() const { 
    return path; 
}

bool LocConfig::is_exact_() const { 
    return exact_match; 
}

void LocConfig::setExact(bool is_exact) {
    exact_match = is_exact; 
}

std::string LocConfig::getRoot() const { 
    return path;
}

std::string LocConfig::getFullPath() const { 
    return full_path; 
}

void LocConfig::setFullPath(const std::string &path) { 
    full_path = path; 
}

std::string LocConfig::getUploadPath() const { 
    return upload_path; 
}

size_t LocConfig::getMaxBodySize() const { 
    return client_max_body_size; 
}

bool LocConfig::infiniteBodySize() const { 
    return (client_max_body_size == 0) ? true : false; 
}

bool LocConfig::hasReturn() const { 
    return return_code != 0; 
}

bool LocConfig::hasMethod(const std::string &method) const {
    if (allowed_methods.empty())
        return true;
    for (std::vector<std::string>::const_iterator it = allowed_methods.begin();
         it != allowed_methods.end(); ++it) {
        if (*it == method)
            return true;
    }
    return false;
}

std::string LocConfig::getAllowedMethodsString() {
    std::string allowed;
    for (size_t i = 0; i < allowed_methods.size(); ++i) {
        allowed += allowed_methods[i];
        if (i != allowed_methods.size() - 1)
            allowed += ", ";
    }
    return allowed;
}

bool LocConfig::acceptExtension(const std::string &ext) const {
    if (cgi_extensions.empty())
        return false;
    std::map<std::string, std::string>::const_iterator it = cgi_extensions.find(ext);
    return (it != cgi_extensions.end());
}

std::string LocConfig::getInterpreter(const std::string &ext) const {
    std::map<std::string, std::string>::const_iterator it = cgi_extensions.find(ext);
    if (it != cgi_extensions.end())
        return it->second;
    else
        return "";
}

