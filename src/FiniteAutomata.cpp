/**
 * @file FiniteAutomata.cpp
 * @author Samuel Stolarik (xstola03@stud.fit.vutbr.cz)
 *
 * @date 2022-12-02
 *
 */

#include "headers/FiniteAutomata.h"
#include "3rd_party/include/simlib/explicit_lts.hh"
#include <iostream>
#include <sstream>
#include <fstream> //getline doesn't work without this for some reason
#include <set>
#include <iomanip>
#include <cstdlib> //exit
#include <vector>
#include <chrono>

using namespace std;

//FiniteAutomata
namespace FA
{
    state_type_t FA::FiniteAutomata::StatesSize() const
    {
        return states.size();
    }

    symbol_type_t FA::FiniteAutomata::AlphabetSize() const
    {
        return alphabet.size();
    }

    bool FA::FiniteAutomata::InsertState(string& state)
    {
        return states.insert_state(state);
    }

    bool FA::FiniteAutomata::InsertStartState(string& state)
    {
        states.insert_state(state);

        state_type_t state_hash;
        if ( states.get_state_hash(state, state_hash) ){
            return states_start.insert_state_at_index(state, state_hash);
        }
        else{
            return false;
        }
    }

    bool FA::FiniteAutomata::RemoveStartState(string& state)
    {
        state_type_t state_hash;

        if ( states.get_state_hash(state, state_hash) ){
            return states_start.delete_state(state_hash);
        }
        else{
            return false;
        }
    }

    bool FA::FiniteAutomata::InsertFinalState(string& state)
    {
        states.insert_state(state);

        state_type_t state_hash;
        if ( states.get_state_hash(state, state_hash) ){
            return states_final.insert_state_at_index(state, state_hash);
        }
        else{
            return false;
        }
    }

    bool FA::FiniteAutomata::RemoveFinalState(string& state)
    {
        state_type_t state_hash;

        if ( states.get_state_hash(state, state_hash) ){
            return states_final.delete_state(state_hash);
        }
        else{
            return false;
        }
    }

    bool FA::FiniteAutomata::DeleteState(string& state)
    {
        state_type_t state_hash;
        // Delete state only if it exists in this finite automaton
        if ( states.get_state_hash(state, state_hash) )
        {
            states_start.delete_state(state_hash);
            states_final.delete_state(state_hash);
            trans_function.erase_state(state_hash);

            return states.delete_state(state_hash);
        }
        else{
            return false;
        }
    }

    bool FA::FiniteAutomata::InsertSymbol(string& symbol)
    {
        return alphabet.insert_symbol(symbol);
    }

    bool FA::FiniteAutomata::InsertTransition(string& state1, string& symbol, string& state2)
    {
        states.insert_state(state1);
        states.insert_state(state2);
        alphabet.insert_symbol(symbol);

        state_type_t hash_state1, hash_state2;
        symbol_type_t hash_symbol;

        if ( states.get_state_hash(state1, hash_state1) && states.get_state_hash(state2, hash_state2) && alphabet.get_symbol_hash(symbol, hash_symbol) ){
            return trans_function.insert_transition(hash_state1, hash_symbol, hash_state2);
        }
        else{
            return false;
        }
    }

    void FA::FiniteAutomata::Load(ifstream  &stream)
    {
        stringstream input = FA::Utils::prepare_file(stream);
        string line;
        string cur_token;
        bool type_read = false;

        while(getline(input, line)){
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
                while (FA::Utils::get_token(line, cur_token))
                {
                    InsertState(cur_token);
                }
                continue;
            }

            // Initial states of the FA, |"%Initial"| == 8
            else if (line.substr(0,8) == "%Initial")
            {
                // line_buff << line << endl;
                line = line.substr(8);
                while (FA::Utils::get_token(line, cur_token))
                {
                    InsertStartState(cur_token);
                }

                continue;
            }

            //Accept states of the FA, |"%Final"| == 6
            else if (line.substr(0,6) == "%Final")
            {
                // line_buff << line << endl;
                line = line.substr(6);
                while (FA::Utils::get_token(line, cur_token))
                {
                    InsertFinalState(cur_token);
                }
                continue;
            }

            //Alphabet of the FA, |"%Alphabet"| == 9
            else if (line.substr(0,9) == "%Alphabet")
            {
                line = line.substr(10);
                while (FA::Utils::get_token(line, cur_token))
                {
                    // Alphabet.insert(cur_token);
                    InsertSymbol(cur_token);
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
                string state1, state2, symbol;
                FA::Utils::get_token(line, state1);
                FA::Utils::get_token(line, symbol);
                FA::Utils::get_token(line,state2);

                InsertTransition(state1, symbol, state2);
            }
        }
    } //FA::FiniteAutomata::Load

    void FA::FiniteAutomata::Print(ostream& stream)
    {
        //Print name of the FA
        if (!Name.empty()){
            stream << "%Name " << Name << endl;
        }

        states.print(stream);
        alphabet.print(stream);
        states_start.print(stream, "Initial");
        states_final.print(stream, "Final");

        stream << endl;

        print_transition_function(stream, this->trans_function);
    }

    state_type_t FA::FiniteAutomata::Minimize()
    {
        state_type_t delete_count = 0;
        delete_count += minimize_delete_unreachable();
        delete_count += minimize_delete_nonterm();
        return delete_count;
    }

    BinaryRelation * FA::FiniteAutomata::IdentityRelation() const
    {
        auto relation = new BinaryRelation(states.get_max_hash()+1, false);

        for ( state_type_t i = 0; i < relation->size(); i++ ){
            relation->set(i, i, true);
        }

        return relation;
    }

    BinaryRelation * FA::FiniteAutomata::SimulationRelation() const
    {
        // Compute reverse transition function
        auto reverse_trans_function = trans_function.revert();

        auto size = states.get_max_hash()+1;

        auto sim_relation_compl =  new FA::BinaryRelation(size, false);
        set<pair<state_type_t, state_type_t>> worklist {};

        vector<vector<vector<state_type_t>>> counters (alphabet.size(), vector<vector<state_type_t>>(size, vector<state_type_t> (size, 0)));

        // Initial refinement
        for ( const auto symbol : alphabet.get_symbols() ){
            for ( const auto state1 : states.get_states() )
            {
                for ( const auto state2 : states.get_states() )
                {
                    // LINE 6 in the algorithm
                    counters[symbol][state1][state2] = trans_function.get_states(state2, symbol).size();

                    // ((p e F) and (q !e F)) OR (|(D(p,a)| != 0 and |D(q,a)| == 0) where e = belongs to
                    if ( (states_final.belongs_to(state1) && ! states_final.belongs_to(state2) ) || ( ( ! trans_function.get_states(state1, symbol).empty() ) && (trans_function.get_states(state2, symbol).empty()) ))
                    {
                        sim_relation_compl->set(state1, state2, true);
                        worklist.insert(make_pair(state1, state2)); // Insert (state1, state2) into worklist
                    }
                }
            }
        } // End of initial refinement

        // Propagate until fixpoint
        state_type_t p1, q1;
        while ( ! worklist.empty() ){

            auto cur_pair = worklist.begin();
            p1 = cur_pair->first;
            q1 = cur_pair->second;
            worklist.erase(cur_pair);

            for ( const auto symbol : alphabet.get_symbols() ){
                for ( const auto q : reverse_trans_function->get_states(q1, symbol) ){
                    counters[symbol][p1][q] -= 1;

                    if ( counters[symbol][p1][q] == 0 ){ // q can't go over an above p1
                        for ( const auto p : reverse_trans_function->get_states(p1, symbol) ){
                            if ( ! sim_relation_compl->get(p,q) ){
                                sim_relation_compl->set(p, q, true);

                                worklist.insert(make_pair(p,q));
                            }
                        }
                    }
                }
            }

        }

        delete reverse_trans_function;

        sim_relation_compl->complement();

        return sim_relation_compl;
    }

    BinaryRelation * FA::FiniteAutomata::SimulationRelation_simlib(std::chrono::microseconds &conversion_time) const
    {
        Simlib::ExplicitLTS LTSforSimulation;

        //Time required to convert automata to simlib format
        auto conv_start = chrono::high_resolution_clock::now();

        // Insert copy *this automaton to LTSforSimulation

        for ( const auto state1 : states.get_states() ){
            for ( const auto symbol : alphabet.get_symbols() ){
                auto dst_states = trans_function.get_states(state1, symbol);

                for ( const auto state2 : dst_states ){
                    // Insert transition
                    LTSforSimulation.add_transition(state1, symbol, state2);
                }
            }
        }

        // final states cannot be simulated by nonfinal -> we add new selfloops over final states with new symbol in LTS
        for ( auto final_state : states_final.get_states() ){
            LTSforSimulation.add_transition(final_state, alphabet.size() + 1, final_state);
        }

        LTSforSimulation.init();

        auto conv_stop = chrono::high_resolution_clock::now();
        auto conv_duration = chrono::duration_cast<chrono::microseconds> (conv_stop - conv_start);


        // Compute simulation and measure the required time
        auto sim_start = chrono::high_resolution_clock::now();

        auto simulation_relation = LTSforSimulation.compute_simulation();

        auto sim_stop = chrono::high_resolution_clock::now();
        auto sim_duration = chrono::duration_cast<chrono::microseconds> (sim_stop - sim_start);
        std::cout << "simulation-time:" << sim_duration.count() << std::endl;

        // Time of converting back from from simlib format
        auto conv2_start = chrono::high_resolution_clock::now();

        // Convert Simlib::BinaryRelation for FA::BinaryRelation

        auto relation_size = simulation_relation.size();

        BinaryRelation * relation = new BinaryRelation(relation_size, false);
        for ( size_t i = 0; i < relation->size(); i++){
            for ( size_t j = 0; j < relation->size(); j++ ){
                bool value = simulation_relation.get(i,j);
                relation->set(i,j, value);
            }
        }

        auto conv2_stop = chrono::high_resolution_clock::now();
        auto conv2_duration = chrono::duration_cast<chrono::microseconds> (conv2_stop - conv2_start);

        conversion_time = (conv_duration + conv2_duration);

        return relation;
    }

    bool FA::FiniteAutomata::isUniversal(const BinaryRelation& relation) const
    {
        size_t visited_states = 0;
        auto start_states = states_start.get_states();
        if ( ! is_macro_state_accepting(start_states) ){
            std::cout << "states-visited:" << visited_states << std::endl;
            return false;
        }

        visited_states++;

        set<macro_state_t> processed {};
        set<macro_state_t> next {};
        set<macro_state_t> pn_union {};

        minimize_macro_state(start_states, relation);
        next.emplace(start_states);

        while ( ! next.empty() ){
            // Pick and remove macro state R from next and move it to processed
            auto ms_R = *next.begin();
            processed.emplace(ms_R);
            next.erase(ms_R);

            auto post_ms_R = post_macro_state(ms_R);
            visited_states += post_ms_R.size();

            for ( auto ms_P : post_ms_R ){

                minimize_macro_state(ms_P, relation);

                if ( ! is_macro_state_accepting(ms_P) ){
                    std::cout << "states-visited:" << visited_states << std::endl;
                    return false;
                }

                pn_union.clear();
                pn_union.insert(processed.begin(), processed.end());
                pn_union.insert(next.begin(), next.end());

                // check if exists macro state S in union of next and processed, such that S < P
                bool exists_smaller = false;

                for ( auto ms_S : pn_union ){
                    if ( is_macro_state_smaller(ms_S, ms_P, relation) )
                    {
                        exists_smaller = true;
                        break;
                    }
                }

                // If such macro state S does not exist, remove all S from union of processed and next, such that P < S and then add P to next
                if ( ! exists_smaller ){
                    for ( auto ms_S : pn_union ){
                        if ( is_macro_state_smaller(ms_P, ms_S, relation) ){
                            processed.erase(ms_S);
                            next.erase(ms_S);
                        }
                    }

                    next.emplace(ms_P);
                }
            }
        }

        std::cout << "states-visited:" << visited_states << std::endl;
        return true;
    }

    bool FA::FiniteAutomata::isIncluded(FiniteAutomata &another_automaton, FiniteAutomata &union_automaton ,BinaryRelation relation) const
    {
        set<product_state_t> ps_initial {};
        auto initial_a = this->states_start.get_states();
        auto initial_b = another_automaton.states_start.get_states();

        minimize_macro_state(initial_b, relation);

        for ( const auto state : initial_a ){
            ps_initial.emplace(make_pair(state, initial_b));
        }

        set<product_state_t> processed {};
        set<product_state_t> next = inclusion_initialize(ps_initial, relation);
        set<product_state_t> pn_union {};

        while ( ! next.empty() ){
            // Pick and remove a product-state (r, R) from next and move it to processed
            auto ps_rR = *next.begin();
            processed.emplace(ps_rR);
            next.erase(ps_rR);

            auto post_ps_rR = union_automaton.post_product_state(ps_rR);

            for ( auto ps_pP : post_ps_rR ){
                minimize_macro_state(ps_pP.second, relation);

                if ( is_product_state_accepting(ps_pP, this->states_final, another_automaton.states_final) ){
                    return false;
                }

                pn_union.clear();
                pn_union.insert(processed.begin(), processed.end());
                pn_union.insert(next.begin(), next.end());


                // If exists p1 in P such that p < p1
                bool exists_smaller_pP = false;

                for ( const auto p1 : ps_pP.second ){
                    if ( relation.get(ps_pP.first, p1) ){
                        exists_smaller_pP = true;
                        break;
                    }
                }

                if ( ! exists_smaller_pP ){
                    // if exist product-state (s,S) in processed U next s.t. s < p && P < S
                    bool exists_smaller_sS = false;

                    for ( const auto &ps_sS : pn_union ){
                        if ( relation.get(ps_pP.first, ps_sS.first)  &&  (is_macro_state_smaller(ps_sS.second, ps_pP.second, relation)) ){
                            exists_smaller_sS = true;
                            break;
                        }
                    }

                    if ( ! exists_smaller_sS ){
                        // Remove all (s, S) from processed U next s.t. s<p && P<S
                        for ( const auto &ps_sS : pn_union ){
                            if ( relation.get(ps_sS.first, ps_pP.first)  &&  is_macro_state_smaller(ps_pP.second, ps_sS.second, relation) ){
                                processed.erase(ps_sS);
                                next.erase(ps_sS);
                            }
                        }
                        next.emplace(ps_pP);
                    }
                }
            }
        }

        return true;
    }

    FiniteAutomata FA::FiniteAutomata::Union(const FiniteAutomata &another_automaton) const
    {
        FiniteAutomata union_automaton (*this);

        auto another_states = another_automaton.states.get_states();
        auto all_symbols = union_automaton.alphabet.get_symbols();

        for ( auto state1 : another_states ){
            auto state1_name = another_automaton.states.get_state_name(state1);
            state_type_t state_index;
            another_automaton.states.get_state_hash(state1_name, state_index);
            union_automaton.states.insert_state_at_index(state1_name, state_index);

            for ( auto symbol : all_symbols ){
                auto symbol_name = union_automaton.alphabet.get_symbol(symbol);

                auto dst_states = another_automaton.trans_function.get_states(state1, symbol);
                for ( auto state2 : dst_states ){
                    auto state2_name = another_automaton.states.get_state_name(state2);

                    union_automaton.InsertTransition(state1_name, symbol_name, state2_name);
                }
            }

            if ( another_automaton.states_start.belongs_to(state1) ){
                union_automaton.InsertStartState(state1_name);
            }

            if ( another_automaton.states_final.belongs_to(state1) ){
                union_automaton.InsertFinalState(state1_name);
            }
        }

        return union_automaton;
    }

    set<product_state_t> FA::FiniteAutomata::inclusion_initialize(set<product_state_t> p_state, BinaryRelation relation)
    {
        set<product_state_t> p_state_copy (p_state);
        set<product_state_t> p_state_copy2 {};

        // for each product state (p, P)
        for ( const auto &ps_pP : p_state ){
            bool deleted = false;

            // Test whether for all p1 in P : p !< p1
            for ( const auto p1 : ps_pP.second ){
                if ( ps_pP.first == p1 ){
                    continue;
                }

                if ( relation.get(ps_pP.first, p1) ){
                    p_state_copy.erase(ps_pP);
                    deleted = true;
                }
            }

            if ( deleted ){
                continue;
            }

            // for each product state (q, Q)
            p_state_copy2 = p_state_copy;

            for ( const auto &ps_qQ : p_state_copy2 ){
                if ( ps_pP == ps_qQ ){
                    continue;
                }

                // p < q OR Q < P
                if ( relation.get(ps_pP.first, ps_qQ.first) && is_macro_state_smaller(ps_qQ.second, ps_pP.second, relation) ){
                    p_state_copy.erase(ps_qQ);
                }
            }
        }

        return p_state_copy;
    }

    void FA::FiniteAutomata::print_transition_function(ostream& stream, TransitionFunction& trans_function)
    {
        auto all_states = states.get_states();
        auto all_symbols = alphabet.get_symbols();

        for ( const auto state1 : all_states ){
            for ( const auto symbol : all_symbols ){
                auto dst_states = trans_function.get_states(state1, symbol);

                for ( const auto state2 : dst_states ){
                    stream << states.get_state_name(state1) << " " << alphabet.get_symbol(symbol) << " " << states.get_state_name(state2) << endl;
                }
            }
        }

    }

    state_type_t FA::FiniteAutomata::minimize_delete_nonterm()
    {
        auto reverse_t_func = trans_function.revert();

    // Start states are always reachable
        set<state_type_t> states_terminal(states_final.get_states());
        set<state_type_t> prev_terminal{};
        set<state_type_t> last_added(states_terminal);

        while ( states_terminal != prev_terminal ){

            prev_terminal = states_terminal;

            auto tmp_set = last_added;
            last_added.clear();

            for ( const auto state : tmp_set ){
                for ( const auto symbol : alphabet.get_symbols() ){
                    auto cur_states = reverse_t_func->get_states(state, symbol);

                    last_added.insert(cur_states.begin(), cur_states.end());
                }
            }

            states_terminal.insert(last_added.begin(), last_added.end());
        }

        state_type_t delete_count = 0;

        // Delete unreachable states from finite automaton
        auto tmp_states = states.get_states();
        for ( auto state : tmp_states ){
            if ( states_terminal.count(state) == 0 ){
                auto name = states.get_state_name(state);
                if (DeleteState(name)) {delete_count++;}
            }
        }
        delete reverse_t_func;

        return delete_count;
    }

    state_type_t FA::FiniteAutomata::minimize_delete_unreachable()
    {

        // Start states are always reachable
        set<state_type_t> states_reachable(states_start.get_states());
        set<state_type_t> prev_reachable{};
        set<state_type_t> last_added(states_reachable);

        while ( states_reachable != prev_reachable ){

            prev_reachable = states_reachable;

            auto tmp_set = last_added;
            last_added.clear();

            for ( const auto state : tmp_set ){
                for ( const auto symbol : alphabet.get_symbols() ){
                    auto cur_states = trans_function.get_states(state, symbol);

                    last_added.insert(cur_states.begin(), cur_states.end());
                }
            }

            states_reachable.insert(last_added.begin(), last_added.end());
        }

        state_type_t delete_count = 0;

        // Delete unreachable states from finite automaton
        auto tmp_states = states.get_states();
        for ( auto state : tmp_states ){
            if ( states_reachable.count(state) == 0 ){
                auto name = states.get_state_name(state);
                if (DeleteState(name)) {delete_count++;}
            }
        }

        return delete_count;
    }

    void FA::FiniteAutomata::minimize_macro_state(set<state_type_t> &macro_state, BinaryRelation relation)
    {
        set<state_type_t> tmp_states (macro_state);

        for ( const auto state1 : tmp_states ){
            if ( macro_state.count(state1) == 0){
                continue;
            }
            for ( const auto state2 : tmp_states ){

                if ( macro_state.count(state2) == 0 ){
                    continue;
                }
                // if state1 and state2 are not the same states and if state1 is simulated by state2, delete state1
                if ( state1 != state2  && relation.get(state1, state2) ){
                    macro_state.erase(state1);
                }
            }
        }
    }

    bool FA::FiniteAutomata::is_macro_state_accepting(set<state_type_t> &macro_state) const
    {
        for ( const auto state : macro_state ){
            if ( states_final.belongs_to(state) ){
                return true;
            }
        }

        return false;
    }

    bool FA::FiniteAutomata::is_product_state_accepting(product_state_t p_state, const States &final_a, const States &final_b)
    {
        if (final_a.belongs_to(p_state.first) ){
            for ( const auto state : p_state.second ){
                if ( final_b.belongs_to(state) ){
                    // if macro state p_state.second is accepting in automaton B, product_state is rejecting
                    return false;
                }
            }

            // No accepting state found in macro state p_state.second
            return true;
        }

        // if state p_state.first is rejecting in automaton A, product state is rejecting
        else{
            return false;
        }
    }

    set<product_state_t> FA::FiniteAutomata::post_product_state(product_state_t p_state) const
    {
        set<product_state_t> ps_post {};
        auto all_symbols = alphabet.get_symbols();

        for ( const auto symbol : all_symbols ){
            auto s_post = post_state(p_state.first, symbol);

            // Post(ms, symbol)
            set<state_type_t> post_macro {};
            for ( const auto state : p_state.second ){
                auto ms = post_state(state, symbol);
                post_macro.insert(ms.begin(), ms.end());
            }

            for ( const auto state : s_post ){
                ps_post.emplace(make_pair(state, post_macro));
            }
        }

        return ps_post;
    }

    set<macro_state_t> FA::FiniteAutomata::post_macro_state(set<state_type_t> macro_state) const
    {
        set<macro_state_t> post_ms {};
        auto symbols = alphabet.get_symbols();

        for ( const auto symbol : symbols ){
            set<state_type_t> tmp {};

            for ( const auto state : macro_state ){
                auto ms = post_state(state, symbol);
                tmp.insert(ms.begin(), ms.end());
            }

            post_ms.emplace(tmp);
        }

        return post_ms;
    }

    set<state_type_t> FA::FiniteAutomata::post_state(state_type_t state, symbol_type_t symbol) const
    {
        return trans_function.get_states(state, symbol);
    }

    bool FA::FiniteAutomata::is_state_smaller(state_type_t state1, state_type_t state2, BinaryRelation relation)
    {
        auto size = relation.size();

        if ( state1 >= size || state2 >= size ){
            return false;
        }

        return relation.get(state1, state2);
    }

    bool FA::FiniteAutomata::is_macro_state_smaller(set<state_type_t> macro_state1, set<state_type_t> macro_state2, BinaryRelation relation)
    {
        for ( auto const state1 : macro_state1 ){
            bool cur_simulated = false;

            for ( auto const state2 : macro_state2 ){
                if ( is_state_smaller(state1, state2, relation) ){
                    cur_simulated = true;
                    break;
                }
            }

            if ( ! cur_simulated ){
                return false;
            }
        }

        return true;
    }

    void FA::FiniteAutomata::rename_state(string name_orig, string name_new)
    {

        if ( name_orig == name_new ){
            return;
        }

        state_type_t state_hash;

        // If state with name @p name_new already exists do nothing
        if ( states.get_state_hash(name_new, state_hash) ){
            return;
        }

        // In case the state doesn't exist in states
        if ( ! states.get_state_hash(name_orig, state_hash) ){
            return;
        }

        states.set_state_name(state_hash, name_new);
        states_final.set_state_name(state_hash, name_new);
        states_start.set_state_name(state_hash, name_new);
    }

    void FA::FiniteAutomata::MakeDifferent(FiniteAutomata &another_automaton)
    {
        state_type_t state_offset = this->states.get_max_hash() + 1;

        FiniteAutomata aux_automaton {};
        aux_automaton.alphabet = another_automaton.alphabet;

        // Set starting index in the new automaton, that way all states will be mapped to index atleast state_offset
        aux_automaton.states.set_state_index_offset(state_offset);

        // Insert all states and transition with respect to given index offset
        auto another_states = another_automaton.states.get_states();
        auto all_symbols = another_automaton.alphabet.get_symbols();

        for ( const auto state : another_states ){
            auto state_name = another_automaton.states.get_state_name(state);
            aux_automaton.InsertState(state_name);

            for ( const auto symbol : all_symbols ){
                auto symbol_name = another_automaton.alphabet.get_symbol(symbol);
                auto dst_states = another_automaton.trans_function.get_states(state, symbol);

                for ( const auto state2 : dst_states ){
                    auto state2_name = another_automaton.states.get_state_name(state2);
                    aux_automaton.InsertTransition(state_name, symbol_name, state2_name);
                }
            }


            // if state is also initial or final insert it
            if ( another_automaton.states_start.belongs_to(state) ){
                aux_automaton.InsertStartState(state_name);
            }

            if ( another_automaton.states_final.belongs_to(state) ){
                aux_automaton.InsertFinalState(state_name);
            }
        }

        // Make sure no states in this and another automaton have the same name
        auto aux_automaton_states = aux_automaton.states.get_states();

        for ( auto state : aux_automaton_states ){
            auto state_name = aux_automaton.states.get_state_name(state);
            auto new_state_name = state_name;
            state_type_t tmp;

            while ( this->states.get_state_hash(new_state_name, tmp) ){
                new_state_name = new_state_name + "\'";
            }

            aux_automaton.rename_state(state_name, new_state_name);
        }

        another_automaton = aux_automaton;
    }

}// namespace FA

//States
namespace FA
{
    FA::States::States()
    {
        m_size = 0;
        m_next_state_index = 0;
    }

    void FA::States::print(std::ostream& stream, const char* typeof_state) const
    {
        auto it = states_hash.begin();
        stream << "%" << typeof_state;

        while ( it != states_hash.end() ){
            stream << " " << (it++)->first;
        }

        stream << endl;
    }

    state_type_t FA::States::size() const
    {
        return m_size;
    }

    set<state_type_t> FA::States::get_states() const
    {
        return m_states;
    }

    string FA::States::get_state_name(state_type_t state) const
    {
        if ( m_states.count(state) == 0 ){
            return {};
        }

        else{
            return states_dictionary.at(state);
        }
    }

    bool FA::States::get_state_hash(string& state, state_type_t& dst) const
    {
        if ( states_hash.count(state) == 0 ){
            return false;
        }

        dst = states_hash.at(state);
        return true;
    }

    state_type_t FA::States::get_max_hash() const
    {
        state_type_t max = *(m_states.begin());

        for ( const auto state : m_states ){
            if ( state > max ){
                max = state;
            }
        }

        return max;
    }

    bool FA::States::insert_state(string& state)
    {
        bool inserted = states_hash.emplace(state, m_next_state_index).second;

        if ( inserted ){
            states_dictionary.emplace(m_next_state_index, state);
            m_states.emplace(m_next_state_index);
            m_size++;
            m_next_state_index++;
        }

        return inserted;
    }

    bool FA::States::insert_state_at_index(string& state, state_type_t index)
    {
        bool inserted = states_hash.emplace(state, index).second;

        if ( inserted ){
            states_dictionary.emplace(index, state);
            m_states.emplace(index);
            m_size++;
            // Choose max from @p index and m_next_state_index end increment
            m_next_state_index = (((index) > (m_next_state_index)) ? index : m_next_state_index) + 1;
        }

        return inserted;
    }

    bool FA::States::delete_state(state_type_t state)
    {
        if ( m_states.count(state) == 0 ){
            return false;
        }

        auto state_name = states_dictionary.at(state);

        m_states.erase(state);
        states_dictionary.erase(state);
        states_hash.erase(state_name);

        m_size -= 1;
        return true;
    }

    bool FA::States::belongs_to(state_type_t state) const
    {
        if ( m_states.count(state) == 0 ){
            return false;
        }
        else{
            return true;
        }
    }

    void FA::States::set_state_name(state_type_t state, string name_new)
    {
        // Check if @p state exists
        if ( states_dictionary.count(state) == 0){
            return;
        }

        auto name_orig = states_dictionary.at(state);

        // Change string mapped to state_type_t @p state
        states_dictionary[state] = name_new;

        // Erase key "name_orig" and insert "name_new" with the same value as "name_orig" had
        states_hash.erase(name_orig);
        states_hash.emplace(name_new, state);
    }

    void FA::States::set_state_index_offset(state_type_t offset)
    {
        m_next_state_index = offset;
    }
}

//Alphabet
namespace FA
{
    FA::Alphabet::Alphabet()
    {
        m_size = 0;
    }

    symbol_type_t FA::Alphabet::size() const
    {
        return m_size;
    }

    set<symbol_type_t> FA::Alphabet::get_symbols()
    {
        return m_symbols;
    }

    void FA::Alphabet::print(ostream& stream) const
    {
        auto it = symbol_hash.begin();

        stream << "%Alphabet";

        while ( it != symbol_hash.end() ){
            stream << " " << (it++)->first;
        }

        stream << endl;
    }

    bool FA::Alphabet::insert_symbol(string& symbol)
    {
        bool inserted = symbol_hash.emplace(symbol, m_size).second;

        if ( inserted ){
            symbol_dictionary.emplace(m_size, symbol);
            m_symbols.emplace(m_size);
            m_size++;
        }

        return inserted;
    }

    set<symbol_type_t> FA::Alphabet::get_symbols() const
    {
        return m_symbols;
    }

    string FA::Alphabet::get_symbol(symbol_type_t symbol) const
    {
        if ( symbol >= m_size ){
            return {};
        }
        else{
            return symbol_dictionary.at(symbol);
        }
    }

    bool FA::Alphabet::get_symbol_hash(string& symbol, symbol_type_t& dst) const
    {
        if ( symbol_hash.count(symbol) == 0 ){
            return false;
        }

        dst = symbol_hash.at(symbol);
        return true;
    }
}

//TransitionFunction
namespace FA
{
    FA::TransitionFunction::TransitionFunction(state_type_t state_count, symbol_type_t symbol_count) : m_state_count(state_count), m_symbol_count(symbol_count)
    {
        if ( m_state_count == 0 ){
            m_symbol_count = 0;
        }

        transition_function = vector<vector<set<state_type_t>>> (m_state_count, vector<set<state_type_t>> (m_symbol_count));
    }


    set<state_type_t> FA::TransitionFunction::get_states(state_type_t p, symbol_type_t a) const
    {
        if ( p >= m_state_count || a >= m_symbol_count )
        {
            return {};
        }
        else{
            return transition_function[p][a];
        }
    }

    bool FA::TransitionFunction::insert_transition(state_type_t p, symbol_type_t a, state_type_t q)
    {
        state_type_t new_state_max = p>q ? p : q;

        if ( new_state_max >= m_state_count ){
            m_state_count = (new_state_max+1) * 2;
            transition_function.resize(m_state_count, vector<set<state_type_t>>(m_symbol_count));
        }

        if ( a >= m_symbol_count ){
            m_symbol_count = a + 1;

            for ( state_type_t i = 0; i < m_state_count; i++ ){
                transition_function[i].resize(m_symbol_count);
            }
        }

        return transition_function[p][a].emplace(q).second;
    }

    bool FA::TransitionFunction::delete_transition(state_type_t p, symbol_type_t a, state_type_t q)
    {
        if ( p >= m_state_count || a >= m_symbol_count ){
            return false;
        }

        return transition_function[p][a].erase(q);
    }

    size_t FA::TransitionFunction::erase_state(state_type_t state)
    {
        if ( state >= m_state_count ){
            return 0;
        }

        size_t erase_counter = 0;

        // Delete outgoing transitions
        for ( symbol_type_t i = 0; i < m_symbol_count; i++ ){
            auto victim_set = transition_function[state][i];

            erase_counter += victim_set.size();
            victim_set.clear();
        }

        // Delete transitions to state
        for ( state_type_t i = 0; i < m_state_count; i++ ){
            for ( symbol_type_t j = 0; j < m_symbol_count; j++ ){
                erase_counter += transition_function[i][j].erase(state);
            }
        }

        return erase_counter;
    }

    FA::TransitionFunction* FA::TransitionFunction::revert() const
    {
        auto reverted = new FA::TransitionFunction(this->m_state_count, this->m_symbol_count);

        for ( state_type_t state1 = 0; state1 < m_state_count; state1++ ){
            for ( symbol_type_t symbol = 0; symbol < m_symbol_count; symbol++ ){
                auto dst_states = get_states(state1, symbol);

                for ( const auto state2 : dst_states ){
                    reverted->insert_transition(state2, symbol, state1);
                }
            }
        }

        return reverted;
    }
}

//Binary Relation
namespace FA
{
    FA::BinaryRelation::BinaryRelation(state_type_t size, bool value) : m_size(size)
    {
        m_relation = vector<vector<bool>> (m_size, vector<bool>(m_size, value));
    }

    state_type_t FA::BinaryRelation::size() const
    {
        return m_size;
    }

    void FA::BinaryRelation::set(state_type_t p, state_type_t q, bool value)
    {
        if ( p >= m_size || q >= m_size )
        {
            return;
        }

        m_relation[p][q] = value;
    }

    void FA::BinaryRelation::set_all(bool value)
    {
        for ( state_type_t i = 0; i < m_size*m_size; i++){
            m_relation[i/m_size][i%m_size] = value;
        }
    }

    bool FA::BinaryRelation::get(state_type_t p, state_type_t q) const
    {
        if ( p >= m_size || q >= m_size ){
            return false;
        }

        return m_relation[p][q];
    }

    void FA::BinaryRelation::complement()
    {
        for ( state_type_t i = 0; i < m_size*m_size; i++){
            m_relation[i/m_size][i%m_size] = ! m_relation[i/m_size][i%m_size];
        }
    }
}

//Utils
namespace FA
{
    bool FA::Utils::get_token(string& line, string& token)
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

    stringstream FA::Utils::prepare_file(ifstream &src_file)
    {
        string line;
        stringstream cleaned_file;
        while(getline(src_file, line))
        {
            FA::Utils::remove_whitespaces(line, " \t");
            FA::Utils::remove_comments(line);
            cleaned_file << line << endl;
        }

        return cleaned_file;
    }

    string FA::Utils::remove_comments(string &line)
    {
        auto hash_pos = line.find('#');
        if (hash_pos == string::npos)
        {
            return line;
        } //No comment found

        return line.erase(hash_pos);
    }

    string FA::Utils::remove_whitespaces(string &line, const string &whitespace)
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

}
