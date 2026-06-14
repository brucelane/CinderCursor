//
//  http.hpp
//  UrlTest
//
//  Created by Ryan Bartley on 6/1/16.
//
//

#pragma once

#if ! defined( ASIO_STANDALONE )
#define ASIO_STANDALONE 1
#endif
#define USING_SSL
#include "asio/asio.hpp"
#if defined( USING_SSL )
#include "asio/ssl.hpp"
#endif

#include "cinder/app/App.h"
#include "cinder/Log.h"

#include "url.hpp"
#include "connector.hpp"
#include "handshaker.hpp"
#include "requester.hpp"
#include "responder.hpp"
#include "request_response.hpp"

namespace cinder {
namespace http {

enum class protocol {
	http,
	https,
	file
};
	
using ResponseHandler = std::function<void( asio::error_code, ResponseRef )>;
using ErrorHandler = std::function<void( asio::error_code, const UrlRef &, ResponseRef )>;
	
using SessionRef = std::shared_ptr<class Session>;

class Session : public std::enable_shared_from_this<Session> {
public:
	
	Session( RequestRef request, ResponseHandler responseHandler, ErrorHandler errorHandler,
			 asio::io_context &io_context = ci::app::App::get()->io_context() )
	: io_context( io_context ), socket( io_context ), responseHandler( responseHandler ),
	errorHandler( errorHandler ), mSessionUrl( request->requestUrl ), request( request ) {}
	~Session() = default;
	
	asio::io_context&	get_io_context() { return io_context; }
	const UrlRef&		getUrl() const { return mSessionUrl; }
	UrlRef&				getUrl() { return mSessionUrl; }
	
	const asio::ip::tcp::endpoint&	getEndpoint() const { return endpoint; }
	asio::ip::tcp::endpoint&	getEndpoint() { return endpoint; }
	
	void start()
	{
		std::make_shared<detail::Connector<Session>>( 
				shared_from_this(), socket )->start();
	}
	
	void start( asio::ip::tcp::endpoint endpoint )
	{
		std::make_shared<detail::Connector<Session>>(
			shared_from_this(), socket )->start( endpoint );
	}
	
private:
	void onOpen( asio::error_code ec )
	{
		std::make_shared<detail::Handshaker<Session>>(
			shared_from_this() )->handshake();
	}
	void onHandshake( asio::error_code ec )
	{
		if( ! request )
			request = std::make_shared<Request>( RequestMethod::GET, mSessionUrl );
		std::make_shared<detail::Requester<Session>>(
			shared_from_this(), std::move( request ) )->request();
	}
	void onRequest( asio::error_code ec )
	{
		std::make_shared<detail::Responder<Session>>(
			shared_from_this() )->read();
	}
	void onResponse( asio::error_code ec )
	{
		responseHandler( ec, response );
	}
	
	void onError( asio::error_code ec ) 
	{
		errorHandler( ec, mSessionUrl, response );
	}
	
	asio::io_context	&io_context;
	asio::ip::tcp::socket	socket;
	
	ResponseHandler		responseHandler;
	ErrorHandler		errorHandler;
	RequestRef			request;
	ResponseRef			response;
	
	UrlRef					mSessionUrl;
	asio::ip::tcp::endpoint	endpoint;
	
	friend struct detail::Connector<Session>;
	friend struct detail::Handshaker<Session>;
	friend struct detail::Requester<Session>;
	friend struct detail::Responder<Session>;
};
	
#if defined( USING_SSL )
	
using SslSessionRef = std::shared_ptr<class SslSession>;

class SslSession : public std::enable_shared_from_this<SslSession> {
public:
	
	SslSession( RequestRef request, ResponseHandler responseHandler, ErrorHandler errorHandler,
			    asio::io_context &io_context = ci::app::App::get()->io_context() )
	: io_context( io_context ), context(asio::ssl::context::tlsv12_client),
	socket( io_context, context ), mSessionUrl( request->requestUrl ), responseHandler( responseHandler ),
	errorHandler( errorHandler ), request( request )
	{
		context.set_default_verify_paths();
		auto host = mSessionUrl->host();
		socket.set_verify_callback(asio::ssl::host_name_verification{host});
	}
	~SslSession() = default;
	
	asio::io_context&		get_io_context() { return io_context; }
	const UrlRef&			getUrl() const { return mSessionUrl; }
	UrlRef&					getUrl() { return mSessionUrl; }
	
	const asio::ip::tcp::endpoint&	getEndpoint() const { return endpoint; }
	asio::ip::tcp::endpoint&		getEndpoint() { return endpoint; }
	
	void start()
	{
		std::make_shared<detail::Connector<SslSession>>(
			shared_from_this(), socket.next_layer() )->start();
	}
	
	void start( asio::ip::tcp::endpoint endpoint )
	{
		std::make_shared<detail::Connector<SslSession>>(
			shared_from_this(), socket.next_layer() )->start( endpoint );
	}
	
private:
	void onOpen( asio::error_code ec )
	{
		std::make_shared<detail::Handshaker<SslSession>>(
			shared_from_this() )->handshake();
	}
	void onHandshake( asio::error_code ec )
	{
		if( ! request )
			request = std::make_shared<Request>( RequestMethod::GET, mSessionUrl );
		std::make_shared<detail::Requester<SslSession>>(
			shared_from_this(), std::move( request ) )->request();
	}
	void onRequest( asio::error_code ec )
	{
		std::make_shared<detail::Responder<SslSession>>(
			shared_from_this() )->read();
	}
	void onResponse( asio::error_code ec )
	{
		responseHandler( ec, response );
	}
	
	void onError( asio::error_code ec ) 
	{
		errorHandler( ec, mSessionUrl, response );
	}
	
	asio::io_context	&io_context;
	asio::ssl::context	context;
	asio::ssl::stream<asio::ip::tcp::socket> socket;
	
	ResponseHandler		responseHandler;
	ErrorHandler		errorHandler;
	RequestRef			request;
	ResponseRef			response;
	
	UrlRef					mSessionUrl;
	asio::ip::tcp::endpoint	endpoint;
	
	friend struct detail::Connector<SslSession>;
	friend struct detail::Handshaker<SslSession>;
	friend struct detail::Requester<SslSession>;
	friend struct detail::Responder<SslSession>;
};
	
#endif
	
}} // http // cinder
