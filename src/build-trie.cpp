#include <iostream>
#include <fstream>
#include <pcrecpp.h>
#include <vector>
#include <sstream>
#include <math.h>

int process_reading(std::string& reading)
{
    static pcrecpp::RE pattern("([‐・◦])", pcrecpp::UTF8());
    return pattern.GlobalReplace("", &reading);
}

std::vector<std::string> process_reading_into_sections(std::string& reading)
{
    static pcrecpp::RE pattern("(.*?)([‐・◦])", pcrecpp::UTF8());
    pcrecpp::StringPiece input(reading);

    std::vector<std::string> sections;
    std::string section;
    while ( pattern.Consume(&input, &section) )
    {
        sections.emplace_back(std::move(section));
    }

    std::string mutated_input = input.as_string();
    sections.emplace_back(std::string(std::make_move_iterator(mutated_input.begin()),
                                      std::make_move_iterator(mutated_input.end())));

    for ( const std::string& x : sections )
    {
        std::cout << x << ", ";
    }
    std::cout << "\n";

    return sections;
}

// struct
// {
//     bool multiple_alternatives;
//     bool multiple_
    
// };

// Reading: あい‐うち
// Alternatives: 【相打ち／相撃ち】
// Reading: あい‐う・つ
// Alternatives: 【相打つ／相撃つ】
// Reading: あい‐えん
// Alternatives: 【合（い）縁／相縁／愛縁】
// Reading: あいえん‐きえん
// Alternatives: 【合（い）縁奇縁／相縁機縁】
// Reading: あい‐みつもり
// Alternatives: 【合（い）見積（も）り／相見積（も）り】
// Reading: アイリス‐にんしょう
// Alternatives: 【アイリス認証】
std::vector<std::string> process_alternatives(int entry_number, std::string& reading, std::string& alternatives)
{
    std::vector<std::string> entry_variations;
    int number_of_separators = process_reading(reading);

    entry_variations.emplace_back(std::move(reading));

    static pcrecpp::RE pattern("(.*?)([／])", pcrecpp::UTF8());
    pcrecpp::StringPiece input(alternatives);

    std::vector<std::string> sections;
    std::string section;
    while ( pattern.Consume(&input, &section) )
    {
        sections.emplace_back(std::move(section));
    }

    std::string mutated_input = input.as_string();
    sections.emplace_back(std::string(std::make_move_iterator(mutated_input.begin()),
                                      std::make_move_iterator(mutated_input.end())));

    static pcrecpp::RE okurigana_filter_pattern("（", pcrecpp::UTF8());
    for ( std::string& section : sections )
    {
        if ( okurigana_filter_pattern.PartialMatch(section) )
        {
            std::vector<std::string> components;
            std::string compulsory;
            std::string optional;
            static pcrecpp::RE okurigana_pattern("(.*?)（(.*?)）");
            pcrecpp::StringPiece input(section);

            while ( okurigana_pattern.Consume(&input, &compulsory, &optional) )
            {
                components.emplace_back(std::move(compulsory));
                components.emplace_back(std::move(optional));
            }
            components.emplace_back(std::string(input.data()));

            int number_of_options = pow(2, components.size() / 2);
            for ( int option_number = 0; option_number < number_of_options; ++option_number )
            {
                std::ostringstream current_option;
                for ( int i = 0; i < components.size(); ++i )
                {
                    if ( i % 2 == 1 )
                    {
                        int mask_value = (int) pow(2, ( i / 2 ));
                        // std::cout << "Masked with " << mask_value << " ";
                        if ( option_number &  mask_value )
                        {
                            current_option << components[i];
                        }
                    }
                    else
                    {
                        current_option << components[i];
                    }
                }
                entry_variations.emplace_back(std::move(current_option.str()));
            }
        }
        else
        {
            entry_variations.emplace_back(std::move(section));
        }
    }

    for ( const auto& variation : entry_variations )
    {
        std::cout << entry_number << ": " << variation << "\n";
    }

    return sections;
}

int main(int argc, char* argv[])
{
    std::ifstream input_file ("/home/dacoda/projects/goo-processing/results/readings-with-alternatives-sans-外来語");
    std::string line;

    pcrecpp::RE line_pattern("(\\d+): (.*)");

    pcrecpp::RE alternative_pattern("(.*?)(〔.*〕)?【(.*)】$");

    while ( std::getline(input_file, line) )
    {
        int entry_number;
        std::string title;
        line_pattern.FullMatch(line, &entry_number, &title);

        std::string reading;
        std::string old_readings;
        std::string alternatives;
        alternative_pattern.FullMatch(title, &reading, &old_readings, &alternatives);

        // std::cout << "Entry is " << entry_number << " and title is " << title << "\n";
        // std::cout << "Reading: " << reading << "\n";
        // std::cout << "Old readings: " << old_readings << "\n";
        // std::cout << "Alternatives: " << alternatives << "\n";
        process_alternatives(entry_number, reading, alternatives);
    }

    std::vector<pcrecpp::RE>
    {
        ".*【.*】$",
            ".*【.*】[^$]",
            ".*[^】]〔.*〕$",
            };
}
