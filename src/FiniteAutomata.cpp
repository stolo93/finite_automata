//
// Created by samuel on 7.10.2022.
//

#include "headers/FiniteAutomata.h"
#include <iostream>
#include <sstream>
#include <fstream> //getline doesn't work without this for some reason
#include <set>
#include <iomanip>
#include <stdlib.h> //exit
#include <vector>

using namespace std;

//TransitionFunction
namespace FA
{
   TransitionFunction::TransitionFunction()
    {
        m_StatesCount = 0;
        AllTransitions = nullptr;
        m_Alphabet = nullptr;
    }

    TransitionFunction::TransitionFunction(StateType Size, std::set<std::string> *Alphabet)
    {
        m_Alphabet = Alphabet;
        m_StatesCount = Size;
        AllTransitions = new Transition_t[Size];
    }

    void TransitionFunction::TransitionFunctionInit(StateType Size, std::set<std::string> *Alphabet)
    {
        if (!this->Empty()) //In case of an already initialized transition function
        {
            return;
        }

        m_StatesCount = Size;
        m_Alphabet = Alphabet;
        AllTransitions = new Transition_t [m_StatesCount];
    }

    TransitionFunction::~TransitionFunction()
    {
        if (!this->Empty())
        {
            delete[] AllTransitions;
        }
    }

    bool TransitionFunction::Empty() const
    {
        return this->AllTransitions == nullptr;
    }

    TransitionFunction* TransitionFunction::Revert()const
    {
        if (m_StatesCount == 0)
        {
            auto ret_val = new TransitionFunction();
            return ret_val;
        }

        auto reversion = new TransitionFunction(m_StatesCount, m_Alphabet);
        for (StateType p = 0; p < m_StatesCount; p++)
        {
            auto cur_transition = this->AllTransitions[p]; //All transitions from state p
            for (const auto &symbol : *m_Alphabet)
            {
                if (cur_transition.count(symbol)) //Checking whether there is an outgoing transition on "symbol"
                {
                    for (auto p1 : cur_transition.at(symbol))
                    {
                        reversion->InsertTransition(p1, symbol, p);
                    }
                }
            }
        }
        return reversion;
    }

    bool TransitionFunction::InsertTransition(const StateType state1, const string &symbol, StateType state2)
    {
        if (this->Empty())
        {
            return false;
        }

        auto transition = &AllTransitions[state1];

        //In case there is no existing transition from state1 on symbol
        if ((*transition).count(symbol) == 0)
        {
            set<StateType> new_states;
            new_states.insert(state2);

            (*transition).emplace(symbol, new_states);
        }
        else
        {
            (*transition).at(symbol).insert(state2);
        }

        return true;
    }

} // FA

//FiniteAutomata
namespace FA
{
    FiniteAutomata::FiniteAutomata()
    {
        StatesCount = 0;
        SymbolCount = 0;
        TFunction = TransitionFunction();
    }

    StateType FiniteAutomata::Size() const
    {
        return StatesCount;
    }

    size_t FiniteAutomata::AlphabetSize() const
    {
        return SymbolCount;
    }

    void FiniteAutomata::Get(ifstream& stream)
    {
        stringstream input = prepare_file(stream);
        stringstream line_buff;
        string line;
        string cur_token;
        bool type_read = false;

        while(getline(input, line))
        {
            //skip the FA type
            if (line[0] == '@')
            {
                if (type_read)
                {
                    input << line;
                    break;
                }
                type_read = true;
                continue;
            }

            //Store FA's name, |"%Name"| == 5
            else if (line.substr(0,5) == "%Name")
            {
                Name = line.substr(5);
                continue;
            }

            //All states of the FA, |"%States"| == 7
            else if (line.substr(0,7) == "%States")
            {
                line = line.substr(8);
                while (get_token(line, cur_token))
                {
                    if (InsertState(cur_token, StatesCount)) //In case state was successfully added to the map increment the states counter
                    {
                        StatesCount++;
                    }
                }
                continue;
            }

            //IMPORTANT Initial and final states have to be postponed until there are no more states which could be added

            //Accept states of the FA, |"%Final"| == 6 || Initial states of the FA, |"%Initial"| == 8
            else if (line.substr(0,6) == "%Final" || line.substr(0,8) == "%Initial")
            {
                line_buff << line << endl;
                continue;
            }

            //Alphabet of the FA, |"%Alphabet"| == 9
            else if (line.substr(0,9) == "%Alphabet")
            {
                line = line.substr(10);
                while (get_token(line, cur_token))
                {
                    Alphabet.insert(cur_token);
                }
                continue;
            }

            //Empty line
            else if (line.empty())
            {
                continue;
            }

            else
            {
                if (TFunction.Empty()) // First line of the Transition function
                {
                    TFunction.TransitionFunctionInit(StatesCount, &Alphabet);
                }

                string state1, state2, symbol;
                get_token(line, state1);
                get_token(line, symbol);
                get_token(line,state2);

                Alphabet.insert(symbol);

                if (States.count(state1) == 0 || States.count(state2) == 0)
                {
                    cout << "\033[1;31mERROR: State not declared. Transition " << state1 << " -" << symbol << "-> " << state2 << " is not valid.\033[0m\n";
                    exit(1);
                }
                TFunction.InsertTransition(States.at(state1), symbol, States.at(state2));
            }
        }

        //Iterating through buffer of postponed Initial and Final states
        while (getline(line_buff, line))
        {
            //Initial states of the FA, |"%Initial"| == 8
            if (line.substr(0,8) == "%Initial")
            {
                line = line.substr(9);
                while (get_token(line, cur_token))
                {
                    StartStates.insert(States.at(cur_token));
                }
                continue;
            }

            //Accept states of the FA, |"%Final"| == 6
            else if (line.substr(0,6) == "%Final")
            {
                line = line.substr(7);
                while (get_token(line, cur_token))
                {
                    FinalStates.insert(States.at(cur_token));
                }
                continue;
            }
        }
    }

    stringstream FiniteAutomata::prepare_file(ifstream& src_file)
    {
        string line;
        stringstream cleaned_file;
        while(getline(src_file, line))
        {
            FiniteAutomata::remove_whitespaces(line, " \t");
            FiniteAutomata::remove_comments(line);
            cleaned_file << line << endl;
        }

        return cleaned_file;
    }

    string FiniteAutomata::remove_comments(string& line)
    {
        auto hash_pos = line.find('#');
        if (hash_pos == string::npos)
        {
            return line;
        } //No comment found

        return line.erase(hash_pos);
    }

    string FiniteAutomata::remove_whitespaces(string& line, const string& whitespace)
    {
        auto first_letter = line.find_first_not_of(whitespace);
        if (first_letter == string::npos)
        {
            return "";
        } //no content

        auto last_letter = line.find_last_not_of(whitespace);
        auto str_range = last_letter - first_letter + 1;

        line = line.substr(first_letter, str_range);
        return line;
    }

    bool FiniteAutomata::get_token(string& line, string& token)
    {
        if (line[0] == ' ')
        {
            line = line.substr(1);
        }

        stringstream line_copy(line);
        getline(line_copy, token, ' ');

        if (token.empty())
        {
            return false;
        }

        line = line.substr(token.length());
        return true;
    }

    bool FiniteAutomata::InsertState(string& state, StateType index)
    {
        auto retval = States.emplace(state, index);
        if (retval.second){
            StatesDictionary.emplace(index, state);
            return true;
        }
        return false;
    }

    void FiniteAutomata::Print(ostream& stream)
    {
        //Print name of the FA
        if (!Name.empty())
        {
            stream << "%Name " << Name << endl;
        }

        print_all_states(stream);
        print_alphabet(stream);
        print_start_states(stream);
        print_accept_states(stream);

        stream << endl;
        print_tfunc(stream, this->TFunction);
    }

    void FiniteAutomata::print_all_states(ostream& stream)
    {
        auto it = States.begin();
        stream << "%States";
        while(it != States.end())
        {
            stream << " " << (it++)->first;
        }
        stream << endl;
    }

    void FiniteAutomata::print_start_states(ostream& stream)
    {
        stream << "%Initial";
        for (auto state : StartStates)
        {
            stream << " " << StatesDictionary.at(state);
        }
        stream << endl;
    }

    void FiniteAutomata::print_accept_states(ostream& stream)
    {
        stream << "%Final";
        for (auto state : FinalStates)
        {
            stream << " " << StatesDictionary.at(state);
        }
        stream << endl;
    }

    void FiniteAutomata::print_alphabet(ostream& stream)
    {
        stream << "%Alphabet";
        for (const auto& symbol : Alphabet)
        {
            stream << " " << symbol;
        }
        stream << endl;
    }

    void FiniteAutomata::print_tfunc(std::ostream &stream, FA::TransitionFunction &Tfunc)
    {
        if (Tfunc.Empty())
        {
            return;
        }

        for (StateType state1 = 0; state1 < StatesCount; state1++)
        {
            auto transitionFromState = Tfunc.AllTransitions[state1];
            for (const auto &symbol : Alphabet)
            {
                if (transitionFromState.count(symbol) != 0)
                {
                    for (auto state2 : transitionFromState.at(symbol))
                    {
                        stream << StatesDictionary.at(state1) << " " << symbol << " " << StatesDictionary.at(state2) << endl;
                    }
                }
            }
        }
    }

    Relation_t FiniteAutomata::IdentityRelation()
    {
        auto relation = new bool * [StatesCount];
        for (StateType i = 0; i < StatesCount; i++)
        {
            relation[i] = new bool [StatesCount]();
            relation[i][i] = true;
        }

        return relation;
    }

    Relation_t FiniteAutomata::MaxSimulation()
    {
        if (StatesCount == 0)
        {
            return nullptr;
        }
        //Allocate and initialize relation, which will be returned
        auto sim_complement = new bool * [StatesCount]; //We will be computing the complement and at the end just make complement again to get the actual simulation
        for (StateType i = 0; i < StatesCount; i++)
        {
            sim_complement[i] = new bool [StatesCount](); //The relation is initially empty
        }

        //Beginning of the algorithm

        //Compute reverse transition function
        auto revTFunc = TFunction.Revert();
        std::set<pair<StateType, StateType>> worklist;

        //Initial refinement
        unordered_map<string,StateType **> Counters; //Counters of size states_count x states_count for each symbol from alphabet

        for (const auto &symbol : Alphabet)
        {
            //Create counter for current symbol
            auto cur_counter = &Counters[symbol];
            *cur_counter = new StateType * [StatesCount];
            for (StateType i = 0; i < StatesCount; i++)
            {
                (*cur_counter)[i] = new StateType [StatesCount]();
            }

            //Iterate through all states
            for (StateType p = 0; p < StatesCount; p++)
            {
                for (StateType q = 0; q < StatesCount; q++)
                {
                    //LINE 5 in the algorithm
                    //Initializing counters according to the algo
                    if (TFunction.AllTransitions[q].count(symbol) != 0) //Checking to prevent out of range indexing
                    {
                        StateType card = TFunction.AllTransitions[q].at(symbol).size(); // |delta(q, symbol)|
                        (*cur_counter)[p][q] = card;
                    }

                    //Condition for adding state pair into the worklist LINE 7
                    if ((FinalStates.count(p) != 0 && FinalStates.count(q) == 0) ||
                        (TFunction.AllTransitions[p].count(symbol) != 0 && TFunction.AllTransitions[q].count(symbol) == 0))
                    {
                        sim_complement[p][q] = true;
                        worklist.insert(make_pair(p,q)); //Worklist enqueue (p,q)
                    }
                }
            }
        } //End of initial refinement

        //Beginning of the algorithm
        StateType p1, q1;
        while (!worklist.empty()){
            //Worklist dequeue (p1,q1)
            auto cur_pair = worklist.begin();
            p1 = cur_pair->first; q1 = cur_pair->second;
            worklist.erase(cur_pair);

            for (const auto &symbol : Alphabet){ //Foreach symbol
                //In case there is no transition from q1 or p1 on symbol
                if (revTFunc->AllTransitions[q1].count(symbol) == 0 || revTFunc->AllTransitions[p1].count(symbol) == 0){
                    continue;
                }
                for (auto q : revTFunc->AllTransitions[q1].at(symbol)){
                    Counters.at(symbol)[p1][q] -= 1; //Decrement Counter_symbol [p1][q]
                    if (Counters.at(symbol)[p1][q] == 0){ // q can’t go over a above p
                        for (auto p : revTFunc->AllTransitions[p1].at(symbol)){
                            if ( ! sim_complement[p][q] ){
                                sim_complement[p][q] = true;
                            worklist.insert(make_pair(p,q));
                            }
                        }
                    }

                }
            }
        }// End of the main loop in the algorithm
        delete revTFunc;

        //Compute sim_complement complement
        for (StateType p = 0; p < StatesCount; p++){
            for (StateType q = 0; q < StatesCount; q++)
            {
                sim_complement[p][q] = !sim_complement[p][q];
            }
        }

        return sim_complement;
    }

    void FiniteAutomata::PrintRelation(ostream &stream, Relation_t Relation)
    {
        //print the header
        stream << std::setw(3);
        for (StateType i = 0; i < StatesCount; i++)
        {
            stream << " " << StatesDictionary.at(i);
        }
        stream << endl;

        for (StateType i = 0; i < StatesCount; i++)
        {
            stream << StatesDictionary.at(i);
            for (StateType j = 0; j < StatesCount; j++)
            {
                stream << " " << (int)Relation[i][j];
            }
            stream << endl;
        }
    }

    bool isSmaller (set<StateType> MState1, set<StateType> MState2, Relation_t Relation)
    {

        for (auto state1 : MState1)
        {
            bool exists_smaller = false;
            for (auto state2 : MState2)
            {
                if (Relation[state1][state2])
                {
                    exists_smaller = true;
                    break;
                }
            }

            if (!exists_smaller)
            {
                return false;
            }
        }

        return true;
    }


    bool isProductSmaller (ProductState_t S_productstate, ProductState_t P_productstate, Relation_t Relation)
    {
        if (Relation[P_productstate.first][S_productstate.first])
        {
            if (isSmaller(S_productstate.second, P_productstate.second, Relation))
            {
                return true;
            }
        }
        return false;
    }

    bool isAccepting(set<StateType> MacroState, set<StateType> FinalStates)
    {
        for (auto state : MacroState)
        {
            if (FinalStates.count(state))
            {
                return true;
            }
        }

        return false;
    }

    bool isProductAccepting(StateType stateA, set<StateType> StatesB, set<StateType> FinalStatesA, set<StateType> FinalStatesB)
    {
        //TODO change arguments to ProductState_t
        if (FinalStatesA.count(stateA) == 0)
        {
            return false;
        }

        if ( ! isAccepting(StatesB, FinalStatesB))
        {
            return true;
        }

        return false;
    }

    bool FiniteAutomata::isUniversal( FA::Relation_t relation )
    {

        //Check if start state is rejecting
        if ( !isAccepting(this->StartStates, this->FinalStates) )
        {
            return false;
        }

        set<set<StateType>> Processed = {};
        set<set<StateType>> Next = {};
        set<set<StateType>> PN_union = {};

        Next.emplace(FiniteAutomata::Minimize( this->StartStates, relation ));


        while ( ! Next.empty() )
        {
            auto R = *Next.begin();
            //Checks for existence of chain in the processed set
            bool exists_smaller = false;
            for (const auto& state : Processed)
            {
                if (isSmaller(state, R, relation))
                {
                    exists_smaller = true;
                    break;
                }
            }
            if (!exists_smaller)
            {
                Processed.emplace(R);
            }

            //LINE 6
            //TODO come up with some better comments
            for (auto symbol : this->Alphabet)
            {
                auto cur_post = Post(R, this->TFunction, symbol);
                auto  P_state = Minimize(cur_post, relation);

                //Preparations
                //TODO optimize this
                PN_union.clear();
                PN_union.insert(Processed.begin(), Processed.end());
                PN_union.insert(Next.begin(), Next.end());

                bool exists_smaller1 = false;
                for (auto S_state : PN_union)
                {
                    if (isSmaller(S_state, P_state, relation))
                    {
                        exists_smaller1 = true;
                        break;
                    }
                }
                //END OF PREPARATIONS


                //LINE 7
                if (!isAccepting(P_state, this->FinalStates))
                {
                    return false;
                }

                //LINE 8
                else if (!exists_smaller1)
                {
                    for (auto S_state : PN_union)
                    {
                        if (isSmaller(P_state, S_state, relation))
                        {
                            Processed.erase(S_state);
                            Next.erase(S_state);

                            Next.emplace(P_state);
                        }
                    }
                }
            }
        }

        return true;
    }

    void Initialize(std::set<ProductState_t> &PStates, Relation_t relation)
    {
        //Optimization 2
        for (auto pstate : PStates)
        {
            bool end_loop = false;
            for (auto p1 : pstate.second)
            {
                if (relation[pstate.first][p1])
                {
                    PStates.erase(pstate);
                    end_loop = true;
                    break;
                }
            }
            if (end_loop)
            {
                break;
            }
        }

        //Optimization 1
        for (auto cur_pstate : PStates)
        {
            bool end_loop = false;

            auto tmp = PStates;
            tmp.erase(cur_pstate);
            for (auto pstate1 : tmp)
            {
                if (relation[cur_pstate.first][pstate1.first] && isSmaller(pstate1.second, cur_pstate.second, relation))
                {
                    PStates.erase(cur_pstate);
                    end_loop = true;
                    break;
                }
            }
            if (end_loop)
            {
                break;
            }
        }
    }

    FiniteAutomata FiniteAutomata::Union(const FiniteAutomata &another_automaton)
    {
        StateType state_offset = this->StatesCount;

        //Create new automaton as a union of the two
        FiniteAutomata temp_automaton;
        //Insert union of states
        for (StateType i = 0; i < this->StatesCount; i++)
        {
            std::string name = "q" + std::to_string(i);
            if (temp_automaton.InsertState(name, i))
            {
                temp_automaton.StatesCount++;
            }
        }
        for (StateType i = 0; i< another_automaton.StatesCount; i++)
        {
            std::string name = "q" + std::to_string(i + state_offset);
            if (temp_automaton.InsertState(name, i + state_offset))
            {
                temp_automaton.StatesCount++;
            }
        }

        //Insert initial states
        for (auto init_state : this->StartStates)
        {
            auto state = temp_automaton.StatesDictionary.at(init_state);
            temp_automaton.StartStates.emplace(temp_automaton.States.at(state));
        }
        for (auto init_state : another_automaton.StartStates)
        {
            auto state = temp_automaton.StatesDictionary.at(init_state+state_offset);
            temp_automaton.StartStates.emplace(temp_automaton.States.at(state));
        }

        //Insert final states
        for (auto final_state : this->FinalStates)
        {
            auto state = temp_automaton.StatesDictionary.at(final_state);
            temp_automaton.FinalStates.emplace(temp_automaton.States.at(state));
        }
        for (auto final_state : another_automaton.FinalStates)
        {
            auto state = temp_automaton.StatesDictionary.at(final_state+state_offset);
            temp_automaton.FinalStates.emplace(temp_automaton.States.at(state));
        }

        //Insert alphabet of the first automaton, whoever is calling this function should ensure that both automata have the same alphabet
        for (auto symbol : this->Alphabet)
        {
            temp_automaton.Alphabet.emplace(symbol);
        }

        //Insert union of the transition rules
        temp_automaton.TFunction.TransitionFunctionInit(temp_automaton.StatesCount, &temp_automaton.Alphabet);
        for (StateType state1 = 0; state1 < this->StatesCount; state1++)
        {
            auto transition_from_state = this->TFunction.AllTransitions[state1];
            for (const auto &symbol : this->Alphabet)
            {
                if (transition_from_state.count(symbol) != 0)
                {
                    for (auto state2 : transition_from_state.at(symbol))
                    {
                        temp_automaton.TFunction.InsertTransition(state1, symbol, state2);
                    }
                }
            }
        }
        for (StateType state1 = 0; state1 < another_automaton.StatesCount; state1++)
        {
            auto transition_from_state = another_automaton.TFunction.AllTransitions[state1];
            for (const auto &symbol : another_automaton.Alphabet)
            {
                if (transition_from_state.count(symbol) != 0)
                {
                    for (auto state2 : transition_from_state.at(symbol))
                    {
                        temp_automaton.TFunction.InsertTransition(state1+state_offset, symbol, state2+state_offset);
                    }
                }
            }
        }

        return temp_automaton;
    }

    //is *this included in @p another_automaton
    bool FiniteAutomata::isIncluded(const FA::FiniteAutomata &another_automaton, FA::Relation_t relation)
    {
        // Check if the alphabets are the same
        if ( this->Alphabet != another_automaton.Alphabet )
        {
            return false;
        }

        auto union_automaton = this->Union(another_automaton);

        set<ProductState_t> Processed = {}, Next = {}, PN_union = {};

        //Create initial product states and minimize them
        auto B_startState = FiniteAutomata::Minimize(another_automaton.StartStates, relation);

        for (auto init_state : this->StartStates)
        {
            Next.emplace(make_pair(init_state, B_startState));
        }
        Initialize(Next, relation);

        while ( ! Next.empty() )
        {
            auto R = *Next.begin();
            for (auto symbol : union_automaton.Alphabet)
            {
                auto r1_states = FiniteAutomata::StatePost(R.first, union_automaton.TFunction, symbol);
                auto tmpR1_states = FiniteAutomata::Post(R.second, union_automaton.TFunction, symbol);
                auto R1_states = Minimize(tmpR1_states, relation);

                //LINE 6
                for (auto r1_state : r1_states)
                {
                    //Preparations
                    ProductState_t P = make_pair(r1_state, R1_states);

                    bool exists_smaller_state = false; //for LINE 8
                    for (auto p1 : P.second)
                    {
                        if (relation[P.first][p1])
                        {
                            exists_smaller_state = true;
                            break;
                        }
                    }

                    PN_union.clear();
                    PN_union.insert(Processed.begin(), Processed.end());
                    PN_union.insert(Next.begin(), Next.end());

                    bool exists_smaller_in_PN = false;
                    for (auto S : PN_union)
                    {
                        if (isProductSmaller(S, P, relation))
                        {
                            exists_smaller_in_PN = true;
                            break;
                        }
                    }

                    //LINE 7
                    if (isProductAccepting(P.first, P.second, this->FinalStates, another_automaton.FinalStates))
                    {
                        return false;
                    }
                    //LINE 8
                    else if ( ! exists_smaller_state)
                    {
                        if ( ! exists_smaller_in_PN )
                        {
                            for (auto S : PN_union)
                            {
                                if (relation[S.first][P.first] && isSmaller(P.second, S.second, relation))
                                {
                                    Processed.erase(S);
                                    Next.erase(S);
                                    Next.emplace(P);
                                }
                            }
                        }
                    }

                }
            }
        }

        return true;
    }

    std::set<StateType> FiniteAutomata::Minimize(const std::set<StateType> &MacroState, FA::Relation_t &Simulation)
    {
        auto R = set<StateType> (MacroState.begin(), MacroState.end());

        for (auto P_state : R)
        {
            auto tmp = set<StateType> (R.begin(), R.end());
            tmp.erase(P_state);

            for (auto S_state : tmp)
            {
                if (Simulation[S_state][P_state])
                {
                    R.erase(S_state);
                }
            }
        }
        return R;
    }

    std::set<StateType> FiniteAutomata::StatePost(FA::StateType State, FA::TransitionFunction &T_Function, std::string &symbol)
    {
        set<StateType> R = {};
        auto transition_from_state = T_Function.AllTransitions[State];

        if (transition_from_state.count(symbol) != 0)
        {
            auto p1_states = transition_from_state.at(symbol);
            R.insert(p1_states.begin(), p1_states.end());
        }

        return R;
    }

    std::set<StateType> FiniteAutomata::Post(std::set<StateType> &State, FA::TransitionFunction &T_Function, string &symbol)
    {
        set<StateType> R = {};
        for (auto P_state : State)
        {
            auto tmp = T_Function.AllTransitions[P_state];
            if (tmp.count(symbol) == 0)
            {
                continue;
            }
            else
            {
                auto cur_transition = tmp.at(symbol);
                R.insert(cur_transition.begin(), cur_transition.end());
            }

        }
        return R;
    }

}// namespace FA
