#ifndef SCHEDULE_HIGHSCHOOL_INPUT_H
#define SCHEDULE_HIGHSCHOOL_INPUT_H

#include <iostream>
#include <vector>
#include <array>
#include <unordered_map>

struct Hour {
   unsigned int week_day;
   unsigned int hour;

   explicit Hour(unsigned int week_day_ = 0, unsigned int hour_ = 0) : week_day{week_day_}, hour{hour_} {}
};

class Input {
public:
   typedef int ID;  // identifies a teacher or a class
   static constexpr int MAX_ID = 4096;  // 2^12;
   static constexpr unsigned int NUM_DAYS_PER_WEEK = 6;
   static constexpr std::array<unsigned int, NUM_DAYS_PER_WEEK> NUM_HOURS_PER_DAY{6, 6, 6, 6, 6, 5};

   static unsigned int total_num_hours_in_week();

   explicit Input(std::istream &is);

   struct Class {
      ID id;  // in interval [0, max_ID)
      std::string name;
      std::array<unsigned int, NUM_DAYS_PER_WEEK> num_hours_per_day;

      explicit Class(const std::string &input);

      static constexpr char input_signal = 'c';
   };

   struct Teacher {
      ID id;  // in interval [0, MAX_ID^2) and multiple of MAX_ID
      std::string name;
      std::vector<std::vector<int>> penalties;

      explicit Teacher(const std::string &input);

      static constexpr char input_signal = 't';
   };

   struct Requirement {
      ID id;  // in interval [0, MAX_ID^2)
      unsigned int num_hours;
      unsigned int num_days_with_cons_hours;
      bool allow_extra_pairs;

      explicit Requirement(const std::string &input);

      ID teacher_id() const { return to_teacher_id(id); }

      ID class_id() const { return to_class_id(id); }

      static constexpr char input_signal = 'r';
   };

   static ID to_requirement_id(ID teacher_id, ID class_id) { return MAX_ID * teacher_id + class_id; }

   static ID to_teacher_id(ID requirement_id) { return requirement_id / MAX_ID; }

   static ID to_class_id(ID requirement_id) { return requirement_id % MAX_ID; }

   const std::vector<Class> classes() const { return _classes; }

   const std::vector<Teacher> teachers() const { return _teachers; }

   const std::vector<Requirement> requirements() const { return _requirements; }

   const unsigned int num_classes() const { return _classes.size(); }

   const unsigned int num_teachers() const { return _teachers.size(); }

   const unsigned int num_requirements() const { return _requirements.size(); }

   const Class *find_class(ID id) const;

   const Teacher *find_teacher(ID id) const;

   const Requirement *find_requirement(ID requirement_id) const;

   const Requirement *find_requirement(ID teacher_id, ID class_id) const;

private:
   bool add_class(const std::string &input);

   bool add_teacher(const std::string &input);

   bool add_requirement(const std::string &input);

   void check_indices() const;

   std::vector<Class> _classes;
   std::vector<Teacher> _teachers;
   std::vector<Requirement> _requirements;
   std::unordered_map<unsigned int, unsigned int> _class_id_map;
   std::unordered_map<unsigned int, unsigned int> _teacher_id_map;
   std::unordered_map<unsigned int, unsigned int> _requirement_id_map;
};


#endif //SCHEDULE_HIGHSCHOOL_INPUT_H
