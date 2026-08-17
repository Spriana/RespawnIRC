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
}

#endif
