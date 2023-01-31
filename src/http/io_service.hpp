/* SPDX-License-Identifier: Apache 2.0 */
#ifndef HTTP_IO_SERVICE_HPP
#define HTTP_IO_SERVICE_HPP

#include <boost/asio.hpp>

namespace webvirt
{

/**
 * A boost::asio::io_service wrapper
 **/
class io_service : public boost::asio::io_service
{
public:
    using boost::asio::io_service::io_service;
    virtual ~io_service() = default;

    virtual std::size_t process();
};

}; // namespace webvirt

#endif /* HTTP_IO_SERVICE_HPP */
