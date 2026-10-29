#pragma once

#include <unordered_set>
#include <vector>

#include "assignment.h"
#include "formula.h"


namespace solver
{
    /**
     * Class modeling a SAT-problem.
     * It contains a CNF-formula, an assignment, 
     * as well as mutable state that is changed in the course of solving the problem
     */
    struct Problem
    {
        const Formula formula;
        Assignment assignment;
        /**
         * A mapping from literals to the clauses where they are found. 
         * This is used to avoid iterating unnecessarily to find relevant clauses, but it is not updated.
         */
        std::vector<std::unordered_set<std::size_t>> literal_occurrences;

        /**
         * Mapping from clause index to bool indicating whether the clause is satisfied.
         */
        std::vector<bool> clauses_satisfied;
        
        /**
         * Mapping from literal to its number of occurrences in the formula.
         * Positive literals span from the start to the middle of the vector, 
         * negative (negated) literals span from the end to the middle.
         */
        std::vector<std::size_t> literal_num_occurrences;
        
        /**
         * Mapping from clause index to how many literals in that clause 
         * have not yet been removed through simplification.
         */
        std::vector<std::size_t> clauses_remaining_literals;
        
        /**
         * Total number of satisfied clauses.
         */
        std::size_t num_clauses_satisfied = 0;

        /**
         * Constructor which prepares the mappings, and populates the literal_occurrences mapping.
         * @param formula CNF-formula object.
         */
        explicit Problem(const Formula& formula);

        std::unordered_set<std::size_t>& get_occurrences(Literal literal);
        std::size_t get_num_occurrences(Literal literal) const;
        void increment_num_occurrences(Literal literal, int delta);
        void assign(Literal literal, bool value);
        void assign_true(Literal literal);
    };
}
