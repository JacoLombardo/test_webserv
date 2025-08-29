/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_Factory1.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   Created: 2025/08/29 00:00:00 by refactor           #+#    #+#             */
/*   Updated: 2025/08/29 00:00:00 by refactor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response Response::continue_() { return Response(100); }
Response Response::ok(const std::string &body) { return Response(200, body); }
Response Response::badRequest() { return Response(400); }
Response Response::forbidden() { return Response(403); }
Response Response::notFound() { return Response(404); }

