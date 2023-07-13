#include <stddef.h>
#include <stdint.h>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <string_view>

const int kMinValidSize = 8; 

// The function we want to fuzz test
int target_function(const uint8_t* data, size_t size) {
	using namespace std;
  if (size < kMinValidSize) return 0;
  
  string buff{reinterpret_cast<const char*>(data), size};

  string_view sv = buff + " Hello world ";
  return sv.size();
}

// The fuzzing entry point    
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  target_function(data, size);
  return 0;
}
