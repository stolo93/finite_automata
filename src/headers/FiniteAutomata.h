/**
 * @file FiniteAutomata.h
 * @author Samuel Stolarik (xstola03@stud.fit.vutbr.cz)
 *
 * @date 2022-12-02
 *
 */

#pragma once

#include <unordered_map>
#include <set>
#include <string>
#include <chrono>
#include <vector>

namespace FA {

    using state_type_t = unsigned;
    using symbol_type_t = unsigned;
    using macro_state_t = std::set<state_type_t>;
    using product_state_t = std::pair<state_type_t, macro_state_t>;

    class States {
    public:
        /**
         * @brief Construct a new States object
         *
         */
        States();

        /**
         * @brief Print all states to output stream @p steam
         *
         * @param stream
         */
        void print(std::ostream& stream, const char* typeof_state = "States") const;

        /**
         * @brief Get number of states
         *
         * @return state_type_t number of states
         */
        state_type_t size() const;

        /**
         * @brief Get the states object
         *
         * @return Set of states
         */
        std::set<state_type_t> get_states() const;

        /**
         * @brief Get the real name of state hashed to @p state
         *
         * @param state
         * @return std::string
         */
        std::string get_state_name(state_type_t state) const;

        void set_state_name(state_type_t state, std::string name_new);

        /**
         * @brief Get the hash of state @p state
         *
         * @param state name of state
         * @param dst return through refference
         * @return true if @p state is in states, otherwise @p dst remains unchanged
         */
        bool get_state_hash(std::string& state, state_type_t& dst) const;

        /**
         * @brief Return maximum of all hashes of currently existing states
         *
         * @return state_type_t
         */
        state_type_t get_max_hash() const;

        /**
         * @brief Insert state to the set of states
         *
         * @param state
         * @return true upon successfull insertion
         */
        bool insert_state(std::string &state);

        bool insert_state_at_index(std::string& state, state_type_t index);

        /**
         * @brief Delete
         *
         * @param state
         * @return true
         * @return false
         */
        bool delete_state(state_type_t state);

        /**
         * @brief Check if @p state belongs to states
         *
         * @param state
         * @return true if @p state is in m_states, false otherwise
         */
        bool belongs_to(state_type_t state) const;

        void set_state_index_offset(state_type_t offset);

    protected:
        state_type_t m_size;
        std::set<state_type_t> m_states;

        std::unordered_map<std::string, state_type_t> states_hash;
        std::unordered_map<state_type_t, std::string> states_dictionary;

    private:
        state_type_t m_next_state_index;
    }; // Class states

    class Alphabet {
    public:

        /**
         * @brief Construct a new Alphabet object
         *
         */
        Alphabet();

        /**
         * @brief Get the symbols object
         *
         * @return std::set<symbol_type_t>
         */
        std::set<symbol_type_t> get_symbols();

        /**
         * @brief Print all symbol to output stream @p steam
         *
         * @param stream
         */
        void print(std::ostream &stream) const;

        /**
         * @brief Get the size of the alphabet
         *
         * @return symbol_type_t number of symbols
         */
        symbol_type_t size() const;

        /**
         * @brief Insert @p symbol into the alphabet
         *
         * @param symbol
         * @return true upon successfull insertion
         */
        bool insert_symbol(std::string& symbol);

        /**
         * @brief Get the symbols object
         *
         * @return Set of all symbols in alphabet
         */
        std::set<symbol_type_t> get_symbols() const;

        /**
         * @brief Get the symbol object
         *
         * @param symbol
         * @return std::string
         */
        std::string get_symbol(symbol_type_t symbol) const;

        /**
         * @brief Return the hash of @p symbol through @p dst
         *
         * @param symbol
         * @param dst
         * @return true if @p symbol is in alphabet, otherwise @p dst remains unchanged
         */
        bool get_symbol_hash(std::string& symbol, symbol_type_t& dst) const;

    protected:
        symbol_type_t m_size;
        std::set<symbol_type_t> m_symbols;
        std::unordered_map<std::string, symbol_type_t> symbol_hash;
        std::unordered_map<symbol_type_t, std::string> symbol_dictionary;
    }; // Class alphabet

    class TransitionFunction {
    public:
        /**
         * @brief Construct a new Transition Function object
         *
         * @param state_count
         * @param symbol_count
         */
        TransitionFunction(state_type_t state_count = 0, symbol_type_t symbol_count = 0);

        /**
         * @brief Get set of all states which are on the right side of rules starting with state @p p and symbol @p a
         * pa -> {}
         *
         * @param p
         * @param a
         * @return std::set<state_type_t> pa -> {}
         */
        std::set<state_type_t> get_states(state_type_t p, symbol_type_t a) const;

        std::set<state_type_t> get_states_from_state(state_type_t state) const;

        /**
         * Insert transition from @p p to @p q on @p a
         *
         * @param p state from
         * @param a symbol
         * @param q state to
         *
         * @return True if transition was successfully added, false otherwise
         */
        bool insert_transition(state_type_t p, symbol_type_t a , state_type_t q);

        /**
         * @brief Delete transition from @p p to @p q on @p a
         *
         * @param p state from
         * @param a symbol
         * @param q state to
         *
         * @return True if transition was successfully added, false otherwise
         */
        bool delete_transition(state_type_t p, symbol_type_t a, state_type_t q);

        /**
         * @brief Deletes all transitions from @p state and also all transitions to @p state
         *
         * @param state
         * @return Number of deleted transiotions
         */
        size_t erase_state(state_type_t state);

        /**
         * @brief Create and return reversion of this transition function
         *
         * @return reverted transition function
         */
        TransitionFunction * revert() const;

    protected:
        std::vector<std::vector<std::set<state_type_t>>> transition_function;
        state_type_t m_state_count;
        symbol_type_t m_symbol_count;
    }; //class TransitionFunction

    class Utils {
    public:

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

        /**
         * @brief Get next token in @p line ,tokens are separated with a space
         *
         * @param line
         * @param whitespace
         * @return Next token in the line or std::string::npos if there are no tokens left in the @p line
         */
        static bool get_token(std::string &line, std::string &token);
    };

    class BinaryRelation {
    public:
        /**
         * @brief Construct a new Binary Relation object with size @p size and all cells are initialized to @p value
         *
         * @param size
         * @param value
         */
        BinaryRelation(state_type_t size, bool value);

        /**
         * @brief Set value of cell at indices @p p and @p q.
         * If one of the indeces is bigger than size of the relation, nothing is done.
         *
         * @param p row index
         * @param q column index
         * @param value
         */
        void set(state_type_t p, state_type_t q, bool value);

        /**
         * @brief Set all cells in relation to @p value
         *
         * @param value
         */
        void set_all(bool value);

        /**
         * @brief Get value of relation between @p p and @p q
         *  If either @p p or @p q is bigger than the size of relation, behaviour is undefined
         * @param p
         * @param q
         */
        bool get(state_type_t p, state_type_t q) const;

        /**
         * @brief Return size of relation (shape)
         *
         * @return size of the relation
         */
        state_type_t size() const;

        /**
         * @brief Complement the relation
         *
         */
        void complement();

        state_type_t rel_size() const;

    protected:
        std::vector<std::vector<bool>> m_relation;
        state_type_t m_size;
    };

    class FiniteAutomata {
    public:
        /**
         * @brief Load an NFA from vata2 format from @p stream
         *
         * @param stream
         */
        void Load(std::ifstream  &stream);

        /**
         * @brief Print an NFA in a vata2 format to @p stream
         *
         * @param stream
         */
        void Print(std::ostream &stream);

        /**
         * @brief Delete all unreachable and non-terminating states
         *
         * @return number of deleted states
         */
        state_type_t Minimize();

        /**
         * @brief Compute simulation relation on states of this finite automaton
         *
         * @return BinaryRelation - simulation
         */
        BinaryRelation * SimulationRelation() const;

        /**
         * @brief Compute identity relation on states of this finite automaton
         *
         * @return BinaryRelation - identity
         */
        BinaryRelation * IdentityRelation() const;

        BinaryRelation * SimulationRelation_simlib(std::chrono::microseconds &conversion_time) const;

        /**
         * @brief Checks universality of this finite automaton
         *
         * @param relation binary relation, which implies language inclusion
         * @return true if universal
         */
        bool isUniversal(const BinaryRelation& relation) const;

        bool isIncluded(FiniteAutomata &another_automaton, FiniteAutomata &union_automaton , BinaryRelation relation) const;

        state_type_t StatesSize() const;

        symbol_type_t AlphabetSize() const;

        bool InsertState(std::string& state);

        bool DeleteState(std::string& state);

        bool InsertStartState(std::string& state);

        bool RemoveStartState(std::string& state);

        bool InsertFinalState(std::string& state);

        bool RemoveFinalState(std::string& state);

        bool InsertSymbol(std::string& symbol);

        bool InsertTransition(std::string& state1, std::string& symbol, std::string& state2);

        /**
         * @brief Make all states in automaton final
         *
         */
        void MakeAllFinal();

        /**
         * @brief Changes names of states in finite automaton @p another_automaton so union of these two can be made. Changes names of states in @p another_automaton .
         *
         * @param another_automaton
         */
        void MakeDifferent(FiniteAutomata &another_automaton);

        /**
         * @brief Create union of this and @p another_automaton. Assumes both have the same alphabet and are different in terms of state names.
         * Use FA::FiniteAutomata::MakeDifferent before calling this function.
         *
         * @param another_automaton
         * @return union of this and @p another_automaton
         */
        FiniteAutomata Union(const FiniteAutomata &another_automaton) const;

        std::unordered_map<state_type_t, size_t> dist_to_final() const;

        void print_distances_to_final(std::ostream &stream) const;

    protected:
        std::string Name;
        States states;
        Alphabet alphabet;
        TransitionFunction trans_function;
        States states_start;
        States states_final;

        void print_transition_function(std:: ostream &stream, TransitionFunction &trans_function);

        state_type_t minimize_delete_unreachable();

        state_type_t minimize_delete_nonterm();

        static std::set<product_state_t> inclusion_initialize(std::set<product_state_t> p_state, BinaryRelation relation);

        std::set<product_state_t> post_product_state(product_state_t p_state) const;

        std::set<macro_state_t> post_macro_state(std::set<state_type_t> macro_state) const;

        std::set<state_type_t> post_state(state_type_t state, symbol_type_t symbol) const;

        static void minimize_macro_state(std::set<state_type_t> &macro_state, BinaryRelation relation);

        bool is_macro_state_accepting(std::set<state_type_t> &macro_state) const;

        static bool is_product_state_accepting(product_state_t p_state, const States &final_a, const States &final_b);

        static bool is_macro_state_smaller(std::set<state_type_t> macro_state1, std::set<state_type_t> macro_state2, BinaryRelation relation);

        /**
         * @brief If one of the states is not in the relation, behaviour is undefined
         *
         * @param state1
         * @param state2
         * @param relation binary relation which implies language inclusion
         * @return true if language of state @p state1 is included in language of state @p state2
         */
        static bool is_state_smaller(state_type_t state1, state_type_t state2, BinaryRelation relation);

        /**
         * @brief If state @p name_orig doesn't exist or if state @p name_new exists, does nothing
         *
         * @param name_orig
         * @param name_new
         */
        void rename_state(std::string name_orig, std::string name_new);

    private:
        std::unordered_map<state_type_t, size_t> dist_init_inf() const;
        void set_distances(std::set<state_type_t> &states, std::unordered_map<state_type_t, size_t> &distances, size_t dist) const;
        static std::set<state_type_t> all_reachable_states(std::set<state_type_t> &states, TransitionFunction &trans_funct);

    };//FiniteAutomata

}
