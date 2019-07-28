//
// Created by mich on 28/07/19.
//

#include "LP_Provider.h"

LP_Provider::LP_Provider(const Input &input_, const Variables &variables_, Direction objective_dir_) :
      _input{input_}, _variables{variables_}, _objective{objective_dir_} {
   initialize_sorted_subsets();
   create_objective();
   create_constraints();
}

const LP_Provider::Constraint &LP_Provider::get_constraint(size_t constr_idx) const {
   if (constr_idx >= num_constraints()) {
      throw std::logic_error("Call to get_constraint with too large const_idx");
   }
   return _constraints[constr_idx];
}

void LP_Provider::create_objective() {
   _objective.lin_vec.clear();
   const std::vector<std::vector<std::vector<Variables::VarID>>> &teacher_is_in_school_var = _variables.get_teacher_is_in_school_var();
   const std::vector<std::vector<Variables::VarID>> &day_weight_for_class_sorted = _variables.get_day_weight_for_class_sorted();

   size_t num_var_in_objective = 0;
   for (const auto &teacher_matr: teacher_is_in_school_var) {
      for (const auto &day_vec: teacher_matr) {
         num_var_in_objective += day_vec.size();
      }
   }
   for (const auto &class_vec: day_weight_for_class_sorted) {
      num_var_in_objective += class_vec.size();
   }

   _objective.lin_vec.reserve(num_var_in_objective);

   for (size_t teacher_idx = 0; teacher_idx != teacher_is_in_school_var.size(); ++teacher_idx) {
      for (unsigned int day_idx = 0; day_idx != teacher_is_in_school_var[teacher_idx].size(); ++day_idx) {
         for (unsigned int hour_idx = 0;
              hour_idx != teacher_is_in_school_var[teacher_idx][hour_idx].size(); ++hour_idx) {
            // objective to minimize the penalties to teachers
            _objective.lin_vec.emplace_back(teacher_is_in_school_var[teacher_idx][day_idx][hour_idx],
                                            _input.get_teachers()[teacher_idx].penalties[day_idx][hour_idx]);
         }
      }
   }
   for (size_t class_idx = 0; class_idx != day_weight_for_class_sorted.size(); ++class_idx) {
      for (unsigned int day_idx = 0; day_idx != day_weight_for_class_sorted[class_idx].size(); ++day_idx) {
         // objective to make the weight of classes as uniform as possible
         _objective.lin_vec.emplace_back(day_weight_for_class_sorted[class_idx][day_idx],
                                         std::max(1 - double(day_idx) / (Input::NUM_DAYS_PER_WEEK - 1), 0.0));
      }
   }
}

void LP_Provider::create_constraints() {
   _constraints.clear();
   _constraints.reserve(guess_num_constraints());

   // create the constraints
   create_teacher_available_constraints();
   create_teacher_has_lesson_constraints();
   create_teacher_is_in_school_constraints();
   create_class_sovrapposition_constraints();
   create_num_lessons_constraints();
   prevent_non_consecutive_hours();
   create_cons_var_constraints();
   create_day_weight_constraints();
   create_day_weight_sorted_constraints();
}

void LP_Provider::initialize_sorted_subsets() {
   if (not _sorted_subsets.empty()) {
      return;
   }
   _sorted_subsets.resize(Input::NUM_DAYS_PER_WEEK + 1);
   _sorted_subsets[0].resize(1);
   for (unsigned int cardinality = 1; cardinality <= Input::NUM_DAYS_PER_WEEK; ++cardinality) {
      size_t num_subsets = 1;
      for (unsigned int idx = 0; idx != cardinality; ++idx) {
         num_subsets *= Input::NUM_DAYS_PER_WEEK - idx;
         num_subsets /= (idx + 1);
      }
      _sorted_subsets[cardinality].reserve(num_subsets);
      for (const auto &head: _sorted_subsets[cardinality - 1]) {
         for (unsigned int day = head.empty() ? 0 : head.back() + 1; day != Input::NUM_DAYS_PER_WEEK; ++day) {
            _sorted_subsets[cardinality].emplace_back(head);
            _sorted_subsets[cardinality].back().emplace_back(day);
         }
      }
   }
}


size_t LP_Provider::guess_num_constraints() const {
   size_t reserve_counter = 0;
   reserve_counter += 2 * _input.num_teachers() * Input::total_num_hours_in_week;
   for (unsigned int day_idx = 0; day_idx != Input::NUM_DAYS_PER_WEEK; ++day_idx) {
      size_t num_costraints_teacher_is_in_school = 2 * Input::NUM_HOURS_PER_DAY[day_idx];
      for (unsigned int hour = 0; hour != Input::NUM_HOURS_PER_DAY[day_idx]; ++hour) {
         num_costraints_teacher_is_in_school += (hour + 1) * (Input::NUM_HOURS_PER_DAY[day_idx] - hour);
      }
      reserve_counter += _input.num_teachers() * num_costraints_teacher_is_in_school;
   }
   reserve_counter += _input.num_classes() * Input::total_num_hours_in_week;
   reserve_counter += _input.num_requirements();
   reserve_counter += _input.num_requirements() * (Input::total_num_hours_in_week - Input::NUM_DAYS_PER_WEEK);
   reserve_counter += _input.num_requirements();
   reserve_counter += _input.num_classes() * Input::NUM_DAYS_PER_WEEK;
   for (unsigned int day_idx = 0; day_idx != Input::NUM_DAYS_PER_WEEK; ++day_idx) {
      for (const auto &sorted_subset: _sorted_subsets[Input::NUM_HOURS_PER_DAY[day_idx] + 1]) {
         reserve_counter += _input.num_classes() * sorted_subset.size();
      }
   }
   return reserve_counter;
}

void LP_Provider::create_teacher_available_constraints() {
   const auto &teacher_is_in_school_var = _variables.get_teacher_is_in_school_var();
   for (Input::ID teacher_id = 0; teacher_id != _input.num_teachers(); ++teacher_id) {
      const Input::Teacher &teacher = _input.get_teachers()[teacher_id];
      for (unsigned int day_idx = 0; day_idx != Input::NUM_DAYS_PER_WEEK; ++day_idx) {
         for (unsigned int hour_idx = 0; hour_idx != Input::NUM_HOURS_PER_DAY[day_idx]; ++hour_idx) {
            _constraints.emplace_back(Leq, teacher.is_available(day_idx, hour_idx) ? 1.0 : 0.0);
            _constraints.back().lhs.emplace_back(teacher_is_in_school_var[teacher_id][day_idx][hour_idx], 1.0);
         }
      }
   }
}

void LP_Provider::create_teacher_has_lesson_constraints() {
   const auto &requirement_var_per_teacher = _variables.get_requirement_var_per_teacher();
   const auto &teacher_has_lesson_var = _variables.get_teacher_has_lesson_var();
   for (Input::ID teacher_id = 0; teacher_id != _input.num_teachers(); ++teacher_id) {
      for (unsigned int day_idx = 0; day_idx != requirement_var_per_teacher[teacher_id].size(); ++day_idx) {
         for (unsigned int hour_idx = 0;
              hour_idx != requirement_var_per_teacher[teacher_id][day_idx].size(); ++hour_idx) {
            _constraints.emplace_back(Eq, 0.0);
            _constraints.back().lhs.reserve(requirement_var_per_teacher[teacher_id][day_idx][hour_idx].size() + 1);
            for (VarID entry: requirement_var_per_teacher[teacher_id][day_idx][hour_idx]) {
               _constraints.back().lhs.emplace_back(entry, 1.0);
            }
            _constraints.back().lhs.emplace_back(teacher_has_lesson_var[teacher_id][day_idx][hour_idx], -1.0);
         }
      }
   }
}

void LP_Provider::create_teacher_is_in_school_constraints() {
   const auto &teacher_is_in_school_var = _variables.get_teacher_is_in_school_var();
   const auto &teacher_has_lesson_var = _variables.get_teacher_has_lesson_var();
   for (Input::ID teacher_id = 0; teacher_id != _input.num_teachers(); ++teacher_id) {
      for (unsigned int day_idx = 0; day_idx != Input::NUM_DAYS_PER_WEEK; ++day_idx) {
         for (unsigned int hour_idx = 0; hour_idx < Input::NUM_HOURS_PER_DAY[day_idx]; ++hour_idx) {
            for (unsigned int earlier_hour_idx = 0; earlier_hour_idx <= hour_idx; ++earlier_hour_idx) {
               for (unsigned int later_hour_idx = hour_idx;
                    later_hour_idx < Input::NUM_HOURS_PER_DAY[day_idx]; ++later_hour_idx) {
                  // if teacher has class in earlier_hour_idx and later_hour_idx, then he's in school at hour_idx
                  _constraints.emplace_back(Geq, -1.0);
                  _constraints.back().lhs.reserve(3);
                  _constraints.back().lhs.emplace_back(teacher_is_in_school_var[teacher_id][day_idx][hour_idx], 1.0);
                  _constraints.back().lhs.emplace_back(teacher_has_lesson_var[teacher_id][day_idx][earlier_hour_idx],
                                                       -1.0);
                  _constraints.back().lhs.emplace_back(teacher_has_lesson_var[teacher_id][day_idx][later_hour_idx],
                                                       -1.0);
               }
            }
            // not in school if the teacher has no lessons in time interval [0,hour_idx]
            _constraints.emplace_back(Leq, 0.0);
            _constraints.back().lhs.reserve(hour_idx + 2);
            _constraints.back().lhs.emplace_back(teacher_is_in_school_var[teacher_id][day_idx][hour_idx], 1.0);
            for (unsigned int earlier_hour_idx = 0; earlier_hour_idx <= hour_idx; ++earlier_hour_idx) {
               _constraints.back().lhs.emplace_back(teacher_has_lesson_var[teacher_id][day_idx][earlier_hour_idx],
                                                    -1.0);
            }
            // not in school if the teacher has no lessons in time interval [hour_idx,NUM_HOURS_PER_DAY[day_idx])
            _constraints.emplace_back(Leq, 0.0);
            _constraints.back().lhs.reserve(Input::NUM_HOURS_PER_DAY[day_idx] - hour_idx + 1);
            _constraints.back().lhs.emplace_back(teacher_is_in_school_var[teacher_id][day_idx][hour_idx], 1.0);
            for (unsigned int later_hour_idx = hour_idx;
                 later_hour_idx != Input::NUM_HOURS_PER_DAY[day_idx]; ++later_hour_idx) {
               _constraints.back().lhs.emplace_back(teacher_has_lesson_var[teacher_id][day_idx][later_hour_idx], -1.0);
            }
         }
      }
   }
}
void LP_Provider::create_class_sovrapposition_constraints() {
   const auto &requirement_var_per_class = _variables.get_requirement_var_per_class();
   for (Input::ID class_id = 0; class_id != _input.num_classes(); ++class_id) {
      const Input::Class &class_object = _input.get_classes()[class_id];
      for (unsigned int day_idx = 0; day_idx != Input::NUM_DAYS_PER_WEEK; ++day_idx) {
         for (unsigned int hour_idx = 0; hour_idx != Input::NUM_HOURS_PER_DAY[day_idx]; ++hour_idx) {
            _constraints.emplace_back(Eq, hour_idx < class_object.num_hours_per_day[day_idx] ? 1.0 : 0.0);
            _constraints.back().lhs.reserve(requirement_var_per_class[class_id][day_idx][hour_idx].size());
            for (VarID entry: requirement_var_per_class[class_id][day_idx][hour_idx]) {
               _constraints.back().lhs.emplace_back(entry);
            }
         }
      }
   }
}

void LP_Provider::create_num_lessons_constraints() {
   const auto &requirement_var = _variables.get_requirement_var();
   for (Input::ID req_idx = 0; req_idx != _input.num_requirements(); ++req_idx) {
      const Input::Requirement &requirement = _input.get_requirements()[req_idx];
      _constraints.emplace_back(Eq, requirement.num_lessons());
      _constraints.back().lhs.reserve(Input::total_num_hours_in_week);
      for (const auto &day: requirement_var[req_idx]) {
         for (VarID hour: day) {
            _constraints.back().lhs.emplace_back(hour);
         }
      }
   }
}
void LP_Provider::prevent_non_consecutive_hours() {
   const auto &requirement_var = _variables.get_requirement_var();
   for (Input::ID req_idx = 0; req_idx != _input.num_requirements(); ++req_idx) {
      for (unsigned int day_idx = 0; day_idx != Input::NUM_DAYS_PER_WEEK; ++day_idx) {
         for (unsigned int first_hour_idx = 0; first_hour_idx < Input::NUM_HOURS_PER_DAY[day_idx]; ++first_hour_idx) {
            for (unsigned int second_hour_idx = first_hour_idx + 2;
                 second_hour_idx < Input::NUM_HOURS_PER_DAY[day_idx]; ++second_hour_idx) {
               _constraints.emplace_back(Leq, 1.0);
               _constraints.back().lhs.reserve(2);
               _constraints.back().lhs.emplace_back(requirement_var[req_idx][day_idx][first_hour_idx], 1.0);
               _constraints.back().lhs.emplace_back(requirement_var[req_idx][day_idx][second_hour_idx], 1.0);
            }
         }
      }
   }
}

void LP_Provider::create_cons_var_constraints() {
   const auto &requirement_cons_var_from_hour = _variables.get_requirement_cons_var_from_hour();
   for (Input::ID req_idx = 0; req_idx != _input.num_requirements(); ++req_idx) {
      _constraints.emplace_back(Eq, _input.get_requirements()[req_idx].num_days_with_cons_hours);
      _constraints.back().lhs.reserve(Input::total_num_hours_in_week - Input::NUM_DAYS_PER_WEEK);
      for (unsigned int day_idx = 0; day_idx != Input::NUM_DAYS_PER_WEEK; ++day_idx) {
         for (unsigned int hour_idx = 0; hour_idx + 1 < Input::NUM_HOURS_PER_DAY[day_idx]; ++hour_idx) {
            _constraints.back().lhs.emplace_back(requirement_cons_var_from_hour[req_idx][day_idx][hour_idx]);
         }
      }
   }
}
void LP_Provider::create_day_weight_constraints() {
   const auto &day_weight_for_class = _variables.get_day_weight_for_class();
   const auto &requirement_var_per_class = _variables.get_requirement_var_per_class();
   size_t reserve_counter = 0;
   for (Input::ID class_id = 0; class_id != _input.num_classes(); ++class_id) {
      for (unsigned int day_idx = 0; day_idx != Input::NUM_DAYS_PER_WEEK; ++day_idx) {
         _constraints.emplace_back(Eq, 0.0);
         reserve_counter = 1;
         for (unsigned int hour_idx = 0; hour_idx != Input::NUM_HOURS_PER_DAY[day_idx]; ++hour_idx) {
            reserve_counter += requirement_var_per_class[class_id][day_idx][hour_idx].size();
         }
         _constraints.back().lhs.reserve(reserve_counter);
         _constraints.back().lhs.emplace_back(day_weight_for_class[class_id][day_idx], 1.0);
         for (unsigned int hour_idx = 0; hour_idx != Input::NUM_HOURS_PER_DAY[day_idx]; ++hour_idx) {
            for (VarID entry: requirement_var_per_class[class_id][day_idx][hour_idx]) {
               Input::ID req_id = _variables.get_variable(entry).holder_id;
               _constraints.back().lhs.emplace_back(entry, -_input.get_requirements()[req_id].average_lesson_weight);
            }
         }
      }
   }
}
void LP_Provider::create_day_weight_sorted_constraints() {
   const auto &day_weight_for_class_sorted = _variables.get_day_weight_for_class_sorted();
   // sets day_weight_for_class_sorted
   for (Input::ID class_id = 0; class_id != _input.num_classes(); ++class_id) {
      for (unsigned int sorted_day_idx = 0; sorted_day_idx != Input::NUM_DAYS_PER_WEEK; ++sorted_day_idx) {
         for (const auto &subset: _sorted_subsets[sorted_day_idx + 1]) {
            _constraints.emplace_back(Geq, 0.0);
            _constraints.back().lhs.reserve(2 * (sorted_day_idx + 1));
            for (unsigned int day_idx = 0; day_idx <= sorted_day_idx; ++day_idx) {
               _constraints.back().lhs.emplace_back(day_weight_for_class_sorted[class_id][day_idx], 1.0);
            }
            for (VarID day_idx : subset) {
               _constraints.back().lhs.emplace_back(day_weight_for_class_sorted[class_id][day_idx], -1.0);
            }
         }
      }
   }
}