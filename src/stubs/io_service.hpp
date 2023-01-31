/* SPDX-License-Identifier: Apache 2.0 */
#ifndef STUBS_IO_SERVICE_HPP
#define STUBS_IO_SERVICE_HPP

#include "../http/io_service.hpp"

namespace webvirt::stubs
{

/**
 * A webvirt::io_service stub with a no-op process()
 **/
class io_service : public webvirt::io_service
{
private:
    std::size_t iterations = 0;

public:
    io_service(std::size_t iterations = 0);
    virtual ~io_service() = default;

    std::size_t process() override;
};

}; // namespace webvirt::stubs

#endif /* STUBS_IO_SERVICE_HPP */
