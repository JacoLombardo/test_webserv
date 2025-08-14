/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkedReq.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jalombar <jalombar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 10:39:43 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/08 14:16:48 by jalombar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "src/HttpServer/Handlers/Handlers.hpp"

bool Handlers::Chunked::processChunkSize(WebServer *server, ::Connection *conn) {
	size_t crlf_pos = findCRLF(conn->readBuffer());
	if (crlf_pos == std::string::npos) {
		// Need more data to read chunk size
		return false;
	}

	std::string chunk_size_line = conn->readBuffer().substr(0, crlf_pos);

	conn->readBuffer() = conn->readBuffer().substr(crlf_pos + 2);

	// ignore chunk extensions after ';'
	size_t semicolon_pos = chunk_size_line.find(';');
	if (semicolon_pos != std::string::npos) {
		chunk_size_line = chunk_size_line.substr(0, semicolon_pos);
	}

	chunk_size_line = su::trim(chunk_size_line);

	// TODO: Check for negative?
	conn->chunkSize() = static_cast<size_t>(std::strtol(chunk_size_line.c_str(), NULL, 16));
	conn->chunkBytesRead() = 0;

	server->getLogger().debug("Chunk size: " + su::to_string(conn->chunkSize()));

	if (conn->chunkSize() == 0) {
		// Last chunk, read trailers
		conn->stateRef() = ::Connection::READING_TRAILER;
		return processTrailer(server, conn);
	} else {
		conn->stateRef() = ::Connection::READING_CHUNK_DATA;
		return processChunkData(server, conn);
	}
}

bool Handlers::Chunked::processChunkData(WebServer *server, ::Connection *conn) {
	size_t available_data = conn->readBuffer().length();
	size_t bytes_needed = conn->chunkSize() - conn->chunkBytesRead();

	if (available_data < bytes_needed + 2) { // +2 for trailing CRLF
		// Need more data
		return false;
	}

	size_t bytes_to_read = bytes_needed;
	std::string chunk_part = conn->readBuffer().substr(0, bytes_to_read);
	conn->chunkData() += chunk_part;
	conn->chunkBytesRead() += bytes_to_read;

	conn->readBuffer() = conn->readBuffer().substr(bytes_to_read);

	// Check if there are trailing CRLF
	if (conn->readBuffer().length() < 2) {
		return false;
	}

	if (conn->readBuffer().substr(0, 2) != "\r\n") {
		server->getLogger().error("Invalid chunk format: missing trailing CRLF");
		return false;
	}

	// Remove trailing CRLF
	conn->readBuffer() = conn->readBuffer().substr(2);

	conn->stateRef() = ::Connection::READING_CHUNK_SIZE;
	return processChunkSize(server, conn);
}

bool Handlers::Chunked::processTrailer(WebServer *server, ::Connection *conn) {
	size_t trailer_end = findCRLF(conn->readBuffer());

	if (trailer_end == std::string::npos) {
		// Need more data
		return false;
	}

	std::string trailer_line = conn->readBuffer().substr(0, trailer_end);
	conn->readBuffer() = conn->readBuffer().substr(trailer_end + 2);

	// If trailer line is empty, we're done
	if (trailer_line.empty()) {
		conn->stateRef() = ::Connection::CHUNK_COMPLETE;

		reconstructChunkedRequest(server, conn);
		return true;
	}

	return processTrailer(server, conn);
}

// TODO: add more debuggin info
void Handlers::Chunked::reconstructChunkedRequest(WebServer *server, ::Connection *conn) {
	std::string reconstructed_request = conn->headersBuffer();

	std::string headers_lower = su::to_lower(reconstructed_request);

	size_t te_pos = headers_lower.find("transfer-encoding: chunked");
	if (te_pos != std::string::npos) {
		// Find the end of this header line
		size_t line_end = reconstructed_request.find("\r\n", te_pos);
		if (line_end != std::string::npos) {
			// Remove the Transfer-Encoding line
			reconstructed_request.erase(te_pos, line_end - te_pos + 2);
		}
	}

	// Add Content-Length header before the final CRLF
	size_t final_crlf = reconstructed_request.find("\r\n\r\n");
	if (final_crlf != std::string::npos) {
		std::string content_length_header =
		    "\r\nContent-Length: " + su::to_string(conn->chunkData().length()) + "\r\n";
		reconstructed_request.insert(final_crlf, content_length_header);
	}

	conn->readBuffer() = reconstructed_request + conn->chunkData();
	conn->stateRef() = ::Connection::REQUEST_COMPLETE;

	server->getLogger().debug("Reconstructed chunked request, total body size: " +
	            su::to_string(conn->chunkData().length()));
}
