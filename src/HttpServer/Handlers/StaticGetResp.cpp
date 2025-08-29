/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticGetResp.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: htharrau <htharrau@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 18:29:33 by htharrau          #+#    #+#             */
/*   Updated: 2025/08/26 10:30:54 by htharrau         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/HttpServer.hpp"
#include "src/HttpServer/Structs/Connection.hpp"
#include "src/HttpServer/Structs/Response.hpp"
#include "src/HttpServer/Structs/WebServer.hpp"
#include "src/Utils/ServerUtils.hpp"

bool WebServer::handleFileSystemErrors(FileType file_type, const std::string &full_path,
                                       Connection *conn) {
	if (file_type == NOT_FOUND_404) {
		_lggr.error("[Resp] Could not open : " + full_path);
		prepareResponse(conn, Response::notFound(conn));
		return false;
	}
	if (file_type == PERMISSION_DENIED_403) {
		_lggr.error("[Resp] Permission denied : " + full_path);
		prepareResponse(conn, Response::forbidden(conn));
		return false;
	}
	if (file_type == FILE_SYSTEM_ERROR_500) {
		_lggr.error("[Resp] Other file access problem : " + full_path);
		prepareResponse(conn, Response::internalServerError(conn));
		return false;
	}
	return true;
}

void WebServer::handleDirectoryRequest(ClientRequest &req, Connection *conn, bool end_slash) {

	const std::string full_path = conn->locConfig->getFullPath();

	_lggr.debug("Directory request: " + full_path);

	if (!end_slash) {
		_lggr.info("Directory request without trailing slash, redirecting to : " + req.path + "/");
		std::string redirectPath = req.path + "/";
		prepareResponse(conn, respReturnDirective(conn, 301, redirectPath));
		return;
	} else {
		prepareResponse(conn, respDirectoryRequest(conn, full_path));
		return;
	}
}

void WebServer::handleFileRequest(ClientRequest &req, Connection *conn, bool end_slash) {

	const std::string full_path = conn->locConfig->getFullPath();
	_lggr.debug("File request: " + full_path);

	// Trailing '/'? Redirect
	if (end_slash) { //&& !conn->locConfig->is_exact_()
		_lggr.info("File request with trailing slash, redirecting: " + req.path);
		std::string redirectPath = req.path.substr(0, req.path.length() - 1);
		prepareResponse(conn, respReturnDirective(conn, 301, redirectPath));
		return;
	}
	
	// HANDLE CGI
	std::string extension = getExtension(full_path);
	if (conn->locConfig->acceptExtension(extension)) {
		std::string interpreter = conn->locConfig->getInterpreter(extension);
		_lggr.debug("CGI request, interpreter location : " + interpreter);
		req.extension = extension;
		uint16_t exit_code = handleCGIRequest(req, conn);
		if (exit_code) {
			_lggr.error("Handling the CGI request failed.");
			prepareResponse(conn, Response(exit_code, conn));
		}
		return;
	}

	// HANDLE STATIC GET RESPONSE
	if (req.method == "GET") {
		_lggr.debug("Static file GET request");
		prepareResponse(conn, respFileRequest(conn, full_path));
		return;
	} else {
		_lggr.error("Non-GET request for static file - not implemented");
		prepareResponse(
		    conn, Response::methodNotAllowed(conn, conn->locConfig->getAllowedMethodsString()));
		return;
	}
}

Response WebServer::generateDirectoryListing(Connection *conn, const std::string &fullDirPath) {
    _lggr.debug("Generating directory listing for: " + fullDirPath);

    // Open directory
    DIR *dir = opendir(fullDirPath.c_str());
    if (dir == NULL) {
        _lggr.error("Failed to open directory: " + fullDirPath + " - " +
                    std::string(strerror(errno)));
        return Response::notFound(conn);
    }

    // Generate HTML content
    std::ostringstream htmlContent;
    struct dirent *entry;
    htmlContent << "<!DOCTYPE html>\n"
                << "<html lang=\"en\">\n"
                << "<head>\n"
                << "<meta charset=\"UTF-8\">\n"
                << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                << "<title>Directory Listing - " << fullDirPath << "</title>\n"
                << "<link rel=\"stylesheet\" href=\"/styles.css\">\n"
                << "</head>\n<body>\n"
                << "<div class=\"container\">\n"
                << "<h1 class=\"title\">Directory Listing</h1>\n"
                << "<p class=\"subtitle\">" << fullDirPath << "</p>\n"
                << "<table>\n<tr><th>Name</th><th>Type</th><th>Size</th></tr>\n";

    while ((entry = readdir(dir)) != NULL) {
        std::string filename = entry->d_name;
        if (filename == ".")
            continue;

        std::string fullPath = fullDirPath;
        if (su::back(fullPath) != '/')
            fullPath += "/";
        fullPath += filename;

        struct stat fileStat;
        htmlContent << "<tr><td><a href=\"" << filename;
        if (stat(fullPath.c_str(), &fileStat) == 0) {
            if (S_ISDIR(fileStat.st_mode)) {
                htmlContent << "/\" class=\"dir\">";
            } else {
                htmlContent << "\" class=\"file\">";
            }
            htmlContent << filename << "</a></td><td>";
            if (S_ISDIR(fileStat.st_mode))
                htmlContent << "<span class=\"dir\">Directory</span>";
            else if (S_ISREG(fileStat.st_mode))
                htmlContent << "<span class=\"file\">File</span>";
            else
                htmlContent << "Other";

            htmlContent << "</td><td class=\"size\">";
            if (S_ISREG(fileStat.st_mode))
                htmlContent << su::to_string(fileStat.st_size);
            else
                htmlContent << "-";
            htmlContent << "</td></tr>\n";
        } else {
            htmlContent << "\">" << filename << "</a></td><td>Unknown</td><td>-</td></tr>\n";
        }
    }

    htmlContent << "</table>\n"
                << "<footer>Generated by WebServer " << __WEBSERV_VERSION__ << "</footer>\n"
                << "</div>\n"
                << "<div class=\"floating-elements\">\n"
                << "<div class=\"floating-element\"></div>\n"
                << "<div class=\"floating-element\"></div>\n"
                << "<div class=\"floating-element\"></div>\n"
                << "</div>\n"
                << "</body>\n</html>";

    closedir(dir);

    // Create response
    std::string body = htmlContent.str();
    Response resp(200, body);
    resp.setContentType("text/html");
    resp.setContentLength(body.length());

    _lggr.debug("Generated directory listing (" + su::to_string(body.length()) + " bytes)");
    return resp;
}
