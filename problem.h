#pragma once

#include <vector>
#include <unordered_set>

#include "formula.h"

namespace solver
{

class Assignment
{

public:

    Assignment(std::size_t count) 
        : count(count)
        , assigned(std::vector<bool>(count))
        , values(std::vector<bool>(count)) {}

    void set_assigned(Literal literal, bool value);
    void set_value(Literal literal, bool value);

    bool get_assigned(Literal literal);
    bool get_value(Literal literal);

    void print();

private:

    std::size_t count;
    std::vector<bool> assigned;
    std::vector<bool> values;
};

struct Problem
{
    Formula formula;
    Assignment assignment;
    std::vector<std::unordered_set<std::size_t>> literal_occurences;

    std::vector<bool> clauses_satisfied;
    std::vector<std::size_t> literal_num_occurences;
    std::vector<std::size_t> clauses_remaining_literals;
    std::size_t num_clauses_satisfied = 0;

    Problem(const Formula& formula);

    std::unordered_set<std::size_t>& get_occurences(const Literal literal);
    std::size_t get_num_occurences(const Literal literal);
    void increment_num_occurences(const Literal literal, const int delta);
    void assign(const Literal literal, const bool value);
    void assign_true(const Literal literal);
};

}
