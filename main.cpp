#include <fstream>
#include "Input.h"
#include "LP_Provider.h"

int main() {
   std::ifstream input_stream;
   input_stream.open("input_example1.txt");
   Input input(input_stream);
   input_stream.close();
   Variables variables(input);
   LP_Provider lp_provider(input, variables, LP_Provider::Min);
   return 0;
}