#ifndef HIGHLIGHTER_HPP
#define HIGHLIGHTER_HPP

#include <QSyntaxHighlighter>
#include <QString>
#include <QStringConverter>
#include <QTextCharFormat>
#include <QTextDocument>
#include "hunspell/hunspell.hxx"

class highlighterClass : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit highlighterClass(QTextDocument* parent = nullptr);
    ~highlighterClass();
    void enableSpellChecking(const bool state);
    bool setDic(const QString newSpellDic);
    void styleChanged();
public slots:
    void addWordToDic(QString word);
private:
    void highlightBlock(const QString& text) override;
    void spellCheck(const QString& text);
    bool checkWord(QString word);
private:
    QString spellDic;
    QString spellEncoding;
    Hunspell* spellChecker = nullptr;
    bool spellCheckingIsEnabled = false;
    QTextCharFormat spellCheckFormat;
    /* Remplace le QTextCodec* que Qt 6 a supprimé. Le code n'encode que dans un sens, du Qt vers ce
     * qu'attend Hunspell, un encodeur suffit donc. Par défaut il est invalide, ce qui tient
     * exactement le rôle de l'ancien pointeur nul, et isValid() celui du test contre nullptr. */
    QStringEncoder encoderUsed;
};

#endif
