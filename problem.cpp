#include "problem.h"

#include <print>

namespace solver
{

void Assignment::print()
{
    std::print("(");
    for (std::size_t i = 1; i <= assigned.size(); i++)
    {
        std::printf("%ld=", i);
        if (!get_assigned(i))
        {
            std::print("X");
        }
        else
        {
            std::printf("%c", (get_value(i) ? 'T' : 'F'));
        }

        if (i < assigned.size())
        {
            std::print(", ");
        }
    }
    std::print(")\n");
}

void Assignment::set_assigned(Literal literal, bool value)
{
    assigned[std::abs(literal) - 1] = value;
}

void Assignment::set_value(Literal literal, bool value)
{
    values[std::abs(literal) - 1] = value;
}

bool Assignment::get_assigned(Literal literal)
{
    return assigned[std::abs(literal) - 1];
}

bool Assignment::get_value(Literal literal)
{
    return values[std::abs(literal) - 1];
}


Problem::Problem(const Formula& formula)
    : formula(formula)
    , assignment(formula.get_max_literal())
    , clauses_satisfied(formula.get_clause_count())
    , clauses_remaining_literals(formula.get_clause_count())
    , literal_occurences(2 * formula.get_max_literal())
    , literal_num_occurences(2 * formula.get_max_literal())
{
    for (std::size_t i = 0; i < formula.get_clause_count(); ++i)
    {
        std::size_t clause_start = formula.get_offsets()[i];
        std::size_t clause_end = formula.get_offsets()[i + 1];

        clauses_remaining_literals[i] = clause_end - clause_start;

        for (std::size_t j = clause_start; j < clause_end; ++j)
        {
            Literal literal = formula.get_literals()[j];
            get_occurences(literal).emplace(i);
            increment_num_occurences(literal, 1);
        }
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

std::unordered_set<size_t>& Problem::get_occurences(Literal literal)
{
    if (literal > 0)
    {
        return literal_occurences[literal - 1];
    }
    else
    {
        return literal_occurences[literal_occurences.size() + literal];
    }
}

std::size_t Problem::get_num_occurences(Literal literal)
{
    if (literal > 0)
    {
        return literal_num_occurences[literal - 1];
    }
    else
    {
        return literal_num_occurences[literal_occurences.size() + literal];
    }
}

void Problem::increment_num_occurences(Literal literal, int delta)
{
    if (literal > 0)
    {
        literal_num_occurences[literal - 1] += delta;
    }
    else
    {
        literal_num_occurences[literal_occurences.size() + literal] += delta;
    }
}

}
