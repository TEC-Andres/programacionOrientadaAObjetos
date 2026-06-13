#ifndef ECCSERVICE_ECCEXCEPTION_H
#define ECCSERVICE_ECCEXCEPTION_H

#include <stdexcept>
#include <string>

namespace ecc {

class EccException : public std::runtime_error {
public:
    explicit EccException(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

} // namespace ecc

#endif // ECCSERVICE_ECCEXCEPTION_H
