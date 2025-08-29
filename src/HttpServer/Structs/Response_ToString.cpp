/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_ToString.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   Created: 2025/08/29 00:00:00 by refactor           #+#    #+#             */
/*   Updated: 2025/08/29 00:00:00 by refactor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::string Response::toString() const {
    std::ostringstream response_stream;
    response_stream << version << " " << status_code << " " << reason_phrase << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        response_stream << it->first << ": " << it->second << "\r\n";
    }
    response_stream << "\r\n";
    response_stream << body;
    return response_stream.str();
}

std::string Response::toStringHeadersOnly() const {
    std::ostringstream response_stream;
    response_stream << version << " " << status_code << " " << reason_phrase << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        response_stream << it->first << ": " << it->second << "\r\n";
    }
    response_stream << "\r\n";
    return response_stream.str();
}

std::string Response::toShortString() const {
    std::ostringstream response_stream;
    response_stream << version << " " << status_code << " " << reason_phrase;
    if (headers.find("Content-Length") != headers.end()) {
        response_stream << " Content-Len.: " << headers.find("Content-Length")->second;
    }
    return response_stream.str();
}

void Response::reset() {
    status_code = 0;
    reason_phrase = "Not ready";
    headers.clear();
    body.clear();
}

