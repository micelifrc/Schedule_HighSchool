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

   [[nodiscard]] const std::vector<Variable> &variables() const { return _variables; }

   [[nodiscard]] VarID num_var() const { return _variables.size(); }



private:
   void reserve_containers_space();

   void create_teacher_is_in_school_var();

   void create_requirement_var();

   void create_requirement_cons_var_from_hour();

   void create_requirement_cons_var();

   void create_day_weight_for_class();

   void create_day_weight_for_class_sorted();

   const Input &_input;
   std::vector<Variable> _variables;

   // one variable for each teacher saying whether he has classes before and after in the same day
   std::vector<std::vector<std::vector<VarID>>> _teacher_is_in_school_var;

   // there will be one variable for each requirement. Read as @p _requirement_var[req][day][h]
   std::vector<std::vector<std::vector<VarID>>> _requirement_var;

   // there will be one variable for each requirement, saying whether there are consecutive hours on that day.
   // Read as @p _requirement_cons_var[req][day] = max{@p _requirement_cons_var_from_hour[req][day][hour]
   // Are only introduces with !req.allow_extra_pairs
   std::vector<std::vector<std::vector<VarID>>> _requirement_cons_var_from_hour;
   std::vector<std::vector<VarID>> _requirement_cons_var;

   // the following are LP variables (not {0,1})
   // there will be one variable for each class x day, measuring the weight of that day
   std::vector<std::vector<VarID>> _day_weight_for_class;
   // same as before, but sorted. These will appear in the weight objective, with decreasing weight (probably @p _day_weight_for_class_sorted[c][d] will have weight 1-d/(NUM_DAYS-1))
   std::vector<std::vector<VarID>> _day_weight_for_class_sorted;
};


#endif //SCHEDULE_HIGHSCHOOL_VARIABLES_H
