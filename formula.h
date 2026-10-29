#pragma once

#include <vector>

namespace solver
{
    using Literal = std::int64_t;

    /**
     * Object modeling a CNF-formula as a flat structure of literals, 
     * with a supporting list of offsets for the start of each clause.
     */
    class Formula
    {
        std::size_t clause_count;
        Literal max_literal = 0;
        std::vector<Literal> clause_literals;
        std::vector<std::size_t> clause_offsets;

    public:

        /**
         * Constructor which calls parse_cnf_string.
         * @param cnf_string string representing a CNF-formula on the specified format.
         */
        explicit Formula(std::string cnf_string);

        /**
         * Updates state by parsing a formula-string.
         * Formula MUST be on the form: c1,c2...,ci
         * where c1 through ci are clauses on the form: n1+n2...+ni
         * where n1 through ni are positive integers representing literals
         * and -ni represents the negation of ni
         * The list of unique integers n1 through ni SHOULD be continuous with no gaps
         * @param cnf_string string representing a CNF-formula on the specified format.
         * @return 
         */
        bool parse_cnf_string(std::string cnf_string);

        std::size_t get_clause_count() const;
        Literal get_max_literal() const;
        const std::vector<Literal>& get_literals() const;
        const std::vector<std::size_t>& get_offsets() const;

        void print() const;
    };
}
