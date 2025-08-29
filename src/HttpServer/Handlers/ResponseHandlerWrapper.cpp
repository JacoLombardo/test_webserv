#include "src/HttpServer/Handlers/Handlers.hpp"

namespace ResponseHandler {
ssize_t prepareResponse(WebServer *server, Connection *conn, const Response &resp) { return server->prepareResponse(conn, resp); }
bool sendResponse(WebServer *server, Connection *conn) { return server->sendResponse(conn); }
}

// EOF

namespace StaticGetRespHandler {
Response respDirectoryRequest(WebServer *server, Connection *conn, const std::string &fullDirPath) { return server->respDirectoryRequest(conn, fullDirPath); }
Response generateDirectoryListing(WebServer *server, Connection *conn, const std::string &fullDirPath) { return server->generateDirectoryListing(conn, fullDirPath); }
Response respFileRequest(WebServer *server, Connection *conn, const std::string &fullFilePath) { return server->respFileRequest(conn, fullFilePath); }
Response respReturnDirective(WebServer *server, Connection *conn, uint16_t code, std::string target) { return server->respReturnDirective(conn, code, target); }
void handleDirectoryRequest(WebServer *server, ClientRequest &req, Connection *conn, bool end_slash) { server->handleDirectoryRequest(req, conn, end_slash); }
void handleFileRequest(WebServer *server, ClientRequest &req, Connection *conn, bool end_slash) { server->handleFileRequest(req, conn, end_slash); }
}
