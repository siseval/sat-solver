#include "problem.h"

namespace solver
{
    Problem::Problem(const Formula& formula)
        : formula(formula)
        , assignment(formula.get_max_literal())
        , literal_occurrences(2 * formula.get_max_literal())
        , clauses_satisfied(formula.get_clause_count())
        , literal_num_occurrences(2 * formula.get_max_literal())
        , clauses_remaining_literals(formula.get_clause_count())
    {
        for (std::size_t i = 0; i < formula.get_clause_count(); ++i)
        {
            const std::size_t clause_start = formula.get_offsets()[i];
            const std::size_t clause_end = formula.get_offsets()[i + 1];

            clauses_remaining_literals[i] = clause_end - clause_start;

            for (std::size_t j = clause_start; j < clause_end; ++j)
            {
                const Literal literal = formula.get_literals()[j];
                get_occurrences(literal).emplace(i);
                increment_num_occurrences(literal, 1);
            }
        }
    }

    std::unordered_set<size_t>& Problem::get_occurrences(const Literal literal)
    {
        if (literal > 0)
        {
            return literal_occurrences[literal - 1];
        }
        return literal_occurrences[literal_occurrences.size() + literal];
    }

    std::size_t Problem::get_num_occurrences(const Literal literal) const
    {
        if (literal > 0)
        {
            return literal_num_occurrences[literal - 1];
        }
        return literal_num_occurrences[literal_occurrences.size() + literal];
    }

    void Problem::increment_num_occurrences(const Literal literal, const int delta)
    {
        if (literal > 0)
        {
            literal_num_occurrences[literal - 1] += delta;
        }
        else
        {
            literal_num_occurrences[literal_occurrences.size() + literal] += delta;
        }
    }

    void Problem::assign(const Literal literal, const bool value)
    {
        assignment.set_assigned(literal, true);
        assignment.set_value(literal, value);
    }

    void Problem::assign_true(const Literal literal)
    {
        if (literal > 0)
        {
            assign(literal, true);
        }
        else
        {
            assign(literal, false);
        }
    }
}
