/*
 * other/io.cpp
 *
 */

#include <allegro.h> // file exists
#include <fstream>
#include <sstream>

#include "io.h"

namespace IO {

Line::Line(const std::string& src)
:
    type(0),
    nr1(0),
    nr2(0),
    nr3(0)
{
    std::string::const_iterator i = src.begin();
    if (i != src.end()) type = *i++;

    bool minus1 = false;
    bool minus2 = false;
    bool minus3 = false;

    switch (type) {
    case '>':
        while (i != src.end() && *i == ' ') ++i;
        while (i != src.end()             ) text1 += *i++;
        
    case '$':
        while (i != src.end() && *i != ' ') text1 += *i++;
        while (i != src.end() && *i == ' ') ++i;
        while (i != src.end()             ) text2 += *i++;
        break;

    case '#':
        while (i != src.end() && *i != ' ') text1 += *i++;
        while (i != src.end() && *i == ' ') ++i;
        while (i != src.end() && *i != ' ') iter_to_long(i, nr1, minus1);
        break;

    default:
        // Zeile als ungueltig markieren
        type = 0;
        break;
    }

    if (minus1) nr1 *= -1;
    if (minus2) nr2 *= -1;
    if (minus3) nr3 *= -1;
}

void Line::iter_to_long(std::string::const_iterator& i, long& nr, bool& minus)
{
    if (*i == '-') minus = true;
    else {
        nr *= 10;
        nr += *i - '0';
    }
    ++i;
}



// Funtionen, die Zeilen zum Schreiben erzeugen und auf den privaten
// Konstruktor (siehe weiter unten) zurueckgreifen
typedef const std::string& Str;

Line LineSimple(const Str t1)
{
    return Line('>', t1, "", "",  0,  0,  0);
}

Line LineHash  (const Str t1, const long n1)
{
    return Line('#', t1, "", "", n1,  0,  0);
}

Line LineColon (const Str t1,  const long n1,
                const long n2, const Str t2)
{
    return Line(':', t1, t2, "", n1, n2,  0);
}

Line LineDollar(const Str t1, const Str t2)
{
    return Line('$', t1, t2, "",  0,  0,  0);
}

Line LinePlus  (const Str t1, const long n1,
                const Str t2, const Str  t3)
{
    return Line('+', t1, t2, t3, n1,  0,  0);
}


Line LineBang  (const long n1, const long n2,
                const Str  t1, const long n3)
{
    return Line('!', t1, "", "", n1, n2, n3);
}

Line LineAngle (const Str t1,  const long n1,
                const long n2, const long n3, const Str t2)
{
    return Line('<', t1, t2, "", n1, n2, n3);
}



// Privater Konstruktor

Line::Line(const char c, const Str  s1, const Str  s2, const Str  s3,
                         const long i1, const long i2, const long i3)
:   type (c),
    text1(s1),
    text2(s2),
    text3(s3),
    nr1  (i1),
    nr2  (i2),
    nr3  (i3)
{
}





// Mindestens ein Leerzeichen, anschliessend auffuellen, damit es schoen ist
void pad(std::ostringstream& o, const int i)
{
    do {
        o << (char) ' ';
    } while ((int) o.str().size() < i);
}



// Anzahl Ziffern in der Zahl, inklusive Minuszeichen
int digits(long nr)
{
    int ret = (nr <= 0);
    while (nr) {
        nr /= 10;
        ++ret;
    }
    return ret;
}



std::ostream& operator << (std::ostream& original_stream, const Line& ld)
{
    std::ostringstream o;
    o << ld.type;

    switch (ld.type) {
    case '>':
        o << ld.text1;
    
    case '$':
        o << ld.text1 << " " << ld.text2;
        break;

    case '#':
        o << ld.text1 << " " << ld.nr1;
        break;

    default:
        break;
    }
    return original_stream << o.str() << std::endl;
}



bool fill_vector_from_file(std::vector <Line>& v, const std::string& filename)
{
    if (!exists(filename.c_str())) return false;
    std::ifstream file(filename.c_str());
    const bool b = fill_vector_from_stream(v, file);
    file.close();
    return b;
}



bool fill_vector_from_stream(std::vector <Line>& v, std::istream& in)
{
    if (!in.good()) return false;
    while (in.good()) {
        std::string s;
        // Gute Zeichen in den String lesen
        while (in.good()) {
            char c = in.get();
            if (c != '\n' && c != '\r') s += c;
            else break;
        }
        // Weitere Zeilenumbruchszeichen entfernen... L++ spielt dos2unix
        while (in.good()) {
            char c = in.peek();
            if (c == '\n' || c == '\r') in.get();
            else break;
        }
        // String uebergeben und naechste Zeile einlesen
        Line line(s);
        if (line.type != '\0') v.push_back(Line(s));
    }
    return true;
}

}
// Ende Namensraum IO
