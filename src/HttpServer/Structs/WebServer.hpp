/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 13:44:09 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/08 14:29:50 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER2_HPP
#define WEBSERVER2_HPP

#include "Connection.hpp"
#include "Response.hpp"
#include "src/HttpServer/HttpServer.hpp"
#include "src/Logger/Logger.hpp"
#include "includes/Types.hpp"

class ServerConfig; // Still needed to break potential circular dependencies
class Connection;
class CGI;

/// HTTP web server implementation using epoll for event-driven I/O.
///
/// This class implements a multi-server HTTP web server that can handle
/// multiple server configurations, persistent connections, chunked transfer
/// encoding, and provides basic HTTP request/response functionality.
class WebServer {

  public:
	/// Constructs a WebServer with the given server configurations.
	/// \param confs Vector of server configurations to initialize.
	WebServer(std::vector<ServerConfig> &confs);

	/// !!! DEPRECATED !!!
	/// Constructs a WebServer with configurations and a root path prefix.
	/// \param confs Vector of server configurations to initialize.
	/// \param prefix_path Root directory prefix for serving files.
	WebServer(std::vector<ServerConfig> &confs, std::string &prefix_path);

	~WebServer();

	/// Initializes all server sockets and prepares for accepting connections.
	/// \returns True on successful initialization, false otherwise.
	bool initialize();

	/// Starts the main event loop to handle client connections and requests.
	void run();

	/// Global flag indicating if the server should continue running.
	static bool _running;

	// Accessors and helper wrappers for handlers
	Logger &getLogger();
	int getEpollFd() const;
	const std::vector<ServerConfig> &getConfigs() const;
	std::vector<ServerConfig> &getConfigs();
	std::map<int, Connection *> &getConnections();
	std::map<int, std::pair<CGI *, Connection *> > &getCgiPool();
	time_t getLastCleanup() const;
	void setLastCleanup(time_t t);

	bool epollCtl(int op, int socket_fd, uint32_t events);
	bool setFdNonBlocking(int fd);
	time_t now() const;
	std::string readFileContent(std::string path);
	FileType getFileType(std::string path);
	std::string buildLocationFullPath(const std::string &uri, LocConfig *Location);

	int getConnectionTimeout() const;
	int getCleanupInterval() const;
	int getBufferSize() const;

  private:
	int _epoll_fd;
	int _backlog;
	std::string _root_prefix_path;

	std::vector<ServerConfig> _confs;
	std::vector<ServerConfig> _have_pending_conn;

	static const int CONNECTION_TO = 30;   // seconds
	static const int CLEANUP_INTERVAL = 5; // seconds
	static const int BUFFER_SIZE = 4096 * 3;

	Logger _lggr;
	static std::map<uint16_t, std::string> err_messages;

	/// @brief List of all CGI Objects
	std::map<int, std::pair<CGI *, Connection *> > _cgi_pool;

	// Connection management arguments
	std::map<int, Connection *> _connections;
	time_t _last_cleanup;

	// MEMBER FUNCTIONS

	/* HttpServer.cpp */

	/// Sets up signal handlers for graceful server shutdown.
	/// \returns True on success, false on failure.
	bool setupSignalHandlers();

	/// Creates and configures the main epoll instance.
	/// \returns True on success, false on failure.
	bool createEpollInstance();

	/// Resolves network address information for a server configuration.
	/// \param config The server configuration containing host/port information.
	/// \param result Pointer to store the resolved address information.
	/// \returns True on successful resolution, false otherwise.
	bool resolveAddress(const ServerConfig &config, struct addrinfo **result);

	/// Creates a socket and configures it with appropriate options.
	/// \param config Server configuration to create socket for.
	/// \param addr_info Resolved address information for the socket.
	/// \returns True on successful creation and configuration, false otherwise.
	bool createAndConfigureSocket(ServerConfig &config, const struct addrinfo *addr_info);

	/// Sets SO_REUSEADDR socket option to allow address reuse.
	/// \param socket_fd The socket file descriptor to configure.
	/// \param host The host address for logging purposes.
	/// \param port The port number for logging purposes.
	/// \returns True on success, false on failure.
	bool setSocketOptions(int socket_fd, const std::string &host, const int port);

	/// Sets a file descriptor to non-blocking mode.
	/// \deprecated This function is no longer used.
	/// \param fd The file descriptor to set as non-blocking.
	/// \returns True on success, false on failure.
	bool setNonBlocking(int fd);

	/// Binds socket to address and starts listening for connections.
	/// \param config Server configuration containing socket information.
	/// \param addr_info Address information to bind to.
	/// \returns True on successful bind and listen, false otherwise.
	bool bindAndListen(const ServerConfig &config, const struct addrinfo *addr_info);

	/// Manages epoll events for file descriptors.
	/// \param op The epoll operation (EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL).
	/// \param socket_fd The file descriptor to manage.
	/// \param events The epoll events to set (e.g. EPOLLIN | EPOLLOUT).
	/// \returns True on success, false on failure.
	bool epollManage(int op, int socket_fd, uint32_t events);

	/// Initializes a single server configuration.
	/// \param config The server configuration to initialize.
	/// \returns True on successful initialization, false otherwise.
	bool initializeSingleServer(ServerConfig &config);

	/// Performs cleanup of all server resources and connectioqns.
	void cleanup();

	/* ServerUtils.cpp */

	/// Gets the current system time.
	/// \deprecated Use standard library functions instead.
	/// \returns Current time as time_t.
	time_t getCurrentTime() const;

	/// Reads file content from filesystem.
	/// \param path The filesystem path to the file.
	/// \returns File content as string, or empty string on error.
	std::string getFileContent(std::string path);
	FileType checkFileType(std::string path);
	std::string buildFullPath(const std::string &uri, LocConfig *Location);
};

#endif
