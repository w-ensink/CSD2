// Written by Wouter Ensink

#include <iostream>


// Fun C++17 feature: Fold Expression combined with variadic templates
// makes for a cool Python like print function that works with any 
// iostreamable type...
template <typename... IOStreamableTypes>
auto print (IOStreamableTypes&&... arguments)
{
    ([] (auto&& arg) 
    { 
        std::cout << arg << ' '; 
    } 
    (std::forward<IOStreamableTypes> (arguments)), ...);

    std::cout << '\n';
}


int main()
{
    print ("Hello,", "World!");
    print ("The meaning of life is:", 42);
    return 0;
}