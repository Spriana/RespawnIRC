#ifndef CONFIGDEPENDENTVAR_HPP
#define CONFIGDEPENDENTVAR_HPP

#include <QtGlobal>
#include <QRegularExpression>

namespace configDependentVar
{
    /* Les options que toutes les expressions rationnelles du programme prennent pour base, passées
     * telles quelles à une cinquantaine de constructeurs.
     *
     * Elles valaient OptimizeOnFirstUsageOption, que Qt 6 a supprimée — c'est la seule API retirée
     * du portage qui ne vienne pas de QRegExp, et elle se cachait dans du code déjà écrit en
     * QRegularExpression. Il n'y a rien à mettre à la place : Qt 5.12 l'avait déjà dépréciée parce
     * que le moteur décide désormais seul quand optimiser, à partir d'un compteur d'utilisations, et
     * QRegularExpression::optimize() reste là pour le forcer. La valeur neutre ci-dessous rend donc
     * exactement le comportement d'avant.
     *
     * La variable survit plutôt que de disparaître des cinquante appels : c'est l'endroit tout
     * trouvé si une option devait un jour valoir pour tout le programme. */
    static const QRegularExpression::PatternOption regexpBaseOptions = QRegularExpression::NoPatternOption;

    /* La classe de caractères qui sépare deux mots pour le correcteur orthographique, et l'option
     * sans laquelle elle ne veut pas dire ce qu'on croit.
     *
     * Les deux vivent ici, et pas dans highlighter.cpp et spellTextEdit.cpp qui s'en servent, pour
     * une raison précise : c'est ce qui permet aux tests de vérifier le motif que le programme
     * emploie vraiment, plutôt qu'une copie qui pourrait diverger sans que rien ne le dise. Le \w de
     * QRegExp reconnaissait les lettres Unicode, celui de PCRE se limite à l'ASCII sans
     * UseUnicodePropertiesOption : sur le dictionnaire français livré, l'oublier découperait
     * « café » en deux mots que Hunspell refuserait. C'est la régression la plus probable du portage
     * vers Qt 6, et la plus silencieuse. */
    static const QString expForWordSeparatorPattern = QStringLiteral(R"rgx([^\w'-])rgx");
    static const QRegularExpression::PatternOptions expForWordSeparatorOptions = QRegularExpression::UseUnicodePropertiesOption;
}

#endif
