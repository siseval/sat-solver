#include "formula.h"

#include <string>
#include <vector>
#include <string_view>
#include <charconv>

namespace solver
{

Formula::Formula(const std::string cnf_string)
{
    parse_cnf_string(cnf_string);
}

bool Formula::parse_cnf_string(const std::string cnf_string)
{
    clause_literals.clear();
    clause_offsets.clear();
    clause_count = 0;

    if (cnf_string.empty()) 
    {
        return false;
    }

    std::string_view sv(cnf_string);
    std::size_t pos = 0;

    clause_offsets.push_back(0);

    while (pos < sv.size()) {
        std::size_t next_comma = sv.find(',', pos);
        std::string_view clause_sv = (next_comma == std::string_view::npos) 
                                        ? sv.substr(pos) 
                                        : sv.substr(pos, next_comma - pos);

        std::size_t lit_pos = 0;
        bool clause_has_literals = false;

        while (lit_pos < clause_sv.size()) 
        {
            std::size_t next_plus = clause_sv.find('+', lit_pos);
            std::string_view lit_sv = (next_plus == std::string_view::npos)
                                        ? clause_sv.substr(lit_pos)
                                        : clause_sv.substr(lit_pos, next_plus - lit_pos);

            if (!lit_sv.empty()) 
            {
                Literal lit = 0;
                auto [ptr, ec] = std::from_chars(lit_sv.data(), lit_sv.data() + lit_sv.size(), lit);

                if (ec != std::errc{}) 
                {
                    return false;
                }

                clause_literals.push_back(lit);
                clause_has_literals = true;

                if (lit > max_literal)
                {
                    max_literal = lit;
                }
            }

            if (next_plus == std::string_view::npos) 
            {
                break;
            }
            lit_pos = next_plus + 1;
        }

        if (clause_has_literals) 
        {
            clause_count++;
            clause_offsets.push_back(clause_literals.size());
        }

        if (next_comma == std::string_view::npos) 
        {
            break;
        }
        pos = next_comma + 1;
    }

    if (clause_count == 0) 
    {
        clause_offsets.clear();
        return false;
    }

    return true;
}

Literal Formula::get_max_literal() const
{
    return max_literal;
}

std::size_t Formula::get_clause_count() const 
{ 
    return clause_count; 
}

const std::vector<Literal>& Formula::get_literals() const 
{ 
    return clause_literals; 
}

const std::vector<std::size_t>& Formula::get_offsets() const 
{ 
    return clause_offsets;
}

void Formula::print() const
{
    for (std::size_t i = 0; i < get_clause_count(); ++i)
    {
        std::printf("(");

        std::size_t clause_start = get_offsets()[i];
        std::size_t clause_end = get_offsets()[i + 1];

        for (std::size_t j = clause_start; j < clause_end; ++j)
        {
            Literal literal = get_literals()[j];
            std::printf("%lld", literal);
            if (j < clause_end - 1)
            {
                std::printf(" + ");
            }
        }

        std::printf(")");
        if (i < get_clause_count() - 1)
        {
            std::printf(", ");
        }
        else 
        {
            std::printf("\n");
        }
    }
}

}

