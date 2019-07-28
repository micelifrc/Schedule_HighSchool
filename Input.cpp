//
// Created by mich on 16/07/19.
//

#include <sstream>
#include "Input.h"

unsigned int Input::total_num_hours_in_week = 0;

Input::Input(std::istream &is) {
   check_nonzero_day();
   read_file(is);
   check_indices();
   set_allow_extra_pairs();
   record_requirements();
}

Input::Class::Class(const std::string &input) : id{0}, num_hours_per_day{0, 0, 0, 0, 0, 0} {
   std::stringstream stream(input);
   char c;
   stream >> c >> id >> name;
   if (c != input_signal) {
      throw std::logic_error("The input string for Class is not a class string");
   }
   if (id <= 0 or id >= MAX_ID) {
      throw std::logic_error(
            "The index " + std::to_string(id) + " is not allowed. Indices should be in the interval [1," +
            std::to_string(MAX_ID) + ")");
   }
   for (unsigned int day = 0; day != NUM_DAYS_PER_WEEK; ++day) {
      if (stream.rdbuf()->in_avail() <= 0) {
         throw std::logic_error(
               "Too few num_hours inputs for class" + name + ": required " + std::to_string(NUM_DAYS_PER_WEEK));
      }
      stream >> num_hours_per_day[day];
   }
   if (stream.rdbuf()->in_avail() > 0) {
      throw std::logic_error(
            "Too many num_hours inputs for class" + name + ": required " + std::to_string(NUM_DAYS_PER_WEEK));
   }
}

Input::Teacher::Teacher(const std::string &input) : id{0}, penalties(NUM_DAYS_PER_WEEK), num_days_available{0} {
   std::stringstream stream(input);
   char c;
   stream >> c >> id >> name;
   if (c != input_signal) {
      throw std::logic_error("The input string for Teacher is not a teacher string");
   }
   if (id <= 0 or id >= MAX_ID) {
      throw std::logic_error(
            "The index " + std::to_string(id) + " is not allowed. Indices should be in the interval [1," +
            std::to_string(MAX_ID) + ")");
   }
   id *= MAX_ID;  // so it is different from the class id
   unsigned int sum = 0;
   for (unsigned int day = 0; day != NUM_DAYS_PER_WEEK; ++day) {
      bool add_day = false;
      penalties[day].resize(NUM_HOURS_PER_DAY[day], 0);
      for (int &hour: penalties[day]) {
         if (stream.rdbuf()->in_avail() <= 0) {
            throw std::logic_error(
                  "Too few penalty inputs for teacher" + name + ": required " +
                  std::to_string(total_num_hours_in_week));
         }
         stream >> hour;
         if (hour < 0) {
            hour = std::numeric_limits<int>::max();
         } else {
            add_day = true;
            sum += hour;
         }
      }
      if (add_day) {
         ++num_days_available;
      }
   }
   if (stream.rdbuf()->in_avail() > 0) {
      throw std::logic_error(
            "Too many penalty inputs for teacher" + name + ": required " + std::to_string(total_num_hours_in_week));
   }
   if (sum > 50) {
      throw std::logic_error("Teacher " + name + " sum of penalties is > 50");
   }
}

Input::Requirement::Requirement(const std::string &input)
      : id{0}, num_days_with_cons_hours{0}, allow_extra_pairs{false}, average_lesson_weight{0.0} {
   std::stringstream stream(input);
   char c;
   std::string s;
   int teacher_id, class_id;
   stream >> c >> teacher_id >> class_id >> s;
   if (c != input_signal) {
      throw std::logic_error("The input string for Requirement is not a teacher string");
   }
   if (teacher_id <= 0 or teacher_id >= MAX_ID) {
      throw std::logic_error(
            "The index " + std::to_string(teacher_id) + " is not allowed. Indices should be in the interval [1," +
            std::to_string(MAX_ID) + ")");
   }
   if (class_id <= 0 or class_id >= MAX_ID) {
      throw std::logic_error(
            "The index " + std::to_string(class_id) + " is not allowed. Indices should be in the interval [1," +
            std::to_string(MAX_ID) + ")");
   }
   id = teacher_id * MAX_ID + class_id;
   if (stream.rdbuf()->in_avail() > 0) {
      stream >> num_days_with_cons_hours;
   }
   int counter = 0;
   for (unsigned int idx = 0; idx != s.length(); ++idx) {
      if (s[idx] >= 'A' and s[idx] <= 'Z') {
         lessons += std::string(std::max(counter, 1), s[idx]);
         counter = 0;
      } else if (s[idx] >= 'a' and s[idx] <= 'z') {
         lessons += std::string(std::max(counter, 1), s[idx] - 'a' + 'A');
         counter = 0;
      } else if (s[idx] >= '0' and s[idx] <= '9') {
         counter = 10 * counter + s[idx] - '0';
      }
   }
   for(char l: lessons) {
      average_lesson_weight += weight_lesson(l);
   }
   average_lesson_weight /= num_lessons();
   if (num_days_with_cons_hours > NUM_DAYS_PER_WEEK) {
      throw std::logic_error("Requirement with " + std::to_string(num_days_with_cons_hours) + " consecutive hours");
   }
   if (2 * num_days_with_cons_hours > num_lessons()) {
      throw std::logic_error("Requirement with more consecutive days than hours");
   }
}


const Input::Class *Input::find_class(ID id) const {
   const auto &map_point = _class_id_map.find(id);
   return map_point == _class_id_map.end() ? nullptr : &_classes[map_point->second];
}

const Input::Teacher *Input::find_teacher(ID id) const {
   const auto &map_point = _teacher_id_map.find(id);
   return map_point == _teacher_id_map.end() ? nullptr : &_teachers[map_point->second];
}

const Input::Requirement *Input::find_requirement(ID requirement_id) const {
   const auto &map_point = _requirement_id_map.find(requirement_id);
   return map_point == _requirement_id_map.end() ? nullptr : &_requirements[map_point->second];
}

const Input::Requirement *Input::find_requirement(ID teacher_id, ID class_id) const {
   return find_requirement(to_requirement_id(teacher_id, class_id));
}

Input::Class *Input::find_class(ID id) {
   const auto &map_point = _class_id_map.find(id);
   return map_point == _class_id_map.end() ? nullptr : &_classes[map_point->second];
}

Input::Teacher *Input::find_teacher(ID id) {
   const auto &map_point = _teacher_id_map.find(id);
   return map_point == _teacher_id_map.end() ? nullptr : &_teachers[map_point->second];
}

Input::Requirement *Input::find_requirement(ID requirement_id) {
   const auto &map_point = _requirement_id_map.find(requirement_id);
   return map_point == _requirement_id_map.end() ? nullptr : &_requirements[map_point->second];
}

Input::Requirement *Input::find_requirement(ID teacher_id, ID class_id) {
   return find_requirement(to_requirement_id(teacher_id, class_id));
}

bool Input::add_class(const std::string &input) {
   if (input.empty() or input[0] != Class::input_signal) {
      return false;
   }
   Class new_class(input);
   const Class *other = find_class(new_class.id);
   if (other != nullptr) {
      if (other->name != new_class.name) {
         throw std::logic_error(
               "Classes " + other->name + " and " + new_class.name + " both have id " + std::to_string(new_class.id));
      }
      for (unsigned int day = 0; day != NUM_DAYS_PER_WEEK; ++day) {
         if (other->num_hours_per_day[day] != new_class.num_hours_per_day[day]) {
            throw std::logic_error("Double definition of class " + new_class.name);
         }
      }
   }
   if (other == nullptr) {
      _class_id_map.emplace(new_class.id, num_classes());
      _classes.push_back(new_class);
      return true;
   }
   return false;
}

bool Input::add_teacher(const std::string &input) {
   if (input.empty() or input[0] != Teacher::input_signal) {
      return false;
   }
   Teacher new_teacher(input);
   const Teacher *other = find_teacher(new_teacher.id);
   if (other != nullptr) {
      if (other->name != new_teacher.name) {
         throw std::logic_error("Classes " + other->name + " and " + new_teacher.name + " both have id " +
                                std::to_string(new_teacher.id / MAX_ID));
      }
      for (unsigned int day = 0; day != NUM_DAYS_PER_WEEK; ++day) {
         for (unsigned int hour = 0; hour != NUM_HOURS_PER_DAY[day]; ++hour) {
            if (other->penalties[day][hour] != new_teacher.penalties[day][hour]) {
               throw std::logic_error("Double definition of teacher " + new_teacher.name);
            }
         }

      }
   }
   if (other == nullptr) {
      _teacher_id_map.emplace(new_teacher.id, num_teachers());
      _teachers.push_back(new_teacher);
      return true;
   }
   return false;
}


bool Input::add_requirement(const std::string &input) {
   if (input.empty() or input[0] != Requirement::input_signal) {
      return false;
   }
   Requirement new_requirement(input);
   const Requirement *other = find_requirement(new_requirement.id);
   if (other != nullptr) {
      if (other->num_lessons() != new_requirement.num_lessons()) {
         throw std::logic_error(
               "The number of hours for teacher " + std::to_string(new_requirement.teacher_id() / MAX_ID) +
               " for class " + std::to_string(new_requirement.class_id()) + " is double defined");
      }
      if (other->num_days_with_cons_hours != new_requirement.num_days_with_cons_hours) {
         throw std::logic_error(
               "The number of cons_days for teacher " + std::to_string(new_requirement.teacher_id() / MAX_ID) +
               " for class " + std::to_string(new_requirement.class_id()) + " is double defined");
      }
   }
   if (other == nullptr) {
      _requirement_id_map.emplace(new_requirement.id, num_requirements());
      _requirements.push_back(new_requirement);
      return true;
   }
   return false;
}

void Input::check_nonzero_day() const {
   for (unsigned int day = 0; day != NUM_DAYS_PER_WEEK; ++day) {
      if (NUM_HOURS_PER_DAY[day] == 0) {
         throw std::logic_error("Day has zero hours");
      }
      total_num_hours_in_week += NUM_HOURS_PER_DAY[day];
   }
}

void Input::read_file(std::istream &is) {
   std::string line;

   auto cut_line_extrema = [&]() -> std::string {
      unsigned int first_character;
      for (first_character = 0; first_character < line.length() and
                                (line[first_character] == ' ' or line[first_character] == '\t' or
                                 line[first_character] == '\n'); ++first_character) {}
      if (first_character == line.length()) {
         return std::string();
      }
      unsigned int last_character = first_character;
      for (unsigned int last_char_cand = first_character; last_char_cand != line.length(); ++last_char_cand) {
         if (line[last_char_cand] != ' ' and line[last_char_cand] != '\t' and line[last_char_cand] != '\n') {
            last_character = last_char_cand;
         }
      }
      return line.substr(first_character, last_character + 1 - first_character);
   };

   while (getline(is, line)) {
      std::string cut_input_line = cut_line_extrema();
      if (not cut_input_line.empty()) {
         switch (cut_input_line[0]) {
            case Class::input_signal: {
               add_class(cut_input_line);
               break;
            }
            case Teacher::input_signal: {
               add_teacher(cut_input_line);
               break;
            }
            case Requirement::input_signal: {
               add_requirement(cut_input_line);
               break;
            }
         }
      }
   }
}

void Input::check_indices() const {
   for (const Class &cl: _classes) {
      unsigned int residual_hours = 0;
      for (unsigned int hour_in_day: cl.num_hours_per_day) {
         residual_hours += hour_in_day;
      }
      if (residual_hours == 0) {
         throw std::logic_error("Class " + cl.name + " has 0 hours on every day");
      }
      for (const Requirement &req: _requirements) {
         if (req.class_id() == cl.id) {
            residual_hours -= req.num_lessons();
         }
      }
      if (residual_hours != 0) {
         throw std::logic_error("Class " + cl.name + " has the wrong number of total hours");
      }
   }

   for (const Teacher &tc: _teachers) {
      bool has_req = false;
      for (const Requirement &req: _requirements) {
         if (req.teacher_id() == tc.id) {
            has_req = true;
            break;
         }
      }
      if (not has_req) {
         throw std::logic_error("Teacher " + tc.name + " doesn't have any class");
      }
   }

   for (const Requirement &req: _requirements) {
      if (find_teacher(req.teacher_id()) == nullptr) {
         throw std::logic_error(
               "Requirement for teacher id " + std::to_string(req.teacher_id()) + " for non-existing teacher id");
      }
      if (find_class(req.class_id()) == nullptr) {
         throw std::logic_error(
               "Requirement for class id " + std::to_string(req.class_id()) + " for non-existing class id");
      }
   }
}

void Input::set_allow_extra_pairs() {
   for (Requirement &req: _requirements) {
      const Teacher *teacher = find_teacher(req.teacher_id());
      if (teacher == nullptr) {
         throw std::logic_error(
               "The teacher " + std::to_string(req.teacher_id() / MAX_ID) + " does not exist but has request");
      }
      req.allow_extra_pairs = (req.num_lessons() - req.num_days_with_cons_hours > teacher->num_days_available);
   }
}

void Input::record_requirements() {
   for (unsigned int requirement_pos = 0; requirement_pos != num_requirements(); ++requirement_pos) {
      const Requirement &requirement = _requirements[requirement_pos];
      Teacher *teacher = find_teacher(requirement.teacher_id());
      if (teacher == nullptr) {
         throw std::logic_error(
               "The teacher " + std::to_string(requirement.teacher_id() / MAX_ID) + " does not exist but has request");
      }
      teacher->requirements.emplace_back(requirement_pos);
      Class *school_class = find_class(requirement.class_id());
      if (school_class == nullptr) {
         throw std::logic_error(
               "The class " + std::to_string(requirement.class_id()) + " does not exist but has request");
      }
      school_class->requirements.emplace_back(requirement_pos);
   }
}

double Input::weight_lesson(char l) {
   switch (l) {
      case 'A': {  // Art
         return 1.0;
      }
      case 'B': {  // Other foreign language (different from English)
         return 4.0;
      }
      case 'C': {  // Informatics / computer science
         return 3.0;
      }
      case 'E': {  // English
         return 4.0;
      }
      case 'F': {  // Philosophy
         return 2.0;
      }
      case 'G': {  // Greek
         return 5.0;
      }
      case 'H': {  // History
         return 2.0;
      }
      case 'I': {  // Italian
         return 3.0;
      }
      case 'J': {  // Gymnastic (Physical Education)
         return 0.0;
      }
      case 'L': {  // Latin
         return 5.0;
      }
      case 'M': {  // Mathematics
         return 5.0;
      }
      case 'O': {  // Geography
         return 2.0;
      }
      case 'P': {  // Physics
         return 4.0;
      }
      case 'R': {  // Religion
         return 0.0;
      }
      case 'S': {  // Science
         return 2.0;
      }
      default: {
         throw std::logic_error("Invalid lesson character " + std::to_string(l));
      }
   }
}
