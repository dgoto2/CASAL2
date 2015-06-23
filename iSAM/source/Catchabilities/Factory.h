/**
 * @file Factory.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 12/03/2013
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 */
#ifndef CATCHABILITIES_FACTORY_H_
#define CATCHABILITIES_FACTORY_H_

// Headers
#include "BaseClasses/Factory.h"
#include "Catchabilities/Manager.h"

// Namespaces
namespace niwa {
namespace catchabilities {

// classes
class Factory {
public:
  // methods
  static CatchabilityPtr Create(const string& object_type, const string& sub_type);

private:
  Factory() = delete;
  ~Factory() = delete;
};

} /* namespace catchabilities */
} /* namespace niwa */
#endif /* CATCHABILITIES_FACTORY_H_ */
