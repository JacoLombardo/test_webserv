/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_StatusInit.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   Created: 2025/08/29 00:00:00 by refactor           #+#    #+#             */
/*   Updated: 2025/08/29 00:00:00 by refactor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "src/HttpServer/Structs/Connection.hpp"
#include "src/Utils/ServerUtils.hpp"

std::string Response::getReasonPhrase(uint16_t code) const {
    switch (code) {
    case 100:
        return "Continue";
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 204:
        return "No Content";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 304:
        return "Not Modified";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 408:
        return "Request Timeout";
    case 411:
        return "Length required";
    case 413:
        return "Content Too Large";
    case 414:
        return "URI Too Long";
    case 417:
        return "Expectation Failed";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "CGI Time-out";
    case 505:
        return "HTTP version not supported";
    default:
        return "Unknown Status";
    }
}

void Response::initFromCustomErrorPage(uint16_t code, Connection *conn) {
    reason_phrase = getReasonPhrase(code);
    if (!conn || !conn->getServerConfig() || !conn->getServerConfig()->hasErrorPage(code)) {
        initFromStatusCode(code);
        return;
    }
    std::string fullPath = conn->getServerConfig()->getErrorPage(code);
    std::ifstream errorFile(fullPath.c_str());
    if (!errorFile.is_open()) {
        initFromStatusCode(code);
        return;
    }
    std::ostringstream errorPage;
    errorPage << errorFile.rdbuf();
    body = errorPage.str();
    setContentLength(body.length());
    setContentType(detectContentType(fullPath));
    errorFile.close();
}

void Response::initFromStatusCode(uint16_t code) {
    reason_phrase = getReasonPhrase(code);
    if (code >= 400) {
        if (body.empty()) {
            std::ostringstream html;
            html << "<!DOCTYPE html>\n"
                 << "<html>\n"
                 << "<head>\n"
                 << "<title>" << code << " DX</title>\n"
                 << "<style>\n"
                 << "@import url('https://fonts.googleapis.com/"
                    "css2?family=Space+Mono:ital,wght@0,400;0,700;1,400;1,700&display=swap'"
                    ");\n"
                 << "body { font-family: \"Space Mono\", monospace; text-align: center; background-color: #f8f9fa; margin: 0; padding: 0; }\n"
                 << "h1 { color: #ff5555; margin-top: 50px; font-weight: 700; font-style: normal; }\n"
                 << "p { color: #6c757d; font-size: 18px; }"
                 << "footer { color: #dcdcdc; position: fixed; width: 100%; margin-top: 50px; }\n"
                 << "</style>\n"
                 << "</head>\n"
                 << "<body>\n"
                 << "<h1>Error " << code << ": " << reason_phrase << "</h1>\n"
                 << "<p>The server encountered an issue and could not complete your request.</p>\n"
                 << "<a href=\"https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status/"
                 << code << "\" target=\"_blank\" rel=\"noopener noreferrer\">MDN Web Docs - "
                 << code << "</a>"
                 << "<footer>" << __WEBSERV_VERSION__ << "</footer>"
                 << "</body>\n"
                 << "</html>\n";
            body = html.str();
            setContentLength(body.length());
            setContentType("text/html");
        }
    }
}

