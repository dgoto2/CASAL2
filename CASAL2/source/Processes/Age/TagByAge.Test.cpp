/**
 * @file TagByAge.Test.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 10/02/2015
 * @section LICENSE
 *
 * Copyright NIWA Science �2015 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * This file contains unit tests for the TagByAge process
 */
#ifdef TESTMODE

// Headers
#include <iostream>

#include "Model/Model.h"
#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Partition/Partition.h"
#include "Processes/Manager.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"

// Namespaces
namespace niwa {
namespace processes {
namespace age {

using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

const std::string test_cases_process_tag_by_age =
    R"(
@model
start_year 1994
final_year 2008
min_age 1
max_age 12
age_plus t
base_weight_units kgs
initialisation_phases iphase1 iphase2
time_steps step_one=[processes=Recruitment] step_two=[processes=Tagging,TagLoss] step_three=[processes=Ageing]

@categories
format stage.sex
names immature.male mature.male immature.female mature.female
age_lengths no_age_length*4


@age_length no_age_length
type none
length_weight no_length_weight

@length_weight no_length_weight
type none

@initialisation_phase iphase1
years 200

@initialisation_phase iphase2
years 1

@ageing Ageing
categories *

@Recruitment Recruitment
type constant
categories immature.male immature.female
proportions 0.5 0.5
R0 997386
age 1

@tag Tagging
type by_age
years 2008
from immature.male immature.female
to mature.male mature.female
selectivities [type=constant; c=0.25] [type=constant; c=0.4]
min_age 3
max_age 6
penalty [type=process]
table numbers
year 3 4 5 6
2008 1000 2000 3000 4000
end_table

@process TagLoss
type tag_loss
categories mature.male mature.female
tag_loss_rate 0
selectivities loss_rate1=[type=constant; c=1] lossrate2=[type=constant; c=1]
tag_loss_type single
year 2008

@report DQ
type derived_quantity
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Processes_Tag_By_Age) {
  AddConfigurationLine(test_cases_process_tag_by_age, __FILE__, 36);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  // 0.000000 0.000000 0.000000 384.615385 769.230769 1153.846154 1538.461538 0.000000 0.000000 0.000000 0.000000 0.000000
  partition::Category& male = model_->partition().category("mature.male");
  EXPECT_DOUBLE_EQ(0.0, male.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[2]);
  EXPECT_DOUBLE_EQ(384.61538461538458, male.data_[3]);
  EXPECT_DOUBLE_EQ(769.23076923076917, male.data_[4]);
  EXPECT_DOUBLE_EQ(1153.84615384615384, male.data_[5]);
  EXPECT_DOUBLE_EQ(1538.4615384615383, male.data_[6]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[7]);

  // 615.384615 1230.769231 1846.153846 2461.538462 0.000000 0.000000 0.000000 0.000000 0.000000
  partition::Category& female = model_->partition().category("mature.female");
  EXPECT_DOUBLE_EQ(0.0, female.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[2]);
  EXPECT_DOUBLE_EQ(615.38461538461547, female.data_[3]);
  EXPECT_DOUBLE_EQ(1230.7692307692309, female.data_[4]);
  EXPECT_DOUBLE_EQ(1846.1538461538462, female.data_[5]);
  EXPECT_DOUBLE_EQ(2461.5384615384619, female.data_[6]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[7]);
}

/**
 * Loss rate
 */
const std::string test_cases_process_tag_by_age_with_loss_rate =
    R"(
@model
start_year 1994
final_year 2008
min_age 1
max_age 12
age_plus t
base_weight_units kgs
initialisation_phases iphase1 iphase2
time_steps step_one=[processes=Recruitment] step_two=[processes=Tagging,TagLoss] step_three=[processes=Ageing]

@categories
format stage.sex
names immature.male mature.male immature.female mature.female
age_lengths no_age_length*4


@age_length no_age_length
type none
length_weight no_length_weight

@length_weight no_length_weight
type none

@initialisation_phase iphase1
years 200

@initialisation_phase iphase2
years 1

@ageing Ageing
categories *

@Recruitment Recruitment
type constant
categories immature.male immature.female
proportions 0.5 0.5
R0 997386
age 1

@tag Tagging
type by_age
years 2008
from immature.male immature.female
to mature.male mature.female
selectivities [type=constant; c=0.25] [type=constant; c=0.4]
min_age 3
max_age 6
penalty [type=process]
table numbers
year 3 4 5 6
2008 500 750 1000 2000
end_table

@process TagLoss
type tag_loss
categories mature.male mature.female
tag_loss_rate 0.02
selectivities loss_rate1=[type=constant; c=1] lossrate2=[type=constant; c=1]
tag_loss_type single
year 2008

@report DQ
type derived_quantity
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Processes_Tag_By_Age_With_Loss_Rate) {
  AddConfigurationLine(test_cases_process_tag_by_age_with_loss_rate, __FILE__, 119);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& male = model_->partition().category("mature.male");
  EXPECT_DOUBLE_EQ(0.0, male.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[2]);
  EXPECT_DOUBLE_EQ(188.4997448666837, male.data_[3]);
  EXPECT_DOUBLE_EQ(282.74961730002553, male.data_[4]);
  EXPECT_DOUBLE_EQ(376.9994897333674, male.data_[5]);
  EXPECT_DOUBLE_EQ(753.9989794667348, male.data_[6]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[7]);

  partition::Category& female = model_->partition().category("mature.female");
  EXPECT_DOUBLE_EQ(0.0, female.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[2]);
  EXPECT_DOUBLE_EQ(301.59959178669396, female.data_[3]);
  EXPECT_DOUBLE_EQ(452.39938768004089, female.data_[4]);
  EXPECT_DOUBLE_EQ(603.19918357338793, female.data_[5]);
  EXPECT_DOUBLE_EQ(1206.3983671467759, female.data_[6]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[7]);
}

/**
 * Loss rate selectivities
 */
const std::string test_cases_process_tag_by_age_with_loss_rate_selectivities =
    R"(
@model
start_year 1994
final_year 2010
min_age 1
max_age 12
base_weight_units kgs
age_plus t
initialisation_phases iphase1 iphase2
time_steps step_one=[processes=Recruitment] step_two=[processes=Tagging,TagLoss] step_three=[processes=Ageing]

@categories
format stage.sex
names immature.male mature.male immature.female mature.female
age_lengths no_age_length*4


@age_length no_age_length
type none
length_weight no_length_weight

@length_weight no_length_weight
type none

@initialisation_phase iphase1
years 200
@initialisation_phase iphase1
years 200

@initialisation_phase iphase2
years 1

@ageing Ageing
categories *

@Recruitment Recruitment
type constant
categories immature.male immature.female
proportions 0.5 0.5
R0 997386
age 1

@tag Tagging
type by_age
years 2008
from immature.male immature.female
to mature.male mature.female
selectivities [type=constant; c=0.25] [type=constant; c=0.4]
min_age 3
max_age 6
penalty [type=process]
table numbers
year 3 4 5 6
2008 500 750 1000 2000
end_table

@process TagLoss
type tag_loss
categories mature.male mature.female
tag_loss_rate 0.02
selectivities lr1=[type=logistic; a50=11.9; ato95=5.25] lr2=[type=constant; c=0.5]
tag_loss_type single
year 2008

@report DQ
type derived_quantity
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Processes_Tag_By_Age_With_Loss_Rate_Selectivities) {
  AddConfigurationLine(test_cases_process_tag_by_age_with_loss_rate_selectivities, __FILE__, 202);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  /**
   * Note the results have moved compared to the other tests because this
   * one has extra years
   */

  // 0.000000 0.000000 0.000000 0.000000 0.000000 190.389423 285.584135 380.778846 761.557692 0.000000 0.000000 0.000000
  partition::Category& male = model_->partition().category("mature.male");
  EXPECT_DOUBLE_EQ(0.0, male.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[2]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[3]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[4]);
  EXPECT_DOUBLE_EQ(192.15793819301646, male.data_[5]);
  EXPECT_DOUBLE_EQ(288.07257446646986, male.data_[6]);
  EXPECT_DOUBLE_EQ(383.7249459510295, male.data_[7]);
  EXPECT_DOUBLE_EQ(766.21588197878077, male.data_[8]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[9]);

  // 0.000000 0.000000 0.000000 0.000000 0.000000 302.788923 454.183385 605.577846 1211.155692 0.000000 0.000000 0.000000
  partition::Category& female = model_->partition().category("mature.female");
  EXPECT_DOUBLE_EQ(0.0, female.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[2]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[3]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[4]);
  EXPECT_DOUBLE_EQ(298.5986257072334, female.data_[5]);
  EXPECT_DOUBLE_EQ(447.89793856084998, female.data_[6]);
  EXPECT_DOUBLE_EQ(597.19725141446679, female.data_[7]);
  EXPECT_DOUBLE_EQ(1194.3945028289336, female.data_[8]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[9]);
}

/**
 * Selectivities
 */
const std::string test_cases_process_tag_by_age_with_selectivities =
    R"(
@model
start_year 1994
final_year 2008
min_age 1
max_age 12
age_plus t
initialisation_phases iphase1 iphase2
time_steps step_one=[processes=Recruitment] step_two=[processes=Tagging,TagLoss] step_three=[processes=Ageing]

@categories
format stage.sex
names immature.male mature.male immature.female mature.female
age_lengths no_age_length*4


@age_length no_age_length
type none
length_weight no_length_weight

@length_weight no_length_weight
type none

@initialisation_phase iphase1
years 200
@initialisation_phase iphase1
years 200

@initialisation_phase iphase2
years 1

@ageing Ageing
categories *

@Recruitment Recruitment
type constant
categories immature.male immature.female
proportions 0.5 0.5
R0 997386
age 1

@tag Tagging
type by_age
years 2008
from immature.male immature.female
to mature.male mature.female
selectivities [type=logistic; a50=9; ato95=3] [type=constant; c=0.7]
min_age 3
max_age 6
penalty [type=process]
table numbers
year 3 4 5 6
2008 500 750 1000 2000
end_table

@process TagLoss
type tag_loss
categories mature.male mature.female
tag_loss_rate 0.02
selectivities lr1=[type=logistic; a50=11.9; ato95=5.25] lr2=[type=constant; c=0.5]
tag_loss_type single
year 2008

@report DQ
type derived_quantity
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Processes_Tag_By_Age_With_Selectivities) {
  AddConfigurationLine(test_cases_process_tag_by_age_with_selectivities, __FILE__, 292);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  // 0.000000 0.000000 0.000000 0.000000 0.000000 190.389423 285.584135 380.778846 761.557692 0.000000 0.000000 0.000000
  partition::Category& male = model_->partition().category("mature.male");
  EXPECT_DOUBLE_EQ(0.0, male.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[2]);
  EXPECT_DOUBLE_EQ(13.442752876251012, male.data_[3]);
  EXPECT_DOUBLE_EQ(49.988235083324454, male.data_[4]);
  EXPECT_DOUBLE_EQ(149.54412296373857, male.data_[5]);
  EXPECT_DOUBLE_EQ(560.16157671188671, male.data_[6]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[7]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[8]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[9]);

  // 0.000000 0.000000 0.000000 0.000000 0.000000 302.788923 454.183385 605.577846 1211.155692 0.000000 0.000000 0.000000
  partition::Category& female = model_->partition().category("mature.female");
  EXPECT_DOUBLE_EQ(0.0, female.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[2]);
  EXPECT_DOUBLE_EQ(481.71412494080471, female.data_[3]);
  EXPECT_DOUBLE_EQ(693.0348836244176, female.data_[4]);
  EXPECT_DOUBLE_EQ(841.93317525604334, female.data_[5]);
  EXPECT_DOUBLE_EQ(1425.1205126517837, female.data_[6]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[7]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[8]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[9]);
}

/**
 * Selectivities
 */
const std::string test_cases_process_tag_by_age_with_proportions_table =
    R"(
@model
start_year 1994
final_year 2010
min_age 1
max_age 12
age_plus t
initialisation_phases iphase1 iphase2
time_steps step_one=[processes=Recruitment] step_two=[processes=Tagging,TagLoss] step_three=[processes=Ageing]

@categories
format stage.sex
names immature.male mature.male immature.female mature.female
age_lengths no_age_length*4


@age_length no_age_length
type none
length_weight no_length_weight

@length_weight no_length_weight
type none

@initialisation_phase iphase1
years 200
@initialisation_phase iphase1
years 200

@initialisation_phase iphase2
years 1

@ageing Ageing
categories *

@Recruitment Recruitment
type constant
categories immature.male immature.female
proportions 0.5 0.5
R0 997386
age 1

@tag Tagging
type by_age
years 2008
from immature.male immature.female
to mature.male mature.female
selectivities [type=constant; c=0.25] [type=constant; c=0.4]
min_age 3
max_age 6
penalty [type=process]
n 10000
table proportions
year 3 4 5 6
2008 0.1 0.2 0.3 0.4
end_table

@process TagLoss
type tag_loss
categories mature.male mature.female
tag_loss_rate 0.02
selectivities lr1=[type=logistic; a50=11.9; ato95=5.25] lr2=[type=constant; c=0.5]
tag_loss_type single
year 2008

@report DQ
type derived_quantity
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Processes_Tag_By_Age_With_Proportions_Table) {
  AddConfigurationLine(test_cases_process_tag_by_age_with_proportions_table, __FILE__, 379);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  // 0.000000 0.000000 0.000000 0.000000 0.000000 380.778846 761.557692 1142.336538 1523.115385 0.000000 0.000000 0.000000
  partition::Category& male = model_->partition().category("mature.male");
  EXPECT_DOUBLE_EQ(0.0, male.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[2]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[3]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[4]);
  EXPECT_DOUBLE_EQ(384.31587638603293, male.data_[5]);
  EXPECT_DOUBLE_EQ(768.19353191058622, male.data_[6]);
  EXPECT_DOUBLE_EQ(1151.1748378530888, male.data_[7]);
  EXPECT_DOUBLE_EQ(1532.4317639575615, male.data_[8]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[9]);

  // 0.000000 0.000000 0.000000 0.000000 0.000000 605.577846 1211.155692 1816.733538 2422.311385 0.000000 0.000000 0.000000
  partition::Category& female = model_->partition().category("mature.female");
  EXPECT_DOUBLE_EQ(0.0, female.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[2]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[3]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[4]);
  EXPECT_DOUBLE_EQ(597.19725141446679, female.data_[5]);
  EXPECT_DOUBLE_EQ(1194.3945028289336, female.data_[6]);
  EXPECT_DOUBLE_EQ(1791.5917542433999, female.data_[7]);
  EXPECT_DOUBLE_EQ(2388.7890056578672, female.data_[8]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[9]);
}

} /* namespace age */
} /* namespace processes */
} /* namespace niwa */

#endif /* TESTMODE */
