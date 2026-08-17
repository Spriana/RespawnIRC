#include <QFileInfo>
#include <QByteArray>
#include <QColor>
#include <QStringList>
#include <QRegularExpression>

#include "highlighter.hpp"
#include "styleTool.hpp"
#include "pathTool.hpp"

namespace
{
    /* UseUnicodePropertiesOption est indispensable et pas décorative : le \w de QRegExp reconnaissait
     * les lettres Unicode, celui de PCRE se limite à l'ASCII tant qu'on ne la passe pas. Sans elle,
     * et sur le dictionnaire français que le programme livre, « café » se découperait en « caf » et
     * « é » — deux mots que Hunspell refuserait, donc tout le texte souligné en rouge. C'est la
     * régression la plus probable de tout le portage, et elle est parfaitement silencieuse.
     *
     * Elle vaut aussi pour \b, que PCRE définit à partir de \w. */
    const QRegularExpression expForWordSeparators(R"rgx([^\w'-]+)rgx",
                                                  QRegularExpression::UseUnicodePropertiesOption);
}

highlighterClass::highlighterClass(QTextDocument* parent) : QSyntaxHighlighter(parent)
{
    spellCheckFormat.setUnderlineColor(QColor(styleTool::getColorInfo().underlineColor));
    spellCheckFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    setDic("");
}

highlighterClass::~highlighterClass()
{
    if(spellChecker != nullptr)
    {
        delete spellChecker;
    }
}

void highlighterClass::enableSpellChecking(const bool state)
{
    bool old = spellCheckingIsEnabled;

    spellCheckingIsEnabled = state;

    if(old != spellCheckingIsEnabled)
    {
        rehighlight();
    }
}

bool highlighterClass::setDic(const QString newSpellDic)
{
    spellDic = newSpellDic;

    if(spellChecker != nullptr)
    {
        delete spellChecker;
    }

    QFileInfo fileInfoForDic(pathTool::pathForReading("resources/" + spellDic + ".dic"));
    if(fileInfoForDic.exists() == false || fileInfoForDic.isReadable() == false)
    {
        spellChecker = nullptr;
        encoderUsed = QStringEncoder(QStringConverter::Utf8);
    }
    else
    {
        QFileInfo fileInfoForUserDic(pathTool::pathForReading("user_" + spellDic + ".dic"));
        spellChecker = new Hunspell(pathTool::pathForReading("resources/" + spellDic + ".aff").toStdString().c_str(),
                                    pathTool::pathForReading("resources/" + spellDic + ".dic").toStdString().c_str());

        if(fileInfoForUserDic.exists() == true && fileInfoForUserDic.isReadable() == true)
        {
            spellChecker->add_dic(fileInfoForUserDic.filePath().toLatin1());
        }
        /* L'encodeur est invalide si le dictionnaire en annonce un que QStringConverter ne connaît
         * pas, et c'est ce que testent les isValid() plus bas : Qt 6 ne sait faire que l'UTF-8,
         * l'UTF-16, l'UTF-32, le latin-1 et celui du système, là où QTextCodec savait tout. Les
         * dictionnaires livrés déclarent SET UTF-8, mais un dictionnaire déposé à la main dans
         * resources/ en ISO-8859-15 tomberait ici — d'où des gardes plutôt qu'une confiance. */
        encoderUsed = QStringEncoder(spellChecker->get_dic_encoding());
    }

    rehighlight();

    if(spellChecker == nullptr)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void highlighterClass::styleChanged()
{
    spellCheckFormat.setUnderlineColor(QColor(styleTool::getColorInfo().underlineColor));
    rehighlight();
}

void highlighterClass::addWordToDic(QString word)
{
    if(spellChecker != nullptr && encoderUsed.isValid() == true)
    {
        spellChecker->add(QByteArray(encoderUsed(word)).toStdString());
        rehighlight();
    }
}

void highlighterClass::highlightBlock(const QString& text)
{
    spellCheck(text);
}

void highlighterClass::spellCheck(const QString& text)
{
    if(spellChecker != nullptr && encoderUsed.isValid() == true && spellCheckingIsEnabled == true)
    {
        QString simplifiedText = text.simplified();
        if(simplifiedText.isEmpty() == false)
        {
            QStringList checkList = simplifiedText.split(expForWordSeparators);
            for(QString thisString : checkList)
            {
                while(thisString.startsWith('\'') == true || thisString.startsWith('-') == true)
                {
                    thisString.remove(0, 1);
                }
                while(thisString.endsWith('\'') == true || thisString.endsWith('-') == true)
                {
                    thisString.remove(thisString.size() - 1, 1);
                }

                if(thisString.size() > 1)
                {
                    if(checkWord(thisString) == false)
                    {
                        /* Le mot est échappé : il vient du texte tapé par l'utilisateur, et les
                         * caractères que le correcteur laisse passer — l'apostrophe et le tiret —
                         * n'ont rien de spécial, mais rien ne garantit qu'il n'y en ait jamais
                         * d'autres. Sans escape, « c-- » ferait un motif invalide qui ne
                         * correspondrait à rien, silencieusement. */
                        const QRegularExpression expForThisWord(R"rgx(\b)rgx" + QRegularExpression::escape(thisString) + R"rgx(\b)rgx",
                                                                QRegularExpression::UseUnicodePropertiesOption);
                        qsizetype wordCount = text.count(expForThisWord);
                        qsizetype index = -1;

                        for(qsizetype j = 0; j < wordCount; ++j)
                        {
                            index = text.indexOf(expForThisWord, index + 1);
                            if(index >= 0)
                            {
                                /* setFormat est restée en int sous Qt 6, d'où les casts : ils sont
                                 * sans risque, un bloc de QTextDocument ne faisant pas deux
                                 * milliards de caractères. */
                                setFormat(static_cast<int>(index), static_cast<int>(thisString.size()), spellCheckFormat);
                            }
                        }
                    }
                }
            }
        }
    }
}

bool highlighterClass::checkWord(QString word)
{
    if(spellChecker != nullptr && encoderUsed.isValid() == true)
    {
        return spellChecker->spell(QByteArray(encoderUsed(word)).toStdString());
    }
    else
    {
        return false;
    }
}
