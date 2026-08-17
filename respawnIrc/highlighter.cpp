#include <QFileInfo>
#include <QByteArray>
#include <QColor>
#include <QStringList>
#include <QRegularExpression>

#include "highlighter.hpp"
#include "styleTool.hpp"
#include "pathTool.hpp"
#include "configDependentVar.hpp"

namespace
{
    /* Le motif et l'option viennent de configDependentVar, où les tests vont les chercher : l'option
     * est ce qui empêche « café » de se découper en deux, et elle mérite d'être gardée par un test
     * plutôt que par un commentaire. Le + est ajouté ici, cette version-ci mangeant les suites de
     * séparateurs d'un coup pour découper une phrase entière. */
    const QRegularExpression expForWordSeparators(configDependentVar::expForWordSeparatorPattern + "+",
                                                  configDependentVar::expForWordSeparatorOptions);
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
        /* L'encodeur est invalide si le dictionnaire annonce un encodage que Qt ne reconnaît pas, et
         * c'est ce que testent les isValid() plus bas. La perte annoncée par MIGRATION-QT6.md n'a
         * pas lieu : l'enum Encoding ne déclare qu'une poignée de valeurs, mais le constructeur par
         * nom passe par ICU, que les binaires officiels de Qt embarquent, et accepte plus de deux
         * cents encodages — ISO-8859-15 compris. Les gardes restent utiles pour un nom réellement
         * inconnu, et tests/testQt6Behaviour.cpp garde ce constat. */
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
                                                                configDependentVar::expForWordSeparatorOptions);
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
