#include "solver.h"

namespace solver
{

Solver::Solver()
{
    undo_stacks.clauses_satisfied.reserve(0xFFFF);
    undo_stacks.literal_num_occurences.reserve(0xFFFF);
    undo_stacks.clauses_remaining_literals.reserve(0xFFFF);
    undo_stacks.assigments.reserve(0xFFFF);
}

std::optional<Literal> Solver::find_pure_literal(Problem& problem)
{
    for (Literal i = 1; i <= problem.formula.get_max_literal(); ++i)
    {
        if (problem.assignment.get_assigned(i))
        {
            continue;
        }
        if (problem.get_num_occurences(i) > 0 && problem.get_num_occurences(-i) <= 0)
        {
            return i;
        }
        if (problem.get_num_occurences(-i) > 0 && problem.get_num_occurences(i) <= 0)
        {
            return -i;
        }
    }
    return std::nullopt;
}

std::optional<Literal> Solver::find_unit(Problem& problem)
{
    for (std::size_t i = 0; i < problem.formula.get_clause_count(); ++i)
    {
        if (!problem.clauses_satisfied[i] && problem.clauses_remaining_literals[i] == 1)
        {
            std::size_t clause_start = problem.formula.get_offsets()[i];
            std::size_t clause_end = problem.formula.get_offsets()[i + 1];

            for (std::size_t j = clause_start; j < clause_end; ++j)
            {
                Literal literal = problem.formula.get_literals()[j];
                if (!problem.assignment.get_assigned(std::abs(literal)))
                {
                    return literal;
                }
            }
        }
    }
    return std::nullopt;
}

Solver::SatState Solver::check_satisfiability(Problem& problem)
{
    for (std::size_t i = 0; i < problem.formula.get_clause_count(); ++i)
    {
        if (!problem.clauses_satisfied[i] && problem.clauses_remaining_literals[i] <= 0)
        {
            return Solver::SatState::UNSATISFIABLE;
        }
    }

    if (problem.num_clauses_satisfied >= problem.formula.get_clause_count())
    {
        return Solver::SatState::SATISFIABLE;
    }

    return Solver::SatState::UNKNOWN;
}

bool Solver::solve(Problem& problem)
{
    UndoStackDeltas stack_deltas;

    while (std::optional<Literal> literal = find_unit(problem))
    {
        problem.assign_true(literal.value());
        undo_stacks.assigments.emplace_back(std::abs(literal.value()));
        stack_deltas.assignments++;

        simplify(problem, literal.value(), stack_deltas);

        Solver::SatState state = check_satisfiability(problem);
        if (state == SatState::SATISFIABLE) 
        {
            return true;
        }
        else if (state == SatState::UNSATISFIABLE) 
        {
            undo(problem, stack_deltas);
            return false; 
        }
    }

    while (std::optional<Literal> literal = find_pure_literal(problem))
    {
        problem.assign_true(literal.value());
        undo_stacks.assigments.emplace_back(std::abs(literal.value()));
        stack_deltas.assignments++;

        simplify(problem, literal.value(), stack_deltas);

        Solver::SatState state = check_satisfiability(problem);
        if (state == SatState::SATISFIABLE) 
        { 
            return true; 
        }
        else if (state == SatState::UNSATISFIABLE) 
        {
            undo(problem, stack_deltas);
            return false; 
        }
    }

    Literal random = 0;
    for (Literal i = 1; i <= problem.formula.get_max_literal(); ++i)
    {
        if (!problem.assignment.get_assigned(i))
        {
            random = i;
            break;
        }
    }
    if (random == 0)
    {
        undo(problem, stack_deltas);
        return false;
    }

    {
        UndoStackDeltas branch_deltas;
        problem.assign_true(random);
        undo_stacks.assigments.emplace_back(random);
        branch_deltas.assignments++;

        simplify(problem, random, branch_deltas);

        if (check_satisfiability(problem) != SatState::UNSATISFIABLE)
        {
            if (solve(problem)) 
            { 
                return true; 
            }
        }
        undo(problem, branch_deltas);
    }

    {
        UndoStackDeltas branch_deltas;
        problem.assign_true(-random);
        undo_stacks.assigments.emplace_back(random);
        branch_deltas.assignments++;

        simplify(problem, -random, branch_deltas);

        if (check_satisfiability(problem) != SatState::UNSATISFIABLE)
        {
            if (solve(problem)) 
            {
                return true; 
            }
        }
        undo(problem, branch_deltas);
    }

    undo(problem, stack_deltas);
    return false;
}

void Solver::undo(Problem& problem, UndoStackDeltas& stack_deltas)
{
    for (std::size_t i = 0; i < stack_deltas.clauses_satisfied; ++i)
    {
        problem.clauses_satisfied[undo_stacks.clauses_satisfied.back()] = false;
        undo_stacks.clauses_satisfied.pop_back();
    }

    for (std::size_t i = 0; i < stack_deltas.literal_num_occurences; ++i)
    {
        problem.increment_num_occurences(undo_stacks.literal_num_occurences.back(), 1);
        undo_stacks.literal_num_occurences.pop_back();
    }

    for (std::size_t i = 0; i < stack_deltas.clauses_remaining_literals; ++i)
    {
        problem.clauses_remaining_literals[undo_stacks.clauses_remaining_literals.back()]++;
        undo_stacks.clauses_remaining_literals.pop_back();
    }

    for (std::size_t i = 0; i < stack_deltas.assignments; ++i)
    {
        problem.assignment.set_assigned(undo_stacks.assigments.back(), false);
        undo_stacks.assigments.pop_back();
    }

    problem.num_clauses_satisfied -= stack_deltas.num_clauses_satisfied;
}

void Solver::simplify(Problem& problem, Literal literal, UndoStackDeltas& stack_deltas)
{
    for (std::size_t clause_index : problem.get_occurences(literal))
    {
        if (problem.clauses_satisfied[clause_index])
        {
            continue;
        }

        problem.clauses_satisfied[clause_index] = true;
        undo_stacks.clauses_satisfied.emplace_back(clause_index);
        stack_deltas.clauses_satisfied++;

        problem.num_clauses_satisfied++;
        stack_deltas.num_clauses_satisfied++;

        std::size_t clause_start = problem.formula.get_offsets()[clause_index];
        std::size_t clause_end = problem.formula.get_offsets()[clause_index + 1];

        for (std::size_t i = clause_start; i < clause_end; ++i)
        {
            Literal l = problem.formula.get_literals()[i];
            problem.increment_num_occurences(l, -1);
            undo_stacks.literal_num_occurences.emplace_back(l);
            stack_deltas.literal_num_occurences++;
        }
    }
    for (std::size_t clause_index : problem.get_occurences(-literal))
    {
        problem.clauses_remaining_literals[clause_index] -= 1;
        undo_stacks.clauses_remaining_literals.emplace_back(clause_index);
        stack_deltas.clauses_remaining_literals++;
    }
}

}
