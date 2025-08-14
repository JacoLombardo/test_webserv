/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCGI.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 11:38:44 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/08 14:17:13 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/Handlers/Handlers.hpp"
#include "src/CGI/CGI.hpp"

static void print_cgi_response(const std::string &cgi_output) {
	std::istringstream response_stream(cgi_output);
	std::string line;
	bool in_body = false;

	while (std::getline(response_stream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (!in_body && line.empty()) {
			in_body = true;
			std::cout << std::endl;
			continue;
		}

		std::cout << line << std::endl;
	}
}

void Handlers::CGIHandler::sendCGIResponse(WebServer *server, std::string &cgi_output, CGI *cgi, ::Connection *conn) {
	::Response resp;
	resp.setStatus(200);
	resp.version = "HTTP/1.1";
	resp.setContentType(cgi->extractContentType(cgi_output));

	size_t header_end = cgi_output.find("\n\n");
	if (header_end != std::string::npos) {
		// Found header separator, extract body after it
		resp.body = cgi_output.substr(header_end + 2); // +2 to skip "\n\n"
		resp.setContentLength(resp.body.length());
	} else {
		// No header separator found, treat entire output as body
		resp.body = cgi_output;
		resp.setContentLength(cgi_output.length());
	}

	std::string raw_response = resp.toString();
	conn->responseReady() = true;
	send(conn->getFd(), raw_response.c_str(), raw_response.length(), 0);
	cgi->cleanup();
	delete cgi;
	(void)server;
}

void Handlers::CGIHandler::chunkedResponse(WebServer *server, CGI *cgi, ::Connection *conn) {
	(void)cgi;
	(void)conn;
	(void)server;
}

void Handlers::CGIHandler::normalResponse(WebServer *server, CGI *cgi, ::Connection *conn) {
	Logger logger;
	std::string cgi_output;
	char buffer[4096];
	ssize_t bytes_read;

	while ((bytes_read = read(cgi->getOutputFd(), buffer, sizeof(buffer))) > 0) {
		cgi_output.append(buffer, bytes_read);
	}

	if (bytes_read == -1) {
		logger.logWithPrefix(Logger::ERROR, "CGI", "Error reading from CGI script");
		close(cgi->getOutputFd());
		waitpid(cgi->getPid(), NULL, 0);
		return;
	}
	print_cgi_response(cgi_output);
	sendCGIResponse(server, cgi_output, cgi, conn);
}

void Handlers::CGIHandler::handleCGIOutput(WebServer *server, int fd) {
	bool chunked = false;
	CGI *cgi = NULL;
	::Connection *conn = NULL;
	for (std::map<int, std::pair<CGI *, ::Connection *> >::iterator it = server->getCgiPool().begin();
	     it != server->getCgiPool().end(); ++it) {
		if (fd == it->first) {
			cgi = it->second.first;
			conn = it->second.second;
		}
	}
	if (chunked)
		chunkedResponse(server, cgi, conn);
	else
		normalResponse(server, cgi, conn);
}

bool Handlers::CGIHandler::isCGIFd(WebServer *server, int fd) { return (server->getCgiPool().find(fd) != server->getCgiPool().end()); }
