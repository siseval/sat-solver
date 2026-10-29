#pragma once

#include <optional>

#include "formula.h"
#include "problem.h"

namespace solver
{

class Solver 
{

public:

    Solver();

    bool solve(Problem& problem);

private:

    enum class SatState
    { 
        SATISFIABLE,
        UNSATISFIABLE,
        UNKNOWN
    };

    struct UndoStackDeltas
    {
        std::size_t clauses_satisfied = 0;
        std::size_t literal_num_occurences = 0;
        std::size_t clauses_remaining_literals = 0;
        std::size_t assignments = 0;
        std::size_t num_clauses_satisfied = 0;
    };

    struct UndoStacks
    {
        std::vector<std::size_t> clauses_satisfied;
        std::vector<Literal> literal_num_occurences;
        std::vector<std::size_t> clauses_remaining_literals;
        std::vector<Literal> assigments;
    };

    UndoStacks undo_stacks;

    void simplify(Problem& problem, Literal literal, UndoStackDeltas& stack_deltas);
    void undo(Problem& problem, UndoStackDeltas& stack_deltas);

    SatState check_satisfiability(Problem& problem);
    std::optional<Literal> find_pure_literal(Problem& problem);
    std::optional<Literal> find_unit(Problem& problem);
};

}
