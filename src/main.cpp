//
// Created by samuel on 7.10.2022.
//

#include "headers/FiniteAutomata.h"
#include <fstream>
#include <iostream>

int main(int argc, char ** argv)
{
    std::ifstream input_file1(argv[1], std::ifstream::in);
    std::ifstream input_file2(argv[2], std::ifstream::in);

    std::cout << argv[1] << " " << argv[2] << std::endl;

    FA::FiniteAutomata nfa1, nfa2;

    nfa1.Get(input_file1);
    nfa2.Get(input_file2);

    std::cout << "Is included: " << nfa1.isIncluded(nfa2) << std::endl;

    return 0;
}