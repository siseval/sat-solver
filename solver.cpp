#include "solver.h"

namespace solver
{
    Solver::Solver()
    {
        undo_stacks.clauses_satisfied.reserve(0xFFFF);
        undo_stacks.literal_num_occurrences.reserve(0xFFFF);
        undo_stacks.clauses_remaining_literals.reserve(0xFFFF);
        undo_stacks.assignments.reserve(0xFFFF);
    }

    bool Solver::solve(Problem& problem)
    {
        UndoStackDeltas stack_deltas;

        while (std::optional<Literal> literal = find_unit(problem))
        {
            problem.assign_true(literal.value());
            undo_stacks.assignments.emplace_back(std::abs(literal.value()));
            stack_deltas.assignments++;

            simplify(problem, literal.value(), stack_deltas);

            const SatState state = check_satisfiability(problem);
            if (state == SatState::SATISFIABLE)
            {
                return true;
            }
            if (state == SatState::UNSATISFIABLE)
            {
                undo(problem, stack_deltas);
                return false;
            }
        }

        while (std::optional<Literal> literal = find_pure_literal(problem))
        {
            problem.assign_true(literal.value());
            undo_stacks.assignments.emplace_back(std::abs(literal.value()));
            stack_deltas.assignments++;

            simplify(problem, literal.value(), stack_deltas);

            const SatState state = check_satisfiability(problem);
            if (state == SatState::SATISFIABLE)
            {
                return true;
            }
            if (state == SatState::UNSATISFIABLE)
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
            undo_stacks.assignments.emplace_back(random);
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
            undo_stacks.assignments.emplace_back(random);
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

    void Solver::simplify(Problem& problem, const Literal literal, UndoStackDeltas& stack_deltas)
    {
        for (std::size_t clause_index : problem.get_occurrences(literal))
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

            const std::size_t clause_start = problem.formula.get_offsets()[clause_index];
            const std::size_t clause_end = problem.formula.get_offsets()[clause_index + 1];

            for (std::size_t i = clause_start; i < clause_end; ++i)
            {
                Literal l = problem.formula.get_literals()[i];
                problem.increment_num_occurrences(l, -1);
                undo_stacks.literal_num_occurrences.emplace_back(l);
                stack_deltas.literal_num_occurrences++;
            }
        }
        for (std::size_t clause_index : problem.get_occurrences(-literal))
        {
            problem.clauses_remaining_literals[clause_index] -= 1;
            undo_stacks.clauses_remaining_literals.emplace_back(clause_index);
            stack_deltas.clauses_remaining_literals++;
        }
    }

    void Solver::undo(Problem& problem, const UndoStackDeltas& stack_deltas)
    {
        for (std::size_t i = 0; i < stack_deltas.clauses_satisfied; ++i)
        {
            problem.clauses_satisfied[undo_stacks.clauses_satisfied.back()] = false;
            undo_stacks.clauses_satisfied.pop_back();
        }

        for (std::size_t i = 0; i < stack_deltas.literal_num_occurrences; ++i)
        {
            problem.increment_num_occurrences(undo_stacks.literal_num_occurrences.back(), 1);
            undo_stacks.literal_num_occurrences.pop_back();
        }

        for (std::size_t i = 0; i < stack_deltas.clauses_remaining_literals; ++i)
        {
            problem.clauses_remaining_literals[undo_stacks.clauses_remaining_literals.back()]++;
            undo_stacks.clauses_remaining_literals.pop_back();
        }

        for (std::size_t i = 0; i < stack_deltas.assignments; ++i)
        {
            problem.assignment.set_assigned(undo_stacks.assignments.back(), false);
            undo_stacks.assignments.pop_back();
        }

        problem.num_clauses_satisfied -= stack_deltas.num_clauses_satisfied;
    }

    Solver::SatState Solver::check_satisfiability(Problem& problem)
    {
        for (std::size_t i = 0; i < problem.formula.get_clause_count(); ++i)
        {
            if (!problem.clauses_satisfied[i] && problem.clauses_remaining_literals[i] <= 0)
            {
                return SatState::UNSATISFIABLE;
            }
        }

        if (problem.num_clauses_satisfied >= problem.formula.get_clause_count())
        {
            return SatState::SATISFIABLE;
        }

        return SatState::UNKNOWN;
    }

    std::optional<Literal> Solver::find_pure_literal(Problem& problem)
    {
        for (Literal i = 1; i <= problem.formula.get_max_literal(); ++i)
        {
            if (problem.assignment.get_assigned(i))
            {
                continue;
            }
            if (problem.get_num_occurrences(i) > 0 && problem.get_num_occurrences(-i) <= 0)
            {
                return i;
            }
            if (problem.get_num_occurrences(-i) > 0 && problem.get_num_occurrences(i) <= 0)
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
                const std::size_t clause_start = problem.formula.get_offsets()[i];
                const std::size_t clause_end = problem.formula.get_offsets()[i + 1];

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
}
