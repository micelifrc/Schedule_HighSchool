#include <fstream>
#include "Input.h"

int main() {
   std::ifstream input_stream;
   input_stream.open("input_example1.txt");
   Input input(input_stream);
   input_stream.close();
   return 0;
}