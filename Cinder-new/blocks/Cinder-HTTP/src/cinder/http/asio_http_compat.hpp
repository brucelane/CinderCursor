#pragma once

#include "asio/asio.hpp"

namespace cinder {
namespace http {
namespace detail {

template<typename Socket, typename Handler>
inline void http_post( Socket &socket, Handler &&handler )
{
	asio::post( socket.get_executor(), std::forward<Handler>( handler ) );
}

template<typename Socket>
inline asio::io_context &http_io_context( Socket &socket )
{
	return static_cast<asio::io_context &>( socket.get_executor().context() );
}

} // namespace detail
} // namespace http
} // namespace cinder
