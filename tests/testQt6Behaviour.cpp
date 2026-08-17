/* Vérifications des comportements que le passage à Qt 6 a changés, et que rien d'autre ne garde.
 *
 * Elles n'ont pas la même nature que celles de testParsing.cpp : celles-là rejouent l'analyse de
 * vraies pages, celles-ci gardent trois endroits où Qt 6 fait silencieusement autre chose que Qt 5.
 * Aucune n'aurait échoué avant le portage ; elles servent à ce qu'on ne défasse pas les correctifs
 * sans s'en apercevoir, ce qui est précisément le risque quand une régression ne se voit ni à la
 * compilation ni au premier essai. */

#include <QFile>
#include <QRegularExpression>
#include <QSettings>
#include <QStringConverter>
#include <QStringList>
#include <QTemporaryDir>
#include <QTextStream>

#include "testTool.hpp"
#include "configDependentVar.hpp"

namespace
{
    /* La régression la plus probable du portage. Le \w de QRegExp reconnaissait les lettres Unicode,
     * celui de PCRE se limite à l'ASCII sans UseUnicodePropertiesOption : sans elle, et sur le
     * dictionnaire français que le programme livre, « café » devient « caf » et « é ».
     *
     * Le motif et l'option sont pris dans configDependentVar, donc c'est bien ce que le correcteur
     * emploie qui est vérifié ici, et pas une copie. */
    void testWordBoundary()
    {
        testTool::startGroup("Frontière de mot Unicode (correcteur)");

        const QRegularExpression expForSeparators(configDependentVar::expForWordSeparatorPattern + "+",
                                                  configDependentVar::expForWordSeparatorOptions);

        const QStringList wordsOfAccentedSentence = QString("café très tôt").split(expForSeparators);
        testTool::checkEquals("trois mots accentués restent trois mots", wordsOfAccentedSentence.size(), 3);
        testTool::checkEquals("« café » n'est pas coupé", wordsOfAccentedSentence.value(0), QString("café"));
        testTool::checkEquals("« très » n'est pas coupé", wordsOfAccentedSentence.value(1), QString("très"));
        testTool::checkEquals("« tôt » n'est pas coupé", wordsOfAccentedSentence.value(2), QString("tôt"));

        /* L'apostrophe et le tiret font partie du mot, c'est tout l'intérêt de la classe. */
        const QStringList wordsOfElidedSentence = QString("aujourd'hui c'est arc-en-ciel").split(expForSeparators);
        testTool::checkEquals("l'apostrophe et le tiret ne coupent pas", wordsOfElidedSentence.size(), 3);
        testTool::checkEquals("« aujourd'hui » reste entier", wordsOfElidedSentence.value(0), QString("aujourd'hui"));
        testTool::checkEquals("« arc-en-ciel » reste entier", wordsOfElidedSentence.value(2), QString("arc-en-ciel"));

        /* Le témoin : sans l'option, le même motif se comporte comme le \w ASCII de PCRE et découpe
         * « café ». Si cette vérification-ci se mettait à échouer, c'est que l'option est devenue le
         * défaut et que la précédente ne garde plus rien. */
        const QRegularExpression expWithoutOption(configDependentVar::expForWordSeparatorPattern + "+");
        testTool::checkEquals("sans l'option, « café » se couperait bien en deux",
                              QString("café").split(expWithoutOption).size(), 2);

        /* La frontière \b suit la définition de \w, donc l'option la concerne aussi : c'est elle qui
         * sert à souligner le mot fautif au bon endroit. */
        const QRegularExpression expForWholeWord(R"rgx(\bcafé\b)rgx",
                                                 configDependentVar::expForWordSeparatorOptions);
        testTool::checkTrue("\\b encadre un mot accentué", QString("un café serré").contains(expForWholeWord));
        testTool::checkTrue("\\b ne déclenche pas au milieu d'un mot",
                            QString("cafétéria").contains(expForWholeWord) == false);
    }

    /* Qt 6 écrit l'IniFormat en UTF-8, là où Qt 5 échappait les caractères non-latin1 en \xNN. Le
     * fichier porte les cookies de connexion, les listes de pseudos et les couleurs : un
     * aller-retour raté ne perd pas du confort, il déconnecte les comptes. On vérifie donc les deux
     * sens, dont la relecture d'un fichier écrit à l'ancienne. */
    void testSettingsRoundTrip()
    {
        testTool::startGroup("Aller-retour QSettings en IniFormat");

        QTemporaryDir dirForSettings;

        if(dirForSettings.isValid() == false)
        {
            testTool::reportFailure("dossier temporaire créé", "un dossier", "échec");
            return;
        }

        const QString pathOfConfig = dirForSettings.filePath("config.ini");
        const QString pseudoWithAccents = QString("Pseudo_éàü");
        const QStringList listOfPseudo = QStringList() << "PremierPseudo" << pseudoWithAccents;

        {
            QSettings settingToWrite(pathOfConfig, QSettings::IniFormat);
            settingToWrite.setValue("pseudoOfUser", pseudoWithAccents);
            settingToWrite.setValue("listOfPseudo", listOfPseudo);
            settingToWrite.setValue("cookieValue", QByteArray("id=abc123; path=/"));
            settingToWrite.sync();
        }

        {
            QSettings settingToRead(pathOfConfig, QSettings::IniFormat);
            testTool::checkEquals("un pseudo accentué survit à l'aller-retour",
                                  settingToRead.value("pseudoOfUser").toString(), pseudoWithAccents);
            testTool::checkEquals("une liste de pseudos survit entière",
                                  settingToRead.value("listOfPseudo").toStringList().size(), 2);
            testTool::checkEquals("le second élément de la liste garde ses accents",
                                  settingToRead.value("listOfPseudo").toStringList().value(1), pseudoWithAccents);
            testTool::checkEquals("un cookie survit à l'aller-retour",
                                  QString::fromUtf8(settingToRead.value("cookieValue").toByteArray()),
                                  QString("id=abc123; path=/"));
        }

        /* Un config.ini écrit par une version Qt 5, avec ses échappements : c'est ce que trouvera la
         * première exécution après mise à jour, et le seul cas où une régression coûterait des
         * comptes déconnectés. */
        const QString pathOfOldConfig = dirForSettings.filePath("config-qt5.ini");
        QFile fileForOldConfig(pathOfOldConfig);

        if(fileForOldConfig.open(QIODevice::WriteOnly | QIODevice::Text) == true)
        {
            QTextStream streamForOldConfig(&fileForOldConfig);
            streamForOldConfig.setEncoding(QStringConverter::Latin1);
            streamForOldConfig << "[General]\n" << "pseudoOfUser=Pseudo_\\xe9\\xe0\\xfc\n";
            fileForOldConfig.close();

            QSettings settingFromQt5(pathOfOldConfig, QSettings::IniFormat);
            testTool::checkEquals("un config.ini échappé à la Qt 5 se relit sous Qt 6",
                                  settingFromQt5.value("pseudoOfUser").toString(), pseudoWithAccents);
        }
        else
        {
            testTool::reportFailure("config.ini à l'ancienne écrit", "un fichier", "échec d'ouverture");
        }
    }

    /* Le dictionnaire utilisateur passe par QStringEncoder et QStringDecoder depuis que QTextCodec a
     * disparu. Deux choses à garder : que l'aller-retour rende bien le mot d'origine, et que les
     * gardes isValid() aient toujours quelque chose à garder — QStringConverter ne connaît qu'une
     * poignée d'encodages là où QTextCodec savait tout. */
    void testDictionaryEncoding()
    {
        testTool::startGroup("Encodage du dictionnaire utilisateur");

        const QString wordWithAccents = QString("déjà-vu");

        QStringEncoder encoderForUtf8(QStringConverter::Utf8);
        QStringDecoder decoderForUtf8(QStringConverter::Utf8);

        testTool::checkTrue("l'encodeur UTF-8 est valide", encoderForUtf8.isValid());
        testTool::checkTrue("le décodeur UTF-8 est valide", decoderForUtf8.isValid());

        const QByteArray encodedWord = encoderForUtf8(wordWithAccents);
        testTool::checkEquals("un mot accentué fait l'aller-retour",
                              QString(decoderForUtf8(encodedWord)), wordWithAccents);
        testTool::checkEquals("il est bien encodé en UTF-8 et non en latin-1",
                              encodedWord.size(), qsizetype(9));

        /* Ce que le dictionnaire livré déclare dans son fr.aff. */
        testTool::checkTrue("l'encodage annoncé par les dictionnaires livrés est reconnu",
                            QStringConverter::encodingForName("UTF-8").has_value());

        /* MIGRATION-QT6.md annonçait ici une perte : QStringConverter ne connaîtrait que l'UTF-8,
         * l'UTF-16, l'UTF-32, le latin-1 et l'encodage du système, si bien qu'un dictionnaire en
         * ISO-8859-15 déposé à la main cesserait d'être lu. C'est faux pour les binaires officiels
         * de Qt, qui embarquent ICU : l'enum Encoding ne déclare bien que cette poignée de valeurs,
         * mais le constructeur par nom passe par ICU et accepte plus de deux cents encodages,
         * ISO-8859-15 compris. Ces deux vérifications gardent ce constat, qui dépend de la présence
         * d'ICU dans le Qt utilisé et mérite donc d'échouer bruyamment s'il cesse d'être vrai. */
        testTool::checkTrue("l'ISO-8859-15 est bien reconnu, contrairement à ce qui était annoncé",
                            QStringEncoder("ISO-8859-15").isValid());
        testTool::checkTrue("le latin-1 aussi", QStringEncoder("ISO-8859-1").isValid());

        /* Ce que les gardes isValid() protègent vraiment : un nom que Qt ne reconnaît pas donne un
         * encodeur invalide plutôt qu'une exception ou un encodeur muet, et c'est ce cas-là que le
         * correcteur doit détecter au lieu de produire du charabia. */
        testTool::checkTrue("un nom d'encodage inconnu rend un encodeur invalide",
                            QStringEncoder("ceci-n-est-pas-un-encodage").isValid() == false);
    }
}

void runQt6BehaviourTests()
{
    testWordBoundary();
    testSettingsRoundTrip();
    testDictionaryEncoding();
}
