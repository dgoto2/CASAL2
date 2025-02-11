/**
 * @file MortalityConstantRate.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 20/12/2012
 * @section LICENSE
 *
 * Copyright NIWA Science �2012 - www.niwa.co.nz
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "MortalityConstantRate.h"

#include <numeric>

#include "Categories/Categories.h"
#include "Selectivities/Manager.h"
#include "Selectivities/Selectivity.h"
#include "TimeSteps/Manager.h"

// Namespaces
namespace niwa {
namespace processes {
namespace age {

/**
 * Default Constructor
 */
MortalityConstantRate::MortalityConstantRate(shared_ptr<Model> model) : Mortality(model), partition_(model) {
  LOG_TRACE();
  parameters_.Bind<string>(PARAM_CATEGORIES, &category_labels_, "The list of category labels", "");
  parameters_.Bind<Double>(PARAM_M, &m_input_, "The mortality rates", "")->set_lower_bound(0.0);
  parameters_.Bind<double>(PARAM_TIME_STEP_PROPORTIONS, &ratios_, "The time step proportions for the mortality rates", "", true)->set_range(0.0, 1.0);
  parameters_.Bind<string>(PARAM_RELATIVE_M_BY_AGE, &selectivity_names_, "The list of mortality by age ogive labels for the categories", "");

  RegisterAsAddressable(PARAM_M, &m_);
}

/**
 * Validate the Mortality Constant Rate process
 *
 * - Validate the required parameters
 * - Assign the label from the parameters
 * - Assign and validate remaining parameters
 * - Duplicate 'm' and 'selectivities' if only one value specified
 * - Check m is between 0.0 and 1.0
 * - Check ratios sum to one
 * - Check the categories are real
 */
void MortalityConstantRate::DoValidate() {

  if (m_input_.size() == 1) {
    auto val_m = m_input_[0];
    m_input_.assign(category_labels_.size(), val_m);
  }

  if (selectivity_names_.size() == 1) {
    auto val_sel = selectivity_names_[0];
    selectivity_names_.assign(category_labels_.size(), val_sel);
  }

  if (m_input_.size() != category_labels_.size()) {
    LOG_ERROR_P(PARAM_M) << ": The number of Ms provided (" << m_input_.size() << ") does not match the number of categories provided (" << category_labels_.size() << ").";
  }

  if (selectivity_names_.size() != category_labels_.size()) {
    LOG_ERROR_P(PARAM_RELATIVE_M_BY_AGE) << ": The number of M-by-age ogives provided (" << selectivity_names_.size() << ") does not match the number of categories provided ("
                                         << category_labels_.size() << ").";
  }

  // Validate our Ms are greater than or equal to 0.0
  for (unsigned i = 0; i < m_input_.size(); ++i) m_[category_labels_[i]] = m_input_[i];

  // Check that the time step ratios sum to one
  // commented out as a nightmare to add due to unit-tests...
  /*
  Double total = 0.0;
  for (Double value : ratios_) {
    total += value;
  }
  if (!utilities::math::IsOne(total)) {
    LOG_ERROR_P(PARAM_TIME_STEP_PROPORTIONS) << " need to sum to one";
  }
  */

}

/**
 * Build any runtime relationships
 *
 * - Build the partition accessor
 * - Build the list of selectivities
 * - Build the ratios for the number of time steps
 */
void MortalityConstantRate::DoBuild() {
  partition_.Init(category_labels_);

  for (string label : selectivity_names_) {
    Selectivity* selectivity = model_->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_RELATIVE_M_BY_AGE) << ": M-by-age ogive label " << label << " was not found.";
    selectivities_.push_back(selectivity);
  }

  /**
   * Organise the time step ratios. Each time step can
   * apply a different ratio of M so here verify that
   * there are enough and re-scale them to 1.0
   */
  vector<TimeStep*> time_steps = model_->managers()->time_step()->ordered_time_steps();
  LOG_FINEST() << "time_steps.size(): " << time_steps.size();
  vector<unsigned> active_time_steps;
  for (unsigned i = 0; i < time_steps.size(); ++i) {
    if (time_steps[i]->HasProcess(label_))
      active_time_steps.push_back(i);
  }

  if (ratios_.size() == 0) {
    for (unsigned i : active_time_steps) time_step_ratios_[i] = 1.0;
  } else {
    if (ratios_.size() != active_time_steps.size())
      LOG_ERROR_P(PARAM_TIME_STEP_PROPORTIONS) << " The number of time step proportions (" << ratios_.size()
                                               << ") does not match the number of time steps this process has been assigned to (" << active_time_steps.size() << ")";

    for (double value : ratios_) {
      if (value < 0.0 || value > 1.0)
        LOG_ERROR_P(PARAM_TIME_STEP_PROPORTIONS) << "Time step proportion values (" << value << ") must be between 0.0 and 1.0 (inclusive) and sum to one";
    }

    for (unsigned i = 0; i < ratios_.size(); ++i) time_step_ratios_[active_time_steps[i]] = ratios_[i];
  }

  // Pre allocate memory to reporting containers, this process is run every year so the beauty of this is we can push back and it wont be
  // dealing with memory allocation during the execute
  unsigned n_years = model_->years().size();
  total_removals_by_year_.reserve(n_years);
}

/**
 * Execute the process
 */
void MortalityConstantRate::DoExecute() {
  LOG_FINEST() << "year: " << model_->current_year();

  // get the ratio to apply first
  unsigned time_step = model_->managers()->time_step()->current_time_step();

  LOG_FINEST() << "Ratios.size() " << time_step_ratios_.size() << " : time_step: " << time_step << "; ratio: " << time_step_ratios_[time_step];
  double ratio = time_step_ratios_[time_step];

  unsigned i = 0;
  Double   amount;
  Double   total_amount = 0.0;
  for (auto category : partition_) {
    Double m = m_[category->name_];

    unsigned j = 0;

    LOG_FINEST() << "category " << category->name_ << "; min_age: " << category->min_age_ << "; ratio: " << ratio;
    for (Double& data : category->data_) {
      amount = data * (1 - exp(-selectivities_[i]->GetAgeResult(category->min_age_ + j, category->age_length_) * (m * ratio)));
      data -= amount;
      total_amount += amount;
      ++j;
    }
    ++i;
  }
  total_removals_by_year_.push_back(total_amount);
}

/**
 * Reset the Mortality Process
 */
void MortalityConstantRate::DoReset() {
  mortality_rates_.clear();
  total_removals_by_year_.clear();
}

/**
 * Fill the report cache
 * @description A method for reporting process information
 * @param cache a cache object to print to
 */
void MortalityConstantRate::FillReportCache(ostringstream& cache) {
  cache << "years: ";
  for (auto year : model_->years()) cache << year << " ";
  cache << "\ntotal_removals: ";
  for (auto removal : total_removals_by_year_) cache << AS_DOUBLE(removal) << " ";
  cache << REPORT_EOL;
}

/**
 * Fill the tabular report cache
 *
 * @description A method for reporting tabular process information
 * @param cache a cache object to print to
 * @param first_run whether to print the header
 */
void MortalityConstantRate::FillTabularReportCache(ostringstream& cache, bool first_run) {
  if (first_run) {
    vector<unsigned> years = model_->years();
    for (auto year : years) {
      cache << "removals[" << label_ << "][" << year << "] ";
    }
    cache << REPORT_EOL;
  }

  for (auto removal : total_removals_by_year_) cache << AS_DOUBLE(removal) << " ";
  cache << REPORT_EOL;
}

} /* namespace age */
} /* namespace processes */
} /* namespace niwa */
