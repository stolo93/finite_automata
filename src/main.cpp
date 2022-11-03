//
// Created by samuel on 7.10.2022.
//

#include "./headers/FiniteAutomata.h"
#include "./headers/args.h"

#include <fstream>
#include <iostream>

int main(int argc, char ** argv)
{
    args::ArgumentParser parser ( "Finite automata" );
    args::HelpFlag help ( parser, "help", "help", {'h', "help"} );

    args::Group commands ( parser, "Availible commands:" );
        args::Command print ( commands, "print", "Concatenate all given files containing finite automata" );
        args::Command universal ( commands, "universal", "Check universality of all given finite automata" );
        args::Command inclusion ( commands, "inclusion", "Check whether the language defined by FA1 is a subset of language defined by FA2. FA1 and FA2 are finite automata given as CL arguments" );

    args::Group inclusion_flags ( parser, "Mantain exclusivity while using these flags. It only makes sense to use these with commands: \"universal\"", args::Group::Validators::AtMostOne );
        args::Flag simulation ( inclusion_flags, "Simulation", "Use simulation relation", {'s',"simulation"} );
        args::Flag identity ( inclusion_flags, "Identity", "Use identity relation (default option)", {'i',"identitiy"} );

    args::Group arguments ( parser, "Command arguments:", args::Group::Validators::DontCare, args::Options::Global );
        args::PositionalList<std::string> pathsList ( arguments, "paths", ".vtf files containing finite automata" );

    try
    {
        parser.ParseCLI(argc, argv);

        if ( print )
        {
            for ( auto &&path : pathsList )
            {
                std::ifstream file (path);
                auto nfa = FA::FiniteAutomata();
                nfa.Get(file);
                nfa.Print(std::cout);
                std::cout << std::endl;
                file.close();
            }

        }

        else if ( universal )
        {
            for ( auto && path : pathsList )
            {
                std::ifstream file (path);
                auto nfa = FA::FiniteAutomata();
                nfa.Get(file);

                FA::Relation_t relation;
                if ( simulation )
                {
                    relation = nfa.MaxSimulation();
                }

                else
                {
                    relation = nfa.IdentityRelation();
                }

                std::cout << path << " " << nfa.isUniversal(relation) << std::endl;
                file.close();
            }
        }

        else if ( inclusion )
        {
            //Get the first 2 files from arg list
            auto path_it = pathsList.begin();
            if ( path_it == pathsList.end() || path_it + 1 == pathsList.end() || path_it + 2 != pathsList.end() )
            {
                std::cout << "Exactly two files required as an argument" << std::endl;
                throw args::Help( "inclusion" );
            }
            //Open the files
            std::ifstream file1(*path_it);
            std::ifstream file2(*(path_it + 1));

            //Load finite automata
            FA::FiniteAutomata nfa1, nfa2;
            nfa1.Get(file1);
            nfa2.Get(file2);

            //Compute union and its relation
            auto union_automaton = nfa1.Union(nfa2);
            FA::Relation_t relation;

            if ( simulation )
            {
                relation = union_automaton.MaxSimulation();
            }
            else
            {
                relation = union_automaton.IdentityRelation();
            }

            // Check language inclusion
            std::cout << *path_it << " <= " << *(path_it + 1) << " :" << nfa1.isIncluded(nfa2, relation) << std::endl;
            delete [] relation;
        }
    }

    catch ( args::Help )
    {
        std::cout << parser;
    }
    catch ( args::Error &e )
    {
        std::cerr << e.what() << std::endl << parser;
        return 1;
    }

    return 0;
}
