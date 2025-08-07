#include <iostream>
#include <cstdio>

int main(int argc, char** argv) {
    printf("MINIMAL TEST: Hello World!\n");
    fflush(stdout);
    
    std::cout << "C++ cout: Hello World!" << std::endl;
    std::cout.flush();
    
    std::cout << "argc = " << argc << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }
    
    return 0;
}
