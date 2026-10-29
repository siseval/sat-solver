#pragma once

#include <vector>

namespace solver
{

using Literal = std::int64_t;

class Formula
{
private:

    std::size_t clause_count;
    Literal max_literal = 0;
    std::vector<Literal> clause_literals;
    std::vector<std::size_t> clause_offsets;

public:

    Formula(const std::string cnf_string);

    bool parse_cnf_string(const std::string cnf_string);

    std::size_t get_clause_count() const;
    Literal get_max_literal() const;
    const std::vector<Literal>& get_literals() const;
    const std::vector<std::size_t>& get_offsets() const;

    void print() const;
};

}
