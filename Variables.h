#ifndef SCHEDULE_HIGHSCHOOL_VARIABLES_H
#define SCHEDULE_HIGHSCHOOL_VARIABLES_H

#include "Input.h"

class Variables {
public:
   typedef size_t VarID;
   static constexpr VarID InvalidVarID = std::numeric_limits<VarID>::max();

   explicit Variables(const Input &input_);

   struct Variable {
      VarID var_id;
      Input::ID holder_id;  // can be teacher_id, class_id or requirement_id
      Hour hour;

      Variable(VarID var_id_, Input::ID holder_id_, unsigned int day_,
               unsigned int hour_ = std::numeric_limits<unsigned int>::max()) :
            var_id{var_id_}, holder_id{holder_id_}, hour(day_, hour_) {}
   };

   [[nodiscard]] const std::vector<Variable> &get_all_variables() const { return _variables; }

   [[nodiscard]] VarID num_var() const { return _variables.size(); }

   [[nodiscard]] VarID num_01_var() const { return _num_01_var; }

   [[nodiscard]] VarID num_lp_var() const { return num_var() - num_01_var(); }

   [[nodiscard]] const Variable &get_variable(size_t var_idx) const;

   [[nodiscard]] const std::vector<std::vector<std::vector<VarID>>> &
   get_teacher_has_lesson_var() const { return _teacher_has_lesson_var; }

   [[nodiscard]] const std::vector<std::vector<std::vector<VarID>>> &
   get_teacher_is_in_school_var() const { return _teacher_is_in_school_var; }

   [[nodiscard]] const std::vector<std::vector<std::vector<VarID>>> &
   get_requirement_var() const { return _requirement_var; }

   [[nodiscard]] const std::vector<std::vector<std::vector<std::vector<VarID>>>> &
   get_requirement_var_per_class() const { return _requirement_var_per_class; }

   [[nodiscard]] const std::vector<std::vector<std::vector<std::vector<VarID>>>> &
   get_requirement_var_per_teacher() const { return _requirement_var_per_teacher; }

   [[nodiscard]] const std::vector<std::vector<std::vector<VarID>>> &
   get_requirement_cons_var_from_hour() const { return _requirement_cons_var_from_hour; }

   [[nodiscard]] const std::vector<std::vector<VarID>> &
   get_day_weight_for_class() const { return _day_weight_for_class; }

   [[nodiscard]] const std::vector<std::vector<VarID>> &
   get_day_weight_for_class_sorted() const { return _day_weight_for_class_sorted; }

private:
   void reserve_containers_space();

   void create_teacher_has_lesson_var();

   void create_teacher_is_in_school_var();

   void create_requirement_var();

   void create_requirement_cons_var_from_hour();

   void create_day_weight_for_class();

   void create_day_weight_for_class_sorted();

   const Input &_input;
   std::vector<Variable> _variables;

   VarID _num_01_var;

   // one variable for each teacher saying whether he has lesson at that day
   std::vector<std::vector<std::vector<VarID>>> _teacher_has_lesson_var;
   // one variable for each teacher saying whether he has lessons before and after in the same day
   std::vector<std::vector<std::vector<VarID>>> _teacher_is_in_school_var;

   // there will be one variable for each requirement. Read as @p _requirement_var[req][day][h]
   std::vector<std::vector<std::vector<VarID>>> _requirement_var;
   // the variables in _requirement_var stored according to classes and teacher respectively
   // they should be read as @p _requirement_var_per_class[class][day][hour] and @p _requirement_var_per_teacher[teacher][day][hour]
   std::vector<std::vector<std::vector<std::vector<VarID>>>> _requirement_var_per_class;
   std::vector<std::vector<std::vector<std::vector<VarID>>>> _requirement_var_per_teacher;

   // _requirement_cons_var_from_hour[req][day][hour] hays whether the requirement req takes hours hour and hour+1 in day
   std::vector<std::vector<std::vector<VarID>>> _requirement_cons_var_from_hour;

   // the following are LP variables (not {0,1})
   // there will be one variable for each class x day, measuring the weight of that day
   std::vector<std::vector<VarID>> _day_weight_for_class;
   // same as before, but sorted. These will appear in the weight objective, with decreasing weight
   std::vector<std::vector<VarID>> _day_weight_for_class_sorted;
};


#endif //SCHEDULE_HIGHSCHOOL_VARIABLES_H
