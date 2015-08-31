/**
 * @file ProportionsAtAge.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 8/04/2013
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "ProportionsAtAge.h"

#include <algorithm>

#include "Model/Model.h"
#include "Partition/Accessors/All.h"
#include "Utilities/DoubleCompare.h"
#include "Utilities/Map.h"
#include "Utilities/Math.h"
#include "Utilities/To.h"

// Namespaces
namespace niwa {
namespace observations {

/**
 * Default constructor
 */
ProportionsAtAge::ProportionsAtAge() {
  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "Minimum age", "");
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "Maximum age", "");
  parameters_.Bind<bool>(PARAM_AGE_PLUS, &age_plus_, "Use age plus group", "", true);
  parameters_.Bind<Double>(PARAM_TOLERANCE, &tolerance_, "Tolerance", "", Double(0.001));
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "Year to execute in", "");
  parameters_.Bind<Double>(PARAM_DELTA, &delta_, "Delta", "", DELTA);
  parameters_.Bind<Double>(PARAM_PROCESS_ERRORS, &process_error_values_, "Process error", "", true);
  parameters_.Bind<string>(PARAM_AGEING_ERROR, &ageing_error_label_, "Label of ageing error to use", "", "");
  obs_table_ = TablePtr(new parameters::Table(PARAM_OBS));
  error_values_table_ = TablePtr(new parameters::Table(PARAM_ERROR_VALUES));
  parameters_.BindTable(PARAM_OBS, obs_table_, "Table of Observatons", "", false);
  parameters_.BindTable(PARAM_ERROR_VALUES, error_values_table_, "", "", false);
}

/**
 * Validate configuration file parameters
 */
void ProportionsAtAge::DoValidate() {
  age_spread_ = (max_age_ - min_age_) + 1;
  map<unsigned, vector<Double>> error_values_by_year;
  map<unsigned, vector<Double>> obs_by_year;

  /**
   * Do some simple checks
   */
  ModelPtr model = Model::Instance();
  if (min_age_ < model->min_age())
    LOG_ERROR_P(PARAM_MIN_AGE) << ": min_age (" << min_age_ << ") is less than the model's min_age (" << model->min_age() << ")";
  if (max_age_ > model->max_age())
    LOG_ERROR_P(PARAM_MAX_AGE) << ": max_age (" << max_age_ << ") is greater than the model's max_age (" << model->max_age() << ")";
  if (process_error_values_.size() != 0 && process_error_values_.size() != years_.size()) {
    LOG_ERROR_P(PARAM_PROCESS_ERRORS) << " number of values provied (" << process_error_values_.size() << ") does not match the number of years provided ("
        << years_.size() << ")";
  }
  for (Double process_error : process_error_values_) {
    if (process_error < 0.0)
      LOG_ERROR_P(PARAM_PROCESS_ERRORS) << ": process_error (" << AS_DOUBLE(process_error) << ") cannot be less than 0.0";
  }
  if (process_error_values_.size() != 0)
    process_errors_by_year_ = utilities::Map::create(years_, process_error_values_);
  if (delta_ < 0.0)
    LOG_ERROR_P(PARAM_DELTA) << ": delta (" << AS_DOUBLE(delta_) << ") cannot be less than 0.0";

  /**
   * Validate the number of obs provided matches age spread * category_labels * years
   * This is because we'll have 1 set of obs per category collection provided.
   * categories male+female male = 2 collections
   */
  unsigned obs_expected = age_spread_ * category_labels_.size() + 1;
  vector<vector<string>>& obs_data = obs_table_->data();
  if (obs_data.size() != years_.size()) {
    LOG_ERROR_P(PARAM_OBS) << " has " << obs_data.size() << " rows defined, but we expected " << years_.size()
        << " to match the number of years provided";
  }

  for (vector<string>& obs_data_line : obs_data) {
    if (obs_data_line.size() != obs_expected) {
      LOG_ERROR_P(PARAM_OBS) << " has " << obs_data_line.size() << " values defined, but we expected " << obs_expected
          << " to match the age speard * categories + 1 (for year)";
    }

    unsigned year = 0;
    if (!utilities::To<unsigned>(obs_data_line[0], year))
      LOG_ERROR_P(PARAM_OBS) << " value " << obs_data_line[0] << " could not be converted in to an unsigned integer. It should be the year for this line";
    if (std::find(years_.begin(), years_.end(), year) == years_.end())
      LOG_ERROR_P(PARAM_OBS) << " value " << year << " is not a valid year for this observation";

    for (unsigned i = 1; i < obs_data_line.size(); ++i) {
      Double value = 0;
      if (!utilities::To<Double>(obs_data_line[i], value))
        LOG_ERROR_P(PARAM_OBS) << " value (" << obs_data_line[i] << ") could not be converted to a double";
      obs_by_year[year].push_back(value);
    }
    if (obs_by_year[year].size() != obs_expected - 1)
      LOG_CODE_ERROR() << "obs_by_year_[year].size() (" << obs_by_year[year].size() << ") != obs_expected - 1 (" << obs_expected -1 << ")";
  }


  /**
   * Build our error value map
   */
  vector<vector<string>>& error_values_data = error_values_table_->data();
  if (error_values_data.size() != years_.size()) {
    LOG_ERROR_P(PARAM_ERROR_VALUES) << " has " << error_values_data.size() << " rows defined, but we expected " << years_.size()
        << " to match the number of years provided";
  }

  for (vector<string>& error_values_data_line : error_values_data) {
    if (error_values_data_line.size() != 2 && error_values_data_line.size() != obs_expected) {
      LOG_ERROR_P(PARAM_ERROR_VALUES) << " has " << error_values_data_line.size() << " values defined, but we expected " << obs_expected
          << " to match the age speard * categories + 1 (for year)";
    }

    unsigned year = 0;
    if (!utilities::To<unsigned>(error_values_data_line[0], year))
      LOG_ERROR_P(PARAM_ERROR_VALUES) << " value " << error_values_data_line[0] << " could not be converted in to an unsigned integer. It should be the year for this line";
    if (std::find(years_.begin(), years_.end(), year) == years_.end())
      LOG_ERROR_P(PARAM_ERROR_VALUES) << " value " << year << " is not a valid year for this observation";
    for (unsigned i = 1; i < error_values_data_line.size(); ++i) {
      Double value = 0;

      if (!utilities::To<Double>(error_values_data_line[i], value))
        LOG_ERROR_P(PARAM_ERROR_VALUES) << " value (" << error_values_data_line[i] << ") could not be converted to a double";
      if (likelihood_type_ == PARAM_LOGNORMAL && value <= 0.0) {
        LOG_ERROR_P(PARAM_ERROR_VALUES) << ": error_value (" << AS_DOUBLE(value) << ") cannot be equal to or less than 0.0";
      } else if (likelihood_type_ == PARAM_MULTINOMIAL && value < 0.0) {
        LOG_ERROR_P(PARAM_ERROR_VALUES) << ": error_value (" << AS_DOUBLE(value) << ") cannot be less than 0.0";
      }

      error_values_by_year[year].push_back(value);
    }
    if (error_values_by_year[year].size() == 1) {
      error_values_by_year[year].assign(obs_expected - 1, error_values_by_year[year][0]);
    }
    if (error_values_by_year[year].size() != obs_expected - 1)
      LOG_CODE_ERROR() << "error_values_by_year_[year].size() (" << error_values_by_year[year].size() << ") != obs_expected - 1 (" << obs_expected -1 << ")";
  }

  /**
   * Validate likelihood type
   */
  if (likelihood_type_ != PARAM_LOGNORMAL && likelihood_type_ != PARAM_MULTINOMIAL)
    LOG_ERROR_P(PARAM_LIKELIHOOD) << ": likelihood " << likelihood_type_ << " is not supported by the proportions at age observation. "
        << "Supported types are " << PARAM_LOGNORMAL << " and " << PARAM_MULTINOMIAL;

  /**
   * Build our proportions and error values for use in the observation
   * If the proportions for a given observation do not sum to 1.0
   * and is off by more than the tolerance rescale them.
   */
  Double value = 0.0;
  for (auto iter = obs_by_year.begin(); iter != obs_by_year.end(); ++iter) {
    Double total = 0.0;

    for (unsigned i = 0; i < category_labels_.size(); ++i) {
      for (unsigned j = 0; j < age_spread_; ++j) {
        unsigned obs_index = i * age_spread_ + j;
        if (!utilities::To<Double>(iter->second[obs_index], value))
          LOG_ERROR_P(PARAM_OBS) << ": obs_ value (" << iter->second[obs_index] << ") at index " << obs_index + 1
              << " in the definition could not be converted to a numeric double";

        Double error_value = error_values_by_year[iter->first][obs_index];
        error_values_[iter->first][category_labels_[i]].push_back(error_value);
        proportions_[iter->first][category_labels_[i]].push_back(value);
        total += value;
      }
    }

    if (fabs(1.0 - total) > tolerance_) {
      LOG_ERROR_P(PARAM_OBS) << ": obs sum total (" << total << ") for year (" << iter->first << ") exceeds tolerance (" << tolerance_ << ") from 1.0";
    }
  }
}

/**
 * Build any runtime relationships we may have and ensure
 * the labels for other objects are valid.
 */
void ProportionsAtAge::DoBuild() {
  partition_ = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(category_labels_));
  cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(category_labels_));

/*
  if (ageing_error_label_ != PARAM_NORMAL || ageing_error_label_ != PARAM_OFF_BY_ONE || ageing_error_label_ != "")
    LOG_ERROR_P(PARAM_AGEING_ERROR) << "Wrong ageing error label choices are" << PARAM_NORMAL << "; " << PARAM_OFF_BY_ONE << " if you haven't specified this parameter no age misclassification is applied";
*/

  age_results_.resize(age_spread_ * category_labels_.size(), 0.0);
}

/**
 * This method is called at the start of the targetted
 * time step for this observation.
 *
 * At this point we need to build our cache for the partition
 * structure to use with any interpolation
 */
void ProportionsAtAge::PreExecute() {
  ModelPtr model = Model::Instance();

  cached_partition_->BuildCache();

  if (cached_partition_->Size() != proportions_[model->current_year()].size())
    LOG_CODE_ERROR() << "cached_partition_->Size() != proportions_[model->current_year()].size()";
  if (partition_->Size() != proportions_[model->current_year()].size())
    LOG_CODE_ERROR() << "partition_->Size() != proportions_[model->current_year()].size()";
}

/**
 *
 */
void ProportionsAtAge::Execute() {
  LOG_TRACE();

  /**
   * Verify our cached partition and partition sizes are correct
   */
  ModelPtr model = Model::Instance();
  auto cached_partition_iter  = cached_partition_->Begin();
  auto partition_iter         = partition_->Begin(); // vector<vector<partition::Category> >

  /**
   * Loop through the provided categories. Each provided category (combination) will have a list of observations
   * with it. We need to build a vector of proportions for each age using that combination and then
   * compare it to the observations.
   */
  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter, ++cached_partition_iter) {
    Double      selectivity_result = 0.0;
    Double      start_value        = 0.0;
    Double      end_value          = 0.0;
    Double      final_value        = 0.0;

    vector<Double> expected_values(age_spread_, 0.0);

    /**
     * Loop through the 2 combined categories building up the
     * expected proportions values.
     */
    auto category_iter = partition_iter->begin();
    auto cached_category_iter = cached_partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++cached_category_iter, ++category_iter) {
      for (unsigned data_offset = 0; data_offset < (*category_iter)->data_.size(); ++data_offset) {
        // Check and skip ages we don't care about.
        if ((*category_iter)->min_age_ + data_offset < min_age_)
          continue;
        if ((*category_iter)->min_age_ + data_offset > max_age_ && !age_plus_)
          break;


        unsigned age_offset = ( (*category_iter)->min_age_ + data_offset) - min_age_;
        unsigned age        = ( (*category_iter)->min_age_ + data_offset);
        if (min_age_ + age_offset > max_age_)
          age_offset = age_spread_ - 1;

        selectivity_result = selectivities_[category_offset]->GetResult(age);
        start_value   = (*cached_category_iter).data_[data_offset];
        end_value     = (*category_iter)->data_[data_offset];
        final_value   = 0.0;

        if (mean_proportion_method_) {
          final_value = start_value + ((end_value - start_value) * proportion_of_time_);
          expected_values[age_offset] += final_value * selectivity_result;
        } else {
          final_value = fabs(start_value - end_value);
        expected_values[age_offset] += final_value * selectivity_result;
        }
        //expected_values[age_offset] += final_value; //* selectivity_result;

        LOG_FINE() << "----------";
        LOG_FINE() << "Category: " << (*category_iter)->name_ << " at age " << age;
        LOG_FINE() << "Selectivity: " << selectivities_[category_offset]->label() << ": " << selectivity_result;
        LOG_FINE() << "start_value: " << start_value << "; end_value: " << end_value << "; final_value: " << final_value;
        LOG_FINE() << "expected_value becomes: " << expected_values[age_offset];
      }
    }

    if (expected_values.size() != proportions_[model->current_year()][category_labels_[category_offset]].size())
      LOG_CODE_ERROR() << "expected_values.size(" << expected_values.size() << ") != proportions_[category_offset].size("
        << proportions_[model->current_year()][category_labels_[category_offset]].size() << ")";

    /**
     * save our comparisons so we can use them to generate the score from the likelihoods later
     */

    for (unsigned i = 0; i < expected_values.size(); ++i) {
      SaveComparison(category_labels_[category_offset], min_age_ + i ,0.0 ,expected_values[i], proportions_[model->current_year()][category_labels_[category_offset]][i],
          process_errors_by_year_[model->current_year()], error_values_[model->current_year()][category_labels_[category_offset]][i], delta_, 0.0);
    }
  }
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void ProportionsAtAge::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  if (Model::Instance()->run_mode() == RunMode::kSimulation) {
    likelihood_->SimulateObserved(comparisons_);
    for (auto& iter :  comparisons_) {
      Double total = 0.0;
      for (auto& comparison : iter.second)
        total += comparison.observed_;
      for (auto& comparison : iter.second)
        comparison.observed_ /= total;
    }
  } else {
    /**
     * Convert the expected_values in to a proportion
     */
    for (unsigned year : years_) {
      Double running_total = 0.0;
      for (obs::Comparison comparison : comparisons_[year]) {
        running_total += comparison.expected_;
      }
      for (obs::Comparison& comparison : comparisons_[year]) {
        if (running_total != 0.0)
          comparison.expected_  = comparison.expected_ / running_total;
        else
          comparison.expected_  = 0.0;
      }

      scores_[year] = likelihood_->GetInitialScore(comparisons_, year);
      LOG_FINEST() << "-- Observation score calculation";
      LOG_FINEST() << "[" << year << "] Initial Score:"<< scores_[year];
      likelihood_->GetScores(comparisons_);
      for (obs::Comparison comparison : comparisons_[year]) {
        LOG_FINEST() << "[" << year << "]+ likelihood score: " << comparison.score_;
        scores_[year] += comparison.score_;
      }
    }
  }
}

} /* namespace observations */
} /* namespace niwa */
