//
//  connector.hpp
//  Test
//
//  Created by Ryan Bartley on 6/17/16.
//
//

#pragma once

#if ! defined( ASIO_STANDALONE )
#define ASIO_STANDALONE 1
#endif

#include "url.hpp"
#include "asio_http_compat.hpp"
#include "asio/asio.hpp"

namespace cinder {
namespace http { namespace detail {
	
template<typename SessionType>
struct Connector : public std::enable_shared_from_this<Connector<SessionType>> {
	
	Connector( std::shared_ptr<SessionType> session, asio::ip::tcp::socket &socket )
	: mSession( session ), mSocket( socket ), mResolver( detail::http_io_context( socket ) ) {}
	
	void start();
	void start( asio::ip::tcp::endpoint endpoint );
	
private:
	void on_resolve( asio::error_code ec, asio::ip::tcp::resolver::results_type results );
	void on_connect_complete( asio::error_code ec );
	void on_connecting_to_endpoint( asio::error_code ec );

	std::shared_ptr<SessionType>	mSession;
	asio::ip::tcp::resolver			mResolver;
	asio::ip::tcp::socket			&mSocket;
};

template<typename SessionType>
void Connector<SessionType>::start()
{
	asio::error_code ec;
	// Fail if the socket is already open.
	if (mSocket.lowest_layer().is_open()) {
		ec = asio::error::already_open;
		detail::http_post( mSocket,
			std::bind( &SessionType::onError, mSession, ec ) );
		return;
	}
	
	mSocket.open(asio::ip::tcp::v4(), ec);
	if( ec ) {
		detail::http_post( mSocket,
			std::bind( &SessionType::onError, mSession, ec ) );
		return;
	}
	
	mResolver.async_resolve( mSession->mSessionUrl->host(),
							 std::to_string( mSession->mSessionUrl->port() ),
							 std::bind( &Connector<SessionType>::on_resolve,
										this->shared_from_this(),
										std::placeholders::_1,
										std::placeholders::_2 ) );
}
	
template<typename SessionType>
void Connector<SessionType>::start( asio::ip::tcp::endpoint endpoint )
{
	asio::error_code ec;
	// Fail if the socket is already open.
	if (mSocket.lowest_layer().is_open()) {
		ec = asio::error::already_open;
		detail::http_post( mSocket,
			std::bind( &SessionType::onError, mSession, ec ) );
		return;
	}
	
	mSocket.open(asio::ip::tcp::v4(), ec);
	if( ec ) {
		detail::http_post( mSocket,
			std::bind( &SessionType::onError, mSession, ec ) );
		return;
	}
	
	mSession->endpoint = endpoint;
	mSocket.async_connect( mSession->endpoint,
						  std::bind( &Connector<SessionType>::on_connecting_to_endpoint,
									this->shared_from_this(),
									std::placeholders::_1 ) );
}

template<typename SessionType>
void Connector<SessionType>::on_resolve( asio::error_code ec, asio::ip::tcp::resolver::results_type results )
{
	if( !ec ) {
		asio::async_connect( mSocket, results,
							   std::bind( &Connector<SessionType>::on_connect_complete,
										  this->shared_from_this(),
										  std::placeholders::_1 ) );
	}
	else
		detail::http_post( mSocket,
			std::bind( &SessionType::onError, mSession, ec ) );
}

template<typename SessionType>
void Connector<SessionType>::on_connect_complete( asio::error_code ec )
{
	if( !ec ) {
		if( !mSocket.is_open() ) {
			ec = asio::error::operation_aborted;
			detail::http_post( mSocket,
				std::bind( &SessionType::onError, mSession, ec ) );
			return;
		}
		
		mSocket.set_option( asio::ip::tcp::no_delay( true ), ec );
		detail::http_post( mSocket,
			std::bind( &SessionType::onOpen, mSession, ec ) );
	}
	else
		detail::http_post( mSocket,
			std::bind( &SessionType::onError, mSession, ec ) );
}
	
template<typename SessionType>
void Connector<SessionType>::on_connecting_to_endpoint( asio::error_code ec )
{
	if( !ec ) {
		if( !mSocket.is_open() ) {
			ec = asio::error::operation_aborted;
			detail::http_post( mSocket,
				std::bind( &SessionType::onError, mSession, ec ) );
			return;
		}
		
		mSocket.set_option( asio::ip::tcp::no_delay( true ), ec );
		detail::http_post( mSocket,
			std::bind( &SessionType::onOpen, mSession, ec ) );
	}
	else
		detail::http_post( mSocket,
			std::bind( &SessionType::onError, mSession, ec ) );
}
	
} // detail
} // http
} // cinder
