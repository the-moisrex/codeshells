import standard;

#include <print>

using namespace std;

// clang-format off
int main() {

    int a = 5;
    int b = 10;

    // Inline assembly block to swap the values of 'a' and 'b' using registers
    __asm__(
        // Move the value of 'a' into register ecx
        "movl %0, %%ecx;\n\t"
        // Move the value of 'b' into register edx
        "movl %1, %%edx;\n\t"
        // Move the value from ecx (originally 'a') into the memory location of 'b'
        "movl %%ecx, %1;\n\t"
        "movl %%edx, %0;\n\t"
        :
        // Output operands: tell the compiler which variables are written to
        "=m" (a), // Mark 'a' as an output operand (memory)
        "=m" (b)  // Mark 'b' as an output operand (memory)
        :
        // Input operands: tell the compiler which variables are read from
        "m" (a),  // Mark 'a' as an input operand (memory)
        "m" (b)   // Mark 'b' as an input operand (memory)
        :
        // Clobbered registers: tell the compiler which registers are modified
        "ecx",    // ecx is used and modified
        "edx"     // edx is used and modified
    );

    // Print the values of 'a' and 'b' after the assembly block
    println("a: {}", a); // Output the value of 'a'
    println("b: {}", b); // Output the value of 'b'

    return 0; // Indicate successful program termination
}
// clang-format on
