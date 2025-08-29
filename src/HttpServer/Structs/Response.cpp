/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: htharrau <htharrau@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 13:35:52 by jalombar          #+#    #+#             */
/*   Updated: 2025/08/25 19:10:17 by htharrau         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "src/ConfigParser/Structs/Struct.hpp"
#include "src/HttpServer/HttpServer.hpp"
#include "src/HttpServer/Structs/Connection.hpp"
#include "src/Utils/ServerUtils.hpp"

// Implementations moved to multiple Response_*.cpp files to keep <=5 functions per file.
