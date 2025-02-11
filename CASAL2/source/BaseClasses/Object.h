/**
 * @file BaseObject.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 18/09/2012
 * @section LICENSE
 *
 * Copyright NIWA Science �2012 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * This class is the highest level class in our object tree. All objects
 * that have to be created from a configuration block need to inherit from this class.
 *
 * This class creates access to some central global objects within the system
 * including: GlobalConfiguration, TheModel, ParameterList etc
 *
 */
#ifndef BASE_OBJECT_H_
#define BASE_OBJECT_H_

// Headers
#include <memory>
#include <string>
#include <string_view>

#include "../Logging/Logging.h"
#include "../ParameterList/ParameterList.h"
#include "../Translations/Translations.h"
#include "../Utilities/Map.h"
#include "../Utilities/NoCopy.h"
#include "../Utilities/Types.h"

using niwa::utilities::Double;

// Namespaces
namespace niwa {
// Enumerated Types
namespace addressable {
enum Type { kInvalid = 0, kSingle = 1, kMultiple = 2, kVector = 3, kStringMap = 4, kUnsignedMap = 5, kVectorStringMap = 6 };

enum Usage {
  kNone           = 0,
  kLookup         = 1,  // Assert, Additional Prior, Equation, Reports
  kEstimate       = 2,
  kInputRun       = 4,
  kProfile        = 8,
  kProject        = 16,
  kSimulate       = 32,
  kSingleStep     = 64,
  kTimeVarying    = 128,
  kTransformation = 256,
  kAll            = 511
};
};  // namespace addressable

using std::map;
using std::shared_ptr;
using std::string;
using std::string_view;
using std::vector;
using utilities::OrderedMap;

namespace base {

// classes
class Object {
public:
  // Methods
  Object() = default;
  virtual ~Object(){};
  bool                        HasAddressable(const string& label) const;
  bool                        HasAddressableUsage(const string& label, const addressable::Usage&) const;
  void                        SetAddressableIsUsed(const string& label, const addressable::Usage&);
  bool                        IsAddressableUsedFor(const string& label, const addressable::Usage&);
  bool                        IsAddressableAVector(const string& label) const;
  unsigned                    GetAddressableSize(const string& label) const;
  Double*                     GetAddressable(const string& label);
  virtual Double*             GetAddressable(const string& label, const string& index);
  vector<Double*>*            GetAddressables(const string& absolute_label, const vector<string> indexes);
  map<unsigned, Double>*      GetAddressableUMap(const string& label);
  map<unsigned, Double>*      GetAddressableUMap(const string& label, bool& create_missing);
  OrderedMap<string, Double>* GetAddressableSMap(const string& label);
  vector<Double>*             GetAddressableVector(const string& label);
  addressable::Type           GetAddressableType(const string& label) const;
  void                        PrintParameterQueryInfo();
  virtual void                RebuildCache();
  void                        SubscribeToRebuildCache(Object* subscriber);
  void                        NotifySubscribers();

  // pure virtual methods
  virtual void Reset() = 0;

  // Accessors and Mutators
  string         label() const { return label_; }
  string         type() const { return utilities::ToLowercase(type_); }  // should always return lower case.
  ParameterList& parameters() { return parameters_; }
  string         location();
  bool           is_estimated() { return is_estimated_; }
  void           set_block_type(string value) {
    block_type_ = value;
    parameters_.set_parent_block_type(value);
  }
  void   set_label(string value) { label_ = value; }
  void   set_defined_file_name(string value) { parameters_.set_defined_file_name(value); }
  void   set_defined_line_number(unsigned value) { parameters_.set_defined_line_number(value); }
  void   set_estimated(bool value) { is_estimated_ = value; }        // This should only be used in Estimate/Creator/EstimateTransformations
  void   set_time_varying(bool value) { is_time_varying_ = value; }  // This should only be TimeVarying.cpp
  string block_type() const { return block_type_; }

  // Operators
  bool operator==(string_view label) const { return label == label_; }

protected:
  // Methods
  void RegisterAsAddressable(const string& label, Double* variable, addressable::Usage usage = addressable::kAll);
  void RegisterAsAddressable(const string& label, vector<Double>* variables, addressable::Usage usage = addressable::kAll);
  void RegisterAsAddressable(const string& label, OrderedMap<string, Double>* variables, addressable::Usage usage = addressable::kAll);
  void RegisterAsAddressable(const string& label, map<unsigned, Double>* variables, addressable::Usage usage = addressable::kAll);
  void RegisterAsAddressable(map<string, vector<Double>>* variables);

  // Members
  string                          block_type_      = "";
  string                          label_           = "";
  string                          type_            = "";
  bool                            is_time_varying_ = false;
  bool                            is_estimated_    = false;
  ParameterList                   parameters_;
  map<string, Double*>            addressables_;
  map<string, bool>               create_missing_addressables_;
  map<string, vector<Double>*>    addressable_vectors_;
  map<string, vector<Double*>>    addressable_custom_vectors_;
  map<string, addressable::Type>  addressable_types_;
  map<string, addressable::Usage> allowed_addressable_usage_;
  map<string, addressable::Usage> actual_addressable_usage_;
  vector<Object*>                 rebuild_cache_subscribers_;

  map<string, map<unsigned, Double>*>      addressable_u_maps_;
  map<string, OrderedMap<string, Double>*> addressable_s_maps_;
  vector<map<string, vector<Double>>*>     unnamed_addressable_s_map_vector_;

  DISALLOW_COPY_MOVE_AND_ASSIGN(Object);
};

} /* namespace base */
} /* namespace niwa */
#endif /* BASE_OBJECT_H_ */
