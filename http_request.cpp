#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>
#include <boost/asio.hpp>
#include "header.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "url.hpp"

namespace {

    std::string const& str(spider::RequestMethod method) {
        using std::string;

        static const string getString = "GET";
        static const string postString = "POST";
        static const string putString = "PUT";
        static const string deleteString = "DELETE";
        static const string headString = "HEAD";
        static const string traceString = "TRACE";
        static const string connectString = "CONNECT";

        switch (method) {
            case spider::GET:
            default:
                return getString;
            case spider::POST:
                return postString;
            case spider::PUT:
                return putString;
            case spider::DELETE:
                return deleteString;
            case spider::HEAD:
                return headString;
            case spider::TRACE:
                return traceString;
            case spider::CONNECT:
                return connectString;
        }
    }

}

namespace spider {

    std::string const& HttpRequest::getNewline() {
         static const std::string newline = "\r\n";
         return newline;
    }

    HttpRequest::HttpRequest(RequestMethod method, Url const& url)
        : m_method(method), m_url(url) {
    }

    HeaderCollection & HttpRequest::getHeaders() {
        return m_headers;
    }

    HttpRequest::response_ptr HttpRequest::getResponse() const {
        using std::ostream_iterator;
        using std::string;
        using boost::asio::ip::tcp;
        using boost::lexical_cast;
        using boost::shared_ptr;

        shared_ptr<tcp::iostream> tcpStream(new tcp::iostream());
        //tcpStream->expires_from_now(boost::posix_time::seconds(60));
        if (m_url.getPort() == Url::getDefaultPort()) {
            tcpStream->connect(m_url.getHost(), m_url.getScheme());
        } else {
            tcpStream->connect(
                m_url.getHost(), lexical_cast<string>(m_url.getPort()));
        }

        if (!*tcpStream) {
            throw ConnectionException(m_url);
        }

        *tcpStream << str(m_method) << " " << m_url.getPath();

        string const& query = m_url.getQuery();
        if (query != "") {
            *tcpStream << "?" << query;
        }
        *tcpStream << " HTTP/1.1" << HttpRequest::getNewline();

        ostream_iterator<Header> destination(
            *tcpStream, HttpRequest::getNewline().c_str());
        m_headers.getHeaders(destination);

        *tcpStream << HttpRequest::getNewline();
        tcpStream->flush();

        response_ptr response(new HttpResponse(tcpStream));
        return response;
    }

    ConnectionException::ConnectionException(Url const& url) throw() {
        using std::ostringstream;

        ostringstream builder;
        builder << "Failed to connect to " << url;
        m_what = builder.str();
    }

    ConnectionException::~ConnectionException() throw() {
    }

    char const* ConnectionException::what() const throw() {
        return m_what.c_str();
    }

}
