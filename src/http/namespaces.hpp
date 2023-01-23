#ifndef NAMESPACES_HPP
#define NAMESPACES_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace webvirt
{

namespace net
{

using unix = boost::asio::local::stream_protocol;
using tcp = boost::asio::ip::tcp;

}; // namespace net

namespace beast = boost::beast;

namespace http
{

enum version : int {
    http_1 = 10,
    http_1_1 = 11,
};

};

}; // namespace webvirt

#endif