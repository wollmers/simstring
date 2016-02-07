#include <iostream>
#include <locale>
#include <string>
#include <simstring/simstring.h>

void retrieve(
    simstring::reader& dbr,
    const std::string& query,
    int measure,
    double threshold
    )
{
    // Retrieve similar strings into a string vector.
    std::vector<std::string> xstrs;
    dbr.retrieve(query, measure, threshold, std::back_inserter(xstrs));

    // Output the retrieved strings separated by ", ".
    for (int i = 0;i < (int)xstrs.size();++i) {
        std::cout << (i != 0 ? ", " : "") << xstrs[i];
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    // Activate std::wcout.
    std::locale::global(std::locale("")); 
    std::wcout.imbue(std::locale(""));

    // Open a SimString database for writing (with std::wstring).
    simstring::ngram_generator gen(3, false);
    simstring::writer_base<std::string> dbw(gen, "sample_unicode.db");
    dbw.insert("スパゲティ");
    dbw.close();

    // Open the database for reading.
    simstring::reader dbr;
    dbr.open("sample_unicode.db");

    // Output similar strings from Unicode queries.
    retrieve(dbr, "スパゲティー", simstring::cosine, 0.6);

    return 0;
}
