#include "mscfile.h"
#include "exceptions.h"
#include "mscparservisitor.h"

#include "parser/MscLexer.h"
#include "parser/MscParser.h"
#include "parser/MscBaseVisitor.h"

#include <antlr4-runtime.h>

#include <QFileInfo>
#include <QObject>

#include <fstream>

/*!
  \class MscFile
  \inmodule MscLibrary

  The class to load MSC files
*/

using namespace antlr4;

namespace msc {
/*!
  \brief MscFile::MscFile
*/
MscFile::MscFile()
{
}

/*!
  \fn MscFile::parseFile(const QString &filename)

  Loads the file \a filename
*/
void MscFile::parseFile(const QString &filename)
{
    if (!QFileInfo::exists(filename)) {
        throw FileNotFoundException();
    }

    std::ifstream stream;
    stream.open(filename.toStdString());
    if (!stream) {
        throw IOException(QObject::tr("Error opening the file"));
    }

    ANTLRInputStream input(stream);
    parse(input);
}

void MscFile::parseText(const QString &text)
{
    ANTLRInputStream input(text.toStdString());
    parse(input);
}

const MscModel &MscFile::model() const
{
    return m_model;
}

void MscFile::parse(ANTLRInputStream &input)
{
    m_model.clear();

    MscLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();

    MscParser parser(&tokens);

    MscParserVisitor visitor;
    visitor.setModel(&m_model);
    visitor.visit(parser.file());

    if (lexer.getNumberOfSyntaxErrors() > 0) {
        throw ParserException(QObject::tr("Syntax error"));
    }
    if (parser.getNumberOfSyntaxErrors() > 0) {
        throw ParserException(QObject::tr("Syntax error"));
    }
}

} // namespace msc
