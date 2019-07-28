#ifndef SCHEDULE_HIGHSCHOOL_LP_PROVIDER_H
#define SCHEDULE_HIGHSCHOOL_LP_PROVIDER_H

#include <vector>
#include "Input.h"
#include "Variables.h"

struct VarIdxCoeffPair {
   Variables::VarID var_idx;
   double coeff;

   explicit VarIdxCoeffPair(Variables::VarID var_idx_, double coeff_ = 1.0) : var_idx{var_idx_}, coeff{coeff_} {}
};

class LP_Provider {
public:
   typedef Variables::VarID VarID;
   enum Direction {
      Min, Max, Feasible
   };
   enum Relation {
      Leq, Eq, Geq
   };

   struct Objective {
      std::vector<VarIdxCoeffPair> lin_vec;
      Direction direction;

      explicit Objective(Direction direction_) : direction{direction_} {}
   };

   struct Constraint {
      std::vector<VarIdxCoeffPair> lhs;
      int rhs;
      Relation rel;

      explicit Constraint(Relation rel_ = Eq, int rhs_ = 0) : rhs{rhs_}, rel{rel_} {}
   };

   LP_Provider(const Input &input_, const Variables &variables_, Direction objective_dir_);

   [[nodiscard]] const Variables &get_variables() const { return _variables; }

   [[nodiscard]] const Variables::Variable &get_variable(size_t var_idx) const {
      return _variables.get_variable(var_idx);
   }

   [[nodiscard]] const Objective &get_objective() const { return _objective; }

   [[nodiscard]] Direction get_objective_direction() const { return _objective.direction; }

   [[nodiscard]] size_t num_constraints() const { return _constraints.size(); }

   [[nodiscard]] const std::vector<Constraint> &get_constraints() const { return _constraints; }

   [[nodiscard]] const Constraint &get_constraint(size_t constr_idx) const;

private:
   void create_objective();

   void create_constraints();

   [[nodiscard]] size_t guess_num_constraints() const;

   void create_teacher_available_constraints();
   void create_teacher_has_lesson_constraints();
   void create_teacher_is_in_school_constraints();
   void create_class_sovrapposition_constraints();
   void create_num_lessons_constraints();
   void prevent_non_consecutive_hours();
   void create_cons_var_constraints();
   void create_day_weight_constraints();
   void create_day_weight_sorted_constraints();

   // all the possible subsets of [0, Input::NUM_DAYS_IN_WEEK) of cardinality @p cardinality
   void initialize_sorted_subsets();

   std::vector<std::vector<std::vector<unsigned int>>> _sorted_subsets;
   const Input &_input;
   const Variables &_variables;
   Objective _objective;
   std::vector<Constraint> _constraints;
};


#endif //SCHEDULE_HIGHSCHOOL_LP_PROVIDER_H
