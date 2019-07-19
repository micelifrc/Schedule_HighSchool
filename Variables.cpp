//
// Created by mich on 16/07/19.
//

#include "Variables.h"

Variables::Variables(const Input &input_) :
      _input{input_},
      _teacher_is_in_school_var(_input.num_teachers(), std::vector<std::vector<VarID>>(Input::NUM_DAYS_PER_WEEK)),
      _requirement_var(_input.num_requirements(), std::vector<std::vector<VarID>>(Input::NUM_DAYS_PER_WEEK)),
      _requirement_cons_var_from_hour(_input.num_requirements(),
                                      std::vector<std::vector<VarID>>(Input::NUM_DAYS_PER_WEEK)),
      _requirement_cons_var(_input.num_requirements(), std::vector<VarID>(Input::NUM_DAYS_PER_WEEK, InvalidVarID)),
      _day_weight_for_class(_input.num_classes(), std::vector<VarID>(Input::NUM_DAYS_PER_WEEK, InvalidVarID)),
      _day_weight_for_class_sorted(_input.num_classes(), std::vector<VarID>(Input::NUM_DAYS_PER_WEEK, InvalidVarID)) {
   reserve_containers_space();
   create_teacher_is_in_school_var();
   create_requirement_var();
   create_requirement_cons_var_from_hour();
   create_requirement_cons_var();
}

void Variables::reserve_containers_space() {
   for (auto &teacher_matr: _teacher_is_in_school_var) {
      for (unsigned int day = 0; day != Input::NUM_DAYS_PER_WEEK; ++day) {
         teacher_matr[day].resize(Input::NUM_HOURS_PER_DAY[day], InvalidVarID);
      }
   }
   for (auto &req_matr: _requirement_var) {
      for (unsigned int day = 0; day != Input::NUM_DAYS_PER_WEEK; ++day) {
         req_matr[day].resize(Input::NUM_HOURS_PER_DAY[day], InvalidVarID);
      }
   }
   for (auto &req_matr: _requirement_cons_var_from_hour) {
      for (unsigned int day = 0; day != Input::NUM_DAYS_PER_WEEK; ++day) {
         req_matr[day].resize(Input::NUM_HOURS_PER_DAY[day] - 1, InvalidVarID);
      }
   }
}

void Variables::create_teacher_is_in_school_var() {
   for (unsigned int teacher_id = 0; teacher_id != _input.num_teachers(); ++teacher_id) {
      for (unsigned int day = 0; day != Input::NUM_DAYS_PER_WEEK; ++day) {
         for (unsigned int hour = 0; hour != Input::NUM_HOURS_PER_DAY[day]; ++hour) {
            VarID var_id = num_var();
            _teacher_is_in_school_var[teacher_id][day][hour] = var_id;
            _variables.emplace_back(var_id, teacher_id, day, hour);
         }
      }
   }
}

void Variables::create_requirement_var() {
   for (unsigned int requirement_id = 0; requirement_id != _input.num_requirements(); ++requirement_id) {
      for (unsigned int day = 0; day != Input::NUM_DAYS_PER_WEEK; ++day) {
         for (unsigned int hour = 0; hour != Input::NUM_HOURS_PER_DAY[day]; ++hour) {
            VarID var_id = num_var();
            _requirement_var[requirement_id][day][hour] = var_id;
            _variables.emplace_back(var_id, requirement_id, day, hour);
         }
      }
   }
}

void Variables::create_requirement_cons_var_from_hour() {
   for (unsigned int requirement_id = 0; requirement_id != _input.num_requirements(); ++requirement_id) {
      if(_input.requirements()[requirement_id].num_days_with_cons_hours == 0) {
         continue;
      }
      for (unsigned int day = 0; day != Input::NUM_DAYS_PER_WEEK; ++day) {
         for (unsigned int hour = 0; hour != Input::NUM_HOURS_PER_DAY[day] - 1; ++hour) {
            VarID var_id = num_var();
            _requirement_cons_var_from_hour[requirement_id][day][hour] = var_id;
            _variables.emplace_back(var_id, requirement_id, day, hour);
         }
      }
   }
}

void Variables::create_requirement_cons_var() {
   for (unsigned int requirement_id = 0; requirement_id != _input.num_requirements(); ++requirement_id) {
      if(_input.requirements()[requirement_id].num_days_with_cons_hours == 0) {
         continue;
      }
      for (unsigned int day = 0; day != Input::NUM_DAYS_PER_WEEK; ++day) {
         VarID var_id = num_var();
         _requirement_cons_var[requirement_id][day] = var_id;
         _variables.emplace_back(var_id, requirement_id, day);
      }
   }
}

void Variables::create_day_weight_for_class() {
   for (unsigned int class_id = 0; class_id != _input.num_classes(); ++class_id) {
      for (unsigned int day = 0; day != Input::NUM_DAYS_PER_WEEK; ++day) {
         VarID var_id = num_var();
         _day_weight_for_class[class_id][day] = var_id;
         _variables.emplace_back(var_id, class_id, day);
      }
   }
}

void Variables::create_day_weight_for_class_sorted() {
   for (unsigned int class_id = 0; class_id != _input.num_classes(); ++class_id) {
      for (unsigned int day = 0; day != Input::NUM_DAYS_PER_WEEK; ++day) {
         VarID var_id = num_var();
         _day_weight_for_class_sorted[class_id][day] = var_id;
         _variables.emplace_back(var_id, class_id, day);
      }
   }
}
