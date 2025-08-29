/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_Factory2.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   Created: 2025/08/29 00:00:00 by refactor           #+#    #+#             */
/*   Updated: 2025/08/29 00:00:00 by refactor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response Response::methodNotAllowed(const std::string &allowed) {
    Response resp(405);
    if (!allowed.empty()) {
        resp.setHeader("Allow", allowed);
    }
    return resp;
}
Response Response::contentTooLarge() { return Response(413); }
Response Response::internalServerError() { return Response(500); }
Response Response::notImplemented() { return Response(501); }
Response Response::badGateway() { return Response(502); }

