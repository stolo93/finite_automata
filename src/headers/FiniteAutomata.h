//
// Created by samuel on 7.10.2022.
//

#pragma once

#include <unordered_map>
#include <set>
#include <string>


namespace FA {

    using StateType = unsigned;
    using Transition_t = std::unordered_map<std::string, std::set<StateType>>;
    using Relation_t = bool **;
    using ProductState_t = std::pair<StateType, std::set<StateType>>;

    class TransitionFunction {
    public:
        FA::Transition_t * AllTransitions;
        StateType m_StatesCount;
        std::set<std::string> * m_Alphabet;

        TransitionFunction();
        explicit TransitionFunction(StateType Size, std::set<std::string> *Alphabet);

        ~TransitionFunction();

        /**
         * Initialize object returned by the default constructor
         * Nothing will happen if the object is already used
         * @param Size
         */
        void TransitionFunctionInit(StateType Size, std::set<std::string> *Alphabet);

        /**
         *
         * @return True if the object is uninitialized, false otherwise
         */
        bool Empty() const;

        /**
         * Insert transition from @p state1 to @p state2 on @p symbol
         *
         * If called on uninitialized TransitionFunction object, nothing will be added
         * @param state1
         * @param symbol
         * @param state2
         *
         * @return True if transition was successfully added, false otherwise
         */
        bool InsertTransition(StateType state1, const std::string &symbol, StateType state2);

        /**
         * Compute reversal of the this transition function
         *
         * @return Initialized object containing the reversed TransitionFunction
         */
        TransitionFunction* Revert() const;

    }; //class TransitionFunction




    class FiniteAutomata {
    public:
        FiniteAutomata();

        StateType Size() const;

        size_t AlphabetSize() const;

        void Get(std::ifstream  &stream);

        void Print(std::ostream &stream);

        Relation_t MaxSimulation();

        Relation_t MaxSimulation_simlib();

        Relation_t IdentityRelation();

        bool isUniversal( FA::Relation_t relation );

        bool isIncluded( const FA::FiniteAutomata &another_automaton, FA::Relation_t relation );

        void PrintRelation(std::ostream &stream, Relation_t Relation);

        FA::FiniteAutomata Union(const FA::FiniteAutomata &another_automaton);

    protected:
        //Data members
        std::string Name;
        std::unordered_map<std::string, StateType> States;
        std::unordered_map<StateType, std::string> StatesDictionary;
        std::unordered_map<std::string, size_t> AlphabetMap;
        std::unordered_map<size_t, std::string> AlphabetDictionary;
        std::set<std::string> Alphabet;
        std::set<StateType> StartStates;
        std::set<StateType> FinalStates;
        FA::TransitionFunction TFunction;

        StateType StatesCount;
        size_t SymbolCount;

        /**
         * @brief Prepare file containing finite automaton for parsing
         *  -removes excessive whitespaces
         *  -removes comments
         *
         * @param src_file
         * @return stringstream containing finite automaton
         */
        static std::stringstream prepare_file(std::ifstream &src_file);

        static std::string remove_comments(std::string &line);

        static std::string remove_whitespaces(std::string &line, const std::string &whitespace);

        void print_all_states(std::ostream &stream);

        void print_start_states(std::ostream &stream);

        void print_accept_states(std::ostream &stream);

        void print_alphabet(std::ostream &stream);

        void print_tfunc(std:: ostream &stream, FA::TransitionFunction &Tfunc);
        /**
         * @brief Add state into the FA
         * Adds to both States and StatesDictionary to keep consistency between these data structures
         *
         * @param state
         * @param index
         * @return True if state was successfully added
         */
        bool InsertState(std::string &state, StateType index);

        bool InsertSymbol(std::string &symbol, size_t &index);

        static std::set<StateType> Minimize ( const std::set<StateType> &State, Relation_t &Simulation);

        static std::set<StateType> Post ( std::set<StateType> &State, TransitionFunction &T_Function, std::string &symbol );

        static std::set<StateType> StatePost ( StateType State, TransitionFunction &T_Function , std::string &symbol );

        /**
         * @brief Get next token in @p line ,tokens are separated by characters in @p whitespace
         *
         * @param line
         * @param whitespace
         * @return Next token in the line or std::string::npos if there are no tokens left in the @p line
         */
        static bool get_token(std::string &line, std::string &token);
    };//FiniteAutomata

} // FA
