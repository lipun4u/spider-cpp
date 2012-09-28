#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
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

std::string createHeader(spider::Header const& header) {
    using std::back_inserter;
    using std::copy;
    using std::ostringstream;
    using std::string;
    using std::vector;
    using boost::join;

    ostringstream builder;
    builder << header.getName() << ": ";
    vector<string> values;
    copy(header.begin(), header.end(), back_inserter(values));
    builder << join(values, ";");
    return builder.str();
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
    using std::transform;
    using boost::asio::ip::tcp;
    using boost::shared_ptr;

    string const& host = m_url.getHost();
    string const& scheme = m_url.getScheme();

    shared_ptr<tcp::iostream> stream(new tcp::iostream());
    stream->connect(host, scheme);

    // TODO - check if we couldn't connect

    *stream << str(m_method) << " " << m_url.getPath();
    string const& query = m_url.getQuery();
    if (query != "") {
        *stream << "?" << query;
    }
    *stream << " HTTP/1.0" << HttpRequest::getNewline();
    ostream_iterator<string> destination(*stream, HttpRequest::getNewline().c_str());
    transform(m_headers.begin(), m_headers.end(), destination, createHeader);
    *stream << HttpRequest::getNewline();

    response_ptr response(new HttpResponse(stream));
    return response;
}

}
