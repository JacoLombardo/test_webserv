#include "src/HttpServer/Handlers/Handlers.hpp"

namespace ServerCGIHandler {
uint16_t handleCGIRequest(WebServer *server, ClientRequest &req, Connection *conn) { return server->handleCGIRequest(req, conn); }
bool prepareCGIResponse(WebServer *server, CGI *cgi, Connection *conn) { return server->prepareCGIResponse(cgi, conn); }
void handleCGIOutput(WebServer *server, int fd) { server->handleCGIOutput(fd); }
bool isCGIFd(WebServer *server, int fd) { return server->isCGIFd(fd); }
}
