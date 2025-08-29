/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_FactoryOverloads.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   Created: 2025/08/29 00:00:00 by refactor           #+#    #+#             */
/*   Updated: 2025/08/29 00:00:00 by refactor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "src/HttpServer/Structs/Connection.hpp"

Response Response::badRequest(Connection *conn) { return Response(400, conn); }
Response Response::forbidden(Connection *conn) { return Response(403, conn); }
Response Response::notFound(Connection *conn) { return Response(404, conn); }
Response Response::methodNotAllowed(Connection *conn, const std::string &allowed) {
    Response resp(405, conn);
    if (!allowed.empty()) {
        resp.setHeader("Allow", allowed);
    }
    return resp;
}
Response Response::contentTooLarge(Connection *conn) { return Response(413, conn); }
Response Response::internalServerError(Connection *conn) { return Response(500, conn); }
Response Response::notImplemented(Connection *conn) { return Response(501, conn); }
Response Response::badGateway(Connection *conn) { return Response(502, conn); }
Response Response::gatewayTimeout(Connection *conn) { return Response(504, conn); }
Response Response::HttpNotSupported(Connection *conn) { return Response(505, conn); }

