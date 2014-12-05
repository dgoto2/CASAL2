/*
 * Manager.h
 *
 *  Created on: 13/12/2012
 *      Author: Admin
 */

#ifndef TIMESTEPS_MANAGER_H_
#define TIMESTEPS_MANAGER_H_

// Headers
#include "BaseClasses/Manager.h"
#include "DerivedQuantities/Manager.h"
#include "TimeSteps/TimeStep.h"

// Namespaces
namespace niwa {
namespace timesteps {

/**
 * Class Definition
 */
class Manager : public niwa::oldbase::Manager<niwa::timesteps::Manager, niwa::TimeStep> {
  friend class niwa::oldbase::Manager<niwa::timesteps::Manager, niwa::TimeStep>;
public:
  // Methods
  virtual                     ~Manager() noexcept(true);
  void                        Validate() override final;
  void                        Build() override final;
  void                        Execute(unsigned year);
  void                        ExecuteInitialisation(const string& phase_label, unsigned years);
  TimeStepPtr                 GetTimeStep(const string& label) const;
  void                        RemoveAllObjects() override final { objects_.clear(); ordered_time_steps_.clear(); derived_quantities_.clear(); }
  unsigned                    GetTimeStepIndex(const string& time_step_label) const;
  unsigned                    GetTimeStepIndexForProcess(const string& process_label) const;
  unsigned                    GetTimeStepCountForProcess(const string& process_label) const;
  vector<unsigned>            GetTimeStepIndexesForProcess(const string& process_label) const;
  vector<ProcessType::Type>   GetOrderedProcessTypes();

  // accessors
  const vector<TimeStepPtr>&  time_steps() const { return ordered_time_steps_; }
  unsigned                    current_time_step() const { return current_time_step_; }

protected:
  // methods
  Manager();

private:
  // Members
  vector<TimeStepPtr>         ordered_time_steps_;
  vector<DerivedQuantityPtr>  derived_quantities_;
  unsigned                    current_time_step_ = 0;
};

} /* namespace timesteps */
} /* namespace niwa */
#endif /* TIMESTEPS_MANAGER_H_ */
