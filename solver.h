#pragma once

#include <optional>

#include "formula.h"
#include "problem.h"

namespace solver
{
    /**
     * Object containing state and behavior for solving an instance of the Problem class.
     * The Problem state is global across recursion levels, no copying is done. 
     * Instead, the UndoStacks struct contain stacks logging all state changes.
     * An instance of UndoStackDeltas is stored per recursion level, 
     * and stores how many elements was pushed to each stack on that level.
     * For example if a call to simplify satisfies two clauses, the two clause indices are pushed to
     * undo_stacks.clauses_satisfied, and the local stack_deltas.clauses_satisfied is incremented by two.
     * Then, when it needs to backtrack to the previous state, it simply pops the two indices off the stack, 
     * and sets them as unsatisfied again in the global state.
     */
    class Solver
    {
    public:

        /**
         * Constructor reserving memory for the UndoStacks as to avoid slow memory allocations later.
         */
        Solver();

        /**
         * Uses recursive DPLL to solve a SAT-problem. Each call initializes an instance of UndoStackDeltas.
         * If it ever ends up in an unsatisfiable state after simplifying, it calls undo passing the deltas.
         * @param problem Instance of the Problem class 
         * @return true if problem represents a satisfiable formula, false if not.
         */
        bool solve(Problem& problem);

    private:

        /**
         * Used when checking if the current state is satisfiable. If UNKNOWN we go deeper.
         */
        enum class SatState
        {
            SATISFIABLE,
            UNSATISFIABLE,
            UNKNOWN
        };

        /**
         * Struct storing how many changes have been made to the different states during the current call. 
         * Used to know how many logged changes to pop off the different stacks when backtracking.
         * num_clauses_satisfied is a special case, as it only models changes to a simple counter, rather than a mapping.
         * Thus, the change stored here is simply subtracted when backtracking, it does not need its own stack.
         */
        struct UndoStackDeltas
        {
            std::size_t clauses_satisfied = 0;
            std::size_t literal_num_occurrences = 0;
            std::size_t clauses_remaining_literals = 0;
            std::size_t assignments = 0;
            std::size_t num_clauses_satisfied = 0;
        };

        /**
         * Stacks logging all state changes made to the Problem instance in the course of solving it.
         * They grow when recursing deeper, and shrink when backtracking, as changes are popped and undone.
         */
        struct UndoStacks
        {
            std::vector<std::size_t> clauses_satisfied;
            std::vector<Literal> literal_num_occurrences;
            std::vector<std::size_t> clauses_remaining_literals;
            std::vector<Literal> assignments;
        };

        UndoStacks undo_stacks;

        /**
         * Marks clauses containing the given literal as satisfied, and increments the number of satisfied clauses.
         * Decrements the negated literals number of occurrences for each clause it is found in. 
         * This models removing it from each clause, without actually doing expensive collection changes. 
         * @param problem Problem state
         * @param literal Literal discovered to be pure, or in a unit clause
         * @param stack_deltas Reference to stack deltas to be updated as state-changes are made while simplifying.
         */
        void simplify(Problem& problem, Literal literal, UndoStackDeltas& stack_deltas);
        /**
         * Restores problem state to how it was previous to this call to solve.
         * This is done by popping changes from each undo stack as many times as 
         * instructed by stack_deltas and undoing each change.
         * @param problem Problem state
         * @param stack_deltas 
         */
        void undo(Problem& problem, const UndoStackDeltas& stack_deltas);

        /**
         * 
         * @param problem Problem state
         * @return SATISFIABLE if the number of satisfied clauses equals the number of total clauses.
         * UNSATISFIABLE if a clause is marked as containing zero literals in problem.clauses_remaining_literals.
         * UNKNOWN otherwise.
         */
        static SatState check_satisfiability(Problem& problem);
        /**
         * Finds and returns an unassigned pure literal.
         * @param problem Problem state
         * @return An optional literal that only occurs with one polarity if it is found. std::nullopt otherwise.
         */
        static std::optional<Literal> find_pure_literal(Problem& problem);

        /**
         * Finds and returns an unassigned literal contained in a unit clause.
         * @param problem Problem state
         * @return An optional literal that occurs in a unit clause if it is found. std::nullopt otherwise.
         */
        static std::optional<Literal> find_unit(Problem& problem);
    };
}
