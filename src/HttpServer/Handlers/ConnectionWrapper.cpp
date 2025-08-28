#include "src/HttpServer/Handlers/Handlers.hpp"

namespace ConnectionHandler {
void handleNewConnection(WebServer *server, ServerConfig *sc) { server->handleNewConnection(sc); }
Connection *addConnection(WebServer *server, int client_fd, ServerConfig *sc) { return server->addConnection(client_fd, sc); }
void cleanupExpiredConnections(WebServer *server) { server->cleanupExpiredConnections(); }
void handleConnectionTimeout(WebServer *server, int client_fd) { server->handleConnectionTimeout(client_fd); }
void closeConnection(WebServer *server, Connection *conn) { server->closeConnection(conn); }
}
