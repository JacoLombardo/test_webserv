#include "src/HttpServer/Handlers/Handlers.hpp"

namespace ChunkedReqHandler {
bool processChunkSize(WebServer *server, Connection *conn) { return server->processChunkSize(conn); }
bool processChunkData(WebServer *server, Connection *conn) { return server->processChunkData(conn); }
bool processTrailer(WebServer *server, Connection *conn) { return server->processTrailer(conn); }
void reconstructChunkedRequest(WebServer *server, Connection *conn) { server->reconstructChunkedRequest(conn); }
}
