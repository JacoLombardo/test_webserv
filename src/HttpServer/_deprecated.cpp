

/* /// !!! DEPRECATED !!!
/// Updates the last activity time for a connection.
/// \deprecated Use Connection::updateActivity instead.
/// \param client_fd The client file descriptor.
void updateConnectionActivity(int client_fd);
void updateConnectionActivity(int client_fd) {
    std::map<int, Connection *>::iterator it = _connections.find(client_fd);
    if (it != _connections.end()) {
        it->second->last_activity = getCurrentTime();
        _lggr.debug("Updated activity for fd: " + su::to_string(client_fd));
    }
} */

/* /// !!! DEPRECATED !!!
/// Determines if a connection should be kept alive based on request headers.
/// \deprecated Logic (will be) moved to antoher place.
/// \param req The client request to analyze.
/// \returns True if connection should be kept alive, false otherwise.
/// !!! DEPRECATED !!!
/// Determines if a connection should be kept alive based on request headers.
/// \deprecated Logic (will be) moved to antoher place.
/// \param req The client request to analyze.
/// \returns True if connection should be kept alive, false otherwise.
bool shouldKeepAlive(const ClientRequest &req);
bool shouldKeepAlive(const ClientRequest &req) {
    // TODO: check if this could be improved using new & awesome Response struct
    std::map<int, Connection *>::iterator it = _connections.find(req.clfd);
    if (it == _connections.end()) {
        return false;
    }

    Connection *conn = it->second;

    if (conn->request_count >= MAX_KEEP_ALIVE_REQS) {
        _lggr.debug("Max keep-alive requests reached for fd: " + su::to_string(req.clfd));
        return false;
    }

    // Check for Connection: close header
    std::map<std::string, std::string>::const_iterator header_it = req.headers.find("connection");
    if (header_it != req.headers.end()) {
        _lggr.debug(" 123123 Found connection header for fd: " + su::to_string(req.clfd));
        std::string connection_value = header_it->second;
        std::transform(connection_value.begin(), connection_value.end(), connection_value.begin(),
                       ::tolower);

        if (connection_value == "close") {
            _lggr.debug("Closing connection for fd: " + su::to_string(req.clfd));
            return false;
        }
        if (connection_value == "keep-alive") {
            _lggr.debug("Keeping connection for fd: " + su::to_string(req.clfd) + " alive");
            return true;
        }
    }

    // Default keep-alive behavior for HTTP/1.1
    return req.version == "HTTP/1.1" || req.version == "HTTP/1.0";
} */

/* /// !!! DEPRECATED !!!
/// Retrieves a connection object by file descriptor.
/// \deprecated Direct map access is preferred.
/// \param client_fd The client file descriptor.
/// \returns Pointer to Connection object or nullptr if not found.
Connection *getConnection(int client_fd);
// DEPRECATED // to double check
Connection *WebServer::getConnection(int client_fd) {
    std::map<int, Connection *>::iterator conn_it = _connections.find(client_fd);
    if (conn_it == _connections.end()) {
        _lggr.error("No connection info found for fd: " + su::to_string(client_fd));
        close(client_fd);
        return NULL;
    }
    return conn_it->second;
} */

/* /// !!! DEPRECATED !!!
/// Checks if a connection has expired.
/// \deprecated Use Connection::isExpired instead.
/// \param conn The connection to check.
/// \returns True if expired, false otherwise.
bool isConnectionExpired(const Connection *conn) const;
bool WebServer::isConnectionExpired(const Connection *conn) const {
    time_t current_time = getCurrentTime();
    time_t timeout = CONNECTION_TO;
    return (current_time - conn->last_activity) > timeout;
} */

/* /// !!! DEPRECATED !!!
    /// Gets the current system time.
    /// \deprecated Use standard library functions instead.
    /// \returns Current time as time_t.
    time_t getCurrentTime() const;
inline time_t WebServer::getCurrentTime() const { return time(NULL); } */


