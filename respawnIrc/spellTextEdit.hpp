#ifndef SPELLTEXTEDIT_HPP
#define SPELLTEXTEDIT_HPP

#include <QWidget>
#include <QTextEdit>
#include <QString>
#include <QVector>
#include <QAction>
#include <QStringList>
#include <QPoint>
#include <QContextMenuEvent>
#include <QStringConverter>
#include "hunspell/hunspell.hxx"

class spellTextEditClass : public QTextEdit
{
    Q_OBJECT
public:
    explicit spellTextEditClass(QWidget* parent = nullptr);
    ~spellTextEditClass();
    void doStuffBeforeQuit();
    void enableSpellChecking(bool newVal);
    bool setDic(const QString newSpellDic);
private:
    void searchWordBoundaryPosition(QString textBlock, int checkPos, int& beginPos, int& endPos) const;
    QStringList getWordPropositions(const QString word) const;
    QString getWordUnderCursor(QPoint cursorPos) const;
    void contextMenuEvent(QContextMenuEvent* event) override;
    bool checkWord(QString word) const;
private slots:
    void correctWord();
    void addWordToUserDic();
    void ignoreWord();
signals:
    void addWord(QString word);
private:
    QVector<QAction*> wordPropositionsActions;
    QString spellDic;
    Hunspell* spellChecker = nullptr;
    /* Remplacent le QTextCodec* que Qt 6 a supprimé, qui faisait les deux sens à lui seul. Il en
     * faut deux ici : le code encode presque partout, du Qt vers ce qu'attend Hunspell, et ne décode
     * qu'une fois, pour relire les suggestions rendues par getWordPropositions.
     *
     * Ils sont mutable parce que l'operator() de ces classes n'est pas const — elles gardent l'état
     * des encodages qui en ont un — alors que checkWord et getWordPropositions le sont et n'ont
     * aucune raison de cesser de l'être : encoder un mot pour le soumettre à Hunspell ne change rien
     * à l'objet du point de vue de l'appelant. */
    mutable QStringEncoder encoderUsed;
    mutable QStringDecoder decoderUsed;
    QStringList addedWords;
    QPoint lastPos;
    bool spellCheckingIsEnabled = false;
};

#endif
