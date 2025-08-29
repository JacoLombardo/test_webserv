/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_Constructors.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   Created: 2025/08/29 00:00:00 by refactor           #+#    #+#             */
/*   Updated: 2025/08/29 00:00:00 by refactor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "src/HttpServer/Structs/Connection.hpp"

Response::Response()
    : version("HTTP/1.1"),
      status_code(0),
      reason_phrase("Not Ready") {}

Response::Response(uint16_t code)
    : version("HTTP/1.1"),
      status_code(code) {
    initFromStatusCode(code);
}

Response::Response(uint16_t code, const std::string &response_body)
    : version("HTTP/1.1"),
      status_code(code),
      body(response_body) {
    initFromStatusCode(code);
}

Response::Response(uint16_t code, Connection *conn)
    : version("HTTP/1.1"),
      status_code(code) {
    initFromCustomErrorPage(code, conn);
}

