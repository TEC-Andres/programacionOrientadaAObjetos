#ifndef ECCSERVICE_ECCSERVICEFACTORY_H
#define ECCSERVICE_ECCSERVICEFACTORY_H

#include <memory>
#include "IEccService.h"

namespace ecc {

std::unique_ptr<IEccService> createEccService();

} // namespace ecc

#endif // ECCSERVICE_ECCSERVICEFACTORY_H
