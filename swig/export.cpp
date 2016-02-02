#include <string>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <errno.h>

#include <simstring/simstring.h>

#include "export.h"


int translate_measure(int measure)
{
    switch (measure) {
    case exact:
        return simstring::exact;
    case dice:
        return simstring::dice;
    case cosine:
        return simstring::cosine;
    case jaccard:
        return simstring::jaccard;
    case overlap:
        return simstring::overlap;
    }
    throw std::invalid_argument("Unknown similarity measure specified");
}

typedef simstring::ngram_generator ngram_generator_type;
typedef simstring::writer_base<std::string, ngram_generator_type> writer_type;
typedef simstring::writer_base<std::wstring, ngram_generator_type> uwriter_type;
typedef simstring::reader reader_type;

writer::writer(const char *filename, int n, bool be, bool unicode)
    : m_dbw(NULL), m_gen(NULL), m_unicode(unicode)
{
    ngram_generator_type *gen = new ngram_generator_type(n, be);

        writer_type *dbw = new writer_type(*gen, filename);
        if (dbw->fail()) {
            std::string message = dbw->error();
            delete dbw;
            delete gen;
            throw std::invalid_argument(message);
        }
    m_dbw = dbw;
    m_gen = gen;
}

writer::~writer()
{
    writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
    ngram_generator_type* gen = reinterpret_cast<ngram_generator_type*>(m_gen);
    
    dbw->close();
    if (dbw->fail()) {
        std::string message = dbw->error();
        delete dbw;
        delete gen;
        throw std::runtime_error(message);
    }
    delete dbw;
    delete gen;
}

void writer::insert(const char *string)
{
        writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
    dbw->insert(string);
    if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
    }
}

void writer::close()
{
        writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
        dbw->close();
        if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
        }
}

reader::reader(const char *filename)
    : m_dbr(NULL), measure(cosine), threshold(0.7)
{
    reader_type *dbr = new reader_type;

    if (!dbr->open(filename)) {
        delete dbr;
        throw std::invalid_argument("Failed to open the database");
    }

    m_dbr = dbr;
}

reader::~reader()
{
    this->close();
    delete reinterpret_cast<reader_type*>(m_dbr);
}

template <class insert_iterator_type>
void retrieve_thru(
    reader_type& dbr,
    const std::string& query,
    int measure,
    double threshold,
    insert_iterator_type ins
    )
{
    switch (measure) {
    case exact:
        dbr.retrieve<simstring::measure::exact>(query, threshold, ins);
        break;
    case dice:
        dbr.retrieve<simstring::measure::dice>(query, threshold, ins);
        break;
    case cosine:
        dbr.retrieve<simstring::measure::cosine>(query, threshold, ins);
        break;
    case jaccard:
        dbr.retrieve<simstring::measure::jaccard>(query, threshold, ins);
        break;
    case overlap:
        dbr.retrieve<simstring::measure::overlap>(query, threshold, ins);
        break;
    }
}

std::vector<std::string> reader::retrieve(const char *query)
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    std::vector<std::string> ret;

    retrieve_thru(dbr, query, this->measure, this->threshold, std::back_inserter(ret));

    return ret;
}

bool reader::check(const char *query)
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    
    if (dbr.char_size() == 1) {
        std::string qstr = query;
        return dbr.check(qstr, translate_measure(this->measure), this->threshold);
    } 
    
    return false;
}

void reader::close()
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    dbr.close();
}
