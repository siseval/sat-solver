#include <chrono>
#include <filesystem>
#include <fstream>
#include <print>
#include <string>
#include "solver.h"

const std::string FORMULA_FILENAME = std::string(SOURCE_DIR) + "/formula.txt";

void print_help()
{
    std::println("Formula MUST be on the form: c1,c2...,ci");
    std::println("where c1 through ci are clauses on the form: n1+n2...+ni");
    std::println("where n1 through ni are positive integers representing literals");
    std::println("and -ni represents the negation of ni.");
    std::println("The list of unique integers n1 through ni SHOULD be continuous with no gaps.");
}

void create_sample_formula_file(const std::string& filename)
{
    std::ofstream outfile(filename);
    if (outfile.is_open())
    {
        const char* clauses[8] = {
            "1+2+3",
            "1+2+-3",
            "1+-2+3",
            "1+-2+-3",
            "-1+2+3",
            "-1+2+-3",
            "-1+-2+3",
            "-1+-2+-3"
        };

        for (int skip = -1; skip < 8; ++skip)
        {
            bool first = true;
            for (int i = 0; i < 8; ++i)
            {
                if (i == skip)
                {
                    continue;
                }
                if (!first)
                {
                    outfile << ",";
                }
                outfile << clauses[i];
                first = false;
            }
            outfile << "\n";
        }
        outfile.close();
    }
}

bool validate_formula_format(const std::string& content)
{
    if (content.empty())
        return false;

    bool has_digit = false;
    for (const char c : content)
    {
        if (std::isdigit(c))
            has_digit = true;
        else if (c != ',' && c != '+' && c != '-' && !std::isspace(static_cast<unsigned char>(c)))
        {
            return false;
        }
    }
    return has_digit;
}

std::string generate_formula(const std::size_t num_literals)
{
    const std::size_t num_clauses = static_cast<std::size_t>(1) << num_literals;
    std::string formula;

    for (std::size_t mask = 0; mask < num_clauses; ++mask)
    {
        for (std::size_t j = 1; j <= num_literals; ++j)
        {
            const bool negate = (mask >> (num_literals - j)) & 1;
            if (negate)
            {
                formula += "-";
            }
            formula += std::to_string(j);
            if (j < num_literals)
            {
                formula += "+";
            }
        }
        if (mask < num_clauses - 1)
        {
            formula += ",";
        }
    }
    return formula;
}

int main(const int argc, char* argv[])
{
    auto run_formula = [](const std::string& formula_str) {
        const auto t0 = std::chrono::high_resolution_clock::now();

        const solver::Formula formula(formula_str);
        solver::Problem problem(formula);
        solver::Solver solver_instance;

        if (problem.formula.get_literals().size() < 64)
        {
            std::print("Formula: ");
            problem.formula.print();
        }

        std::printf(
            "Clauses: %ld, Literals: %lld\n",
            problem.formula.get_clause_count(),
            problem.formula.get_max_literal()
        );

        const bool is_sat = solver_instance.solve(problem);
        const auto t1 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        std::println("Time elapsed: {} ms", duration);

        if (is_sat)
        {
            std::println("Result: SATISFIABLE");
            if (problem.formula.get_max_literal() < 32)
            {
                std::print("Assignment: ");
                problem.assignment.print();
            }
        }
        else
        {
            std::println("Result: UNSATISFIABLE");
        }
    };

    if (argc >= 2)
    {
        const std::size_t num_literals = std::stoull(argv[1]);
        if (num_literals > 0)
        {
            const std::string formula_string = generate_formula(num_literals);
            run_formula(formula_string);
        }
        return 0;
    }

    if (!std::filesystem::exists(FORMULA_FILENAME))
    {
        std::println("File '{}' not found.", FORMULA_FILENAME);
        create_sample_formula_file(FORMULA_FILENAME);

        std::println("An input file has been created at '{}'.", FORMULA_FILENAME);
        print_help();
        return 0;
    }

    std::ifstream infile(FORMULA_FILENAME);
    if (!infile.is_open())
    {
        std::println(stderr, "Error opening file '{}'.", FORMULA_FILENAME);
        return 1;
    }

    std::string line;
    while (std::getline(infile, line))
    {
        if (line.empty() || (line.size() == 1 && std::isspace(line[0])))
        {
            continue;
        }

        if (!validate_formula_format(line))
        {
            std::println(
                "Error parsing input file '{}', contents do not match the expected formula format.",
                FORMULA_FILENAME
            );
            print_help();
            return 1;
        }

        run_formula(line);
        std::println("----------------------------------------");
    }
    infile.close();

    return 0;
}
