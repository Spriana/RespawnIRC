#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QUrl>
#include <QtGlobal>

#include "parsingTool.hpp"
#include "styleTool.hpp"
#include "shortcutTool.hpp"
#include "configDependentVar.hpp"

namespace
{
    const QRegularExpression expForNormalLink(R"rgx(http(s)?://[^\(\)\]\[ \\<>]*)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForAjaxTimestamp(R"rgx(<input type="hidden" name="ajax_timestamp_liste_messages" id="ajax_timestamp_liste_messages" value="([^"]*)" />)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForAjaxHash(R"rgx(<input type="hidden" name="ajax_hash_liste_messages" id="ajax_hash_liste_messages" value="([^"]*)" />)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForAjaxModTimestamp(R"rgx(<input type="hidden" name="ajax_timestamp_moderation_forum" id="ajax_timestamp_moderation_forum" value="([^"]*)" />)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForAjaxModHash(R"rgx(<input type="hidden" name="ajax_hash_moderation_forum" id="ajax_hash_moderation_forum" value="([^"]*)" />)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForMessageEdit(R"rgx(<textarea tabindex="3" class="area-editor" name="text_commentaire" id="text_commentaire" placeholder="[^"]*">([^<]*)</textarea>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForMessageQuote(R"rgx("txt":"(.*)"})rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForVersionName(R"rgx("tag_name"[^"]*:[^"]*"([^"]*)")rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForVersionChangelog(R"rgx("body"[^"]*:[^"]*"(.*)")rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForFormTopic(R"rgx((<form role="form" class="form-post-topic[^"]*" method="post" action="".*?>.*?</form>))rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForFormConnect(R"rgx((<form role="form" class="form-connect-jv" method="post" action="".*?>.*?</form>))rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForInput(R"rgx(<input ([^=]*)="([^"]*)" ([^=]*)="([^"]*)" ([^=]*)="([^"]*)"/>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForTopicLocked(R"rgx(<div class="message-lock-topic">)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForCaptcha(R"rgx(<img src="([^"]*)" alt=[^>]*>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForError(R"rgx(<div class="alert-row">([^<]*)</div>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForCurrentPage(R"rgx(<span class="page-active">([^<]*)</span>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForPageLink(R"rgx(<span><a href="([^"]*)" class="lien-jv">([^<]*)</a></span>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForNameOfTopic(R"rgx(<span id="bloc-title-forum">([^<]*)</span>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForNumberOfConnected(R"rgx(<span class="nb-connect-fofo">([^<]*)</span>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForMpJvc(R"rgx(<div class=".*?account-mp.*?">[^<]*<span[^c]*class="account-number-mp[^"]*".*?data-val="([^"]*)")rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForEntireMessage(R"rgx((<div class="bloc-message-forum[^"]*".*?)(<span id="post_[^"]*" class="bloc-message-forum-anchor">|<div class="bloc-outils-plus-modo bloc-outils-bottom">|<div class="bloc-pagi-default">))rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForEntireTopic(R"rgx(<li class="[^"]*" data-id="[^"]*">.*?<span class="topic-subject">.*?</li>)rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForTopicNameAndLink(R"rgx(<a class="lien-jv topic-title[^"]*" href="([^"]*" title="[^"]*)"[^>]*>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForTopicNumberMessage(R"rgx(<span class="topic-count">[^0-9]*([0-9]*))rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForTopicPseudoInfo(R"rgx(<span class="JvCare [^ ]* text-([^ ]*) topic-author)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForTopicType(R"rgx(<img src="/img/forums/topic-(.*?)\.png" alt="[^"]*" title="[^"]*" class="topic-img")rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForMessageID(R"rgx(<div class="bloc-message-forum[^"]*" data-id="([^"]*)">)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForAvatars(R"rgx(<img src="[^"]*" data-srcset="(http:)?//([^"]*)" class="user-avatar-msg")rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForPseudo(R"rgx(<span class="JvCare [^ ]* bloc-pseudo-msg text-([^"]*)" target="_blank">[^a-zA-Z0-9_\[\]-]*([a-zA-Z0-9_\[\]-]*)[^<]*</span>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForDate(R"rgx(<div class="bloc-date-msg">([^<]*<span class="JvCare [^ ]* lien-jv" target="_blank">)?[^a-zA-Z0-9]*([^ ]* [^ ]* [^ ]* [^ ]* ([0-9:]*)))rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForMessage(R"rgx(<div class="bloc-contenu"><div class="txt-msg  text-[^-]*-forum ">((.*?)(?=<div class="info-edition-msg">)|(.*?)(?=<div class="signature-msg)|(.*)))rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForEdit(R"rgx(<div class="info-edition-msg">Message édité le ([^ ]* [^ ]* [^ ]* [^ ]* [0-9:]*) par <span)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForSignature(R"rgx(<div class="signature-msg[^"]*">(.*?)</div>)rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForTopicLinkNumber(R"rgx((http://([^/]*)/forums/[^-]*-([^-]*)-([^-]*)-)([^-]*)(-[^-]*-[^-]*-[^-]*-[^\.]*\.htm))rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForForumName(R"rgx(<title>(.*?)- jeuxvideo\.com</title>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForJvfLink(R"rgx(http://jvforum\.fr/([^/]*)/([^-]*)-([^/]*))rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForSmiley(R"rgx(<img src="http(s)?://image\.jeuxvideo\.com/smileys_img/([^"]*)" alt="[^"]*" data-code="([^"]*)" title="[^"]*" [^>]*>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForStickers(R"rgx(<img class="img-stickers" src="(http://jv\.stkr\.fr/p[^/]*/([^"]*))"/>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForLongLink(R"rgx(<span class="JvCare [^"]*"[^i]*itle="([^"]*)">[^<]*<i></i><span>[^<]*</span>[^<]*</span>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForShortLink(R"rgx(<span class="JvCare [^"]*" rel="nofollow[^"]*" target="_blank">([^<]*)</span>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForJvcLink(R"rgx(<a href="([^"]*)"( )?( title="[^"]*")?>.*?</a>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForNoelshack(R"rgx(<a href="([^"]*)" target="_blank"><img class="img-shack" .*? src="http(s)?://([^"]*)" [^>]*></a>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForYoutubeVideo(R"rgx(<div class="player-contenu"><div class="[^"]*"><iframe .*? src="http(s)?://www\.youtube\.com/embed/([^"]*)"[^>]*></iframe></div></div>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForSpoilLine(R"rgx(<span class="bloc-spoil-jv en-ligne">.*?<span class="contenu-spoil">(.*?)</span></span>)rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForSpoilBlock(R"rgx(<span class="bloc-spoil-jv">.*?<span class="contenu-spoil">(.*?)</span></span>)rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForCodeBlock(R"rgx(<pre class="pre-jv"><code class="code-jv">([^<]*)</code></pre>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForCodeLine(R"rgx(<code class="code-jv">(.*?)</code>)rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForAllJVCare(R"rgx(<span class="JvCare [^"]*">([^<]*)</span>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForUnicodeInText(R"rgx(\\u([a-zA-Z0-9]{4}))rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForOverlyQuote(R"rgx(<(/)?blockquote>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForOverlySpoils(R"rgx((<span class="bloc-spoil-jv[^"]*">.*?<span class="contenu-spoil">|</span></span>))rgx", configDependentVar::regexpBaseOptions | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression expForUglyImage(R"rgx(issou|risit|jesus|picsart|chancla)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForAd(R"rgx(<ins[^>]*></ins>)rgx", configDependentVar::regexpBaseOptions);
    const QRegularExpression expForWebsite(R"rgx(http://([^/]*)/)rgx", configDependentVar::regexpBaseOptions);
    QString userAgentToUse = "RespatatouilleIRC";

    QString stringModificatorRemoveFirstAndLastP(QString baseMessage)
    {
        baseMessage = baseMessage.trimmed();
        while(baseMessage.startsWith("<p>") == true)
        {
            baseMessage.remove(0, 3);
            baseMessage = baseMessage.trimmed();
        }
        while(baseMessage.endsWith("</p>") == true)
        {
            baseMessage.remove(baseMessage.size() - 4, 4);
            baseMessage = baseMessage.trimmed();
        }
        return baseMessage;
    }

    QString stringModificatorMakeLinkIfPossible(QString baseMessage)
    {
        if((baseMessage.startsWith("http://") || baseMessage.startsWith("https://")) && baseMessage.contains(" ") == false)
        {
            baseMessage = "<a style=\"color: " + styleTool::getColorInfo().linkColor + ";\" href=\"" + baseMessage + "\">" + baseMessage + "</a>";
        }
        return baseMessage;
    }

    QString stringModificatorHideUglyImages(QString baseMessage)
    {
        QRegularExpressionMatch matcherForUglyImage = expForUglyImage.match(baseMessage);
        if(matcherForUglyImage.hasMatch() == true)
        {
            return "";
        }
        return baseMessage;
    }
}

void parsingTool::generateNewUserAgent() {
    QStringList allStringForGeneration;
    int lastMessageId = qrand() % 3;

    allStringForGeneration.append("RespawnIRC");
    allStringForGeneration.append("jeuxvideo.com");

    if(lastMessageId == 0)
    {
        allStringForGeneration.append("laissez moi fonctionner");
    }
    else if(lastMessageId == 1)
    {
        allStringForGeneration.append("je suis gentil");
    }
    else
    {
        allStringForGeneration.append("fonctionne optimal");
    }

    userAgentToUse.clear();

    while(allStringForGeneration.isEmpty() == false)
    {
        int currentString = qrand() % allStringForGeneration.size();
        int currentChar = qrand() % allStringForGeneration.at(currentString).size();

        userAgentToUse += allStringForGeneration.at(currentString).at(currentChar);
        allStringForGeneration[currentString].remove(currentChar, 1);

        if(allStringForGeneration.at(currentString).isEmpty() == true)
        {
            allStringForGeneration.removeAt(currentString);
        }
    }
}

bool parsingTool::checkIfTopicAreSame(const QString& firstTopic, const QString& secondTopic)
{
    QRegularExpressionMatch matcherForFirstTopic = expForTopicLinkNumber.match(firstTopic);
    QRegularExpressionMatch matcherForSecondTopic = expForTopicLinkNumber.match(secondTopic);

    if(matcherForFirstTopic.hasMatch() == true && matcherForSecondTopic.hasMatch() == true)
    {
        return matcherForFirstTopic.captured(2) == matcherForSecondTopic.captured(2) &&
               matcherForFirstTopic.captured(3) == matcherForSecondTopic.captured(3) &&
               matcherForFirstTopic.captured(4) == matcherForSecondTopic.captured(4);
    }
    else
    {
        return false;
    }
}

ajaxInfoStruct parsingTool::getAjaxInfo(const QString& source)
{
    ajaxInfoStruct newAjaxInfo;

    QString ajaxTimestamp = expForAjaxTimestamp.match(source).captured(1);
    QString ajaxHash = expForAjaxHash.match(source).captured(1);
    QString ajaxModTimestamp = expForAjaxModTimestamp.match(source).captured(1);
    QString ajaxModHash = expForAjaxModHash.match(source).captured(1);

    if(ajaxTimestamp.isEmpty() == false && ajaxHash.isEmpty() == false)
    {
        newAjaxInfo.list = "ajax_timestamp=" + ajaxTimestamp + "&ajax_hash=" + ajaxHash;
    }

    if(ajaxModTimestamp.isEmpty() == false && ajaxModHash.isEmpty() == false)
    {
        newAjaxInfo.mod = "ajax_timestamp=" + ajaxModTimestamp + "&ajax_hash=" + ajaxModHash;
    }

    return newAjaxInfo;
}

QString parsingTool::getMessageEdit(const QString& source)
{
    return specialCharToNormalChar(expForMessageEdit.match(source).captured(1));
}

QString parsingTool::getMessageQuote(const QString& source)
{
    QString message = specialCharToNormalChar(parsingAjaxMessages(expForMessageQuote.match(source).captured(1)));
    message.replace("\n", "\n>");

    return message;
}

QString parsingTool::getWebsite(const QString& topicLink)
{
    return expForWebsite.match(topicLink).captured(1);
}

QString parsingTool::getVersionName(const QString& source)
{
    return expForVersionName.match(source).captured(1);
}

QString parsingTool::getVersionChangelog(const QString& source)
{
    QString changelog = expForVersionChangelog.match(source).captured(1).replace("\\n", "<br />").replace("\\r", "").replace("\\\"", "\"").replace(" -", "--").replace("   --", "---").replace("\\\\", "\\");
    replaceWithCapNumber(changelog, expForNormalLink, 0, "<a style=\"color: " + styleTool::getColorInfo().linkColor + ";\" href=\"", "\">", 0, "</a>");
    changelog = changelog.trimmed();
    while(changelog.startsWith("<br />") == true)
    {
        changelog.remove(0, 6);
        changelog = changelog.trimmed();
    }
    while(changelog.endsWith("<br />") == true)
    {
        changelog.remove(changelog.size() - 6, 6);
        changelog = changelog.trimmed();
    }
    return changelog;
}

void parsingTool::getListOfHiddenInputFromThisForm(const QString& source, QString formName, QList<QPair<QString, QString>>& listOfInput)
{
    QString formSource;

    if(formName == "form-post-topic")
    {
        formSource = expForFormTopic.match(source).captured(1);
    }
    else if(formName == "form-connect-jv")
    {
        formSource = expForFormConnect.match(source).captured(1);
    }

    QRegularExpressionMatchIterator matchIteratorForInput = expForInput.globalMatch(formSource);
    while(matchIteratorForInput.hasNext())
    {
        QRegularExpressionMatch matchForInput = matchIteratorForInput.next();
        if(matchForInput.captured(1) == "type")
        {
            if(matchForInput.captured(3) == "name")
            {
                listOfInput.push_back(QPair<QString, QString>(matchForInput.captured(4), matchForInput.captured(6)));
            }
            else
            {
                listOfInput.push_back(QPair<QString, QString>(matchForInput.captured(6), matchForInput.captured(4)));
            }
        }
        else if(matchForInput.captured(3) == "type")
        {
            if(matchForInput.captured(1) == "name")
            {
                listOfInput.push_back(QPair<QString, QString>(matchForInput.captured(2), matchForInput.captured(6)));
            }
            else
            {
                listOfInput.push_back(QPair<QString, QString>(matchForInput.captured(6), matchForInput.captured(2)));
            }
        }
        else
        {
            if(matchForInput.captured(1) == "name")
            {
                listOfInput.push_back(QPair<QString, QString>(matchForInput.captured(2), matchForInput.captured(4)));
            }
            else
            {
                listOfInput.push_back(QPair<QString, QString>(matchForInput.captured(4), matchForInput.captured(2)));
            }
        }
    }
}

bool parsingTool::getTopicLocked(const QString& source)
{
    return expForTopicLocked.match(source).hasMatch();
}

QString parsingTool::getCaptchaLink(const QString& source)
{
    return expForCaptcha.match(source).captured(1);
}

QString parsingTool::getErrorMessage(const QString& source)
{
    QRegularExpressionMatch match = expForError.match(source);

    if(match.hasMatch() == true)
    {
        return expForError.match(source).captured(1);
    }
    else
    {
        return "Le message n'a pas été envoyé.";
    }
}

QString parsingTool::getNextPageOfTopic(const QString& source, const QString& website)
{
    int currentPage = 0;
    currentPage = expForCurrentPage.match(source).captured(1).toInt();

    QRegularExpressionMatchIterator matchIteratorForPgaeLink = expForPageLink.globalMatch(source);
    while(matchIteratorForPgaeLink.hasNext())
    {
        QRegularExpressionMatch matchForPageLink = matchIteratorForPgaeLink.next();
        if(matchForPageLink.captured(2).toInt() == currentPage + 1)
        {
            return "http://" + website + matchForPageLink.captured(1);
        }
    }

    return "";
}

QString parsingTool::getLastPageOfTopic(const QString& source, const QString& website)
{
    int currentPage = 0;
    QString lastPage;
    currentPage = expForCurrentPage.match(source).captured(1).toInt();

    QRegularExpressionMatchIterator matchIteratorForPgaeLink = expForPageLink.globalMatch(source);
    while(matchIteratorForPgaeLink.hasNext())
    {
        QRegularExpressionMatch matchForPageLink = matchIteratorForPgaeLink.next();
        if(matchForPageLink.captured(2).toInt() > currentPage)
        {
            currentPage = matchForPageLink.captured(2).toInt();
            lastPage = "http://" + website + matchForPageLink.captured(1);
        }
    }

    return lastPage;
}

QString parsingTool::getFirstPageOfTopic(const QString& topicLink)
{
    QRegularExpressionMatch matchForFirstPage = expForTopicLinkNumber.match(topicLink);

    if(matchForFirstPage.hasMatch() == true)
    {
        return matchForFirstPage.captured(1) + "1" + matchForFirstPage.captured(6);
    }
    else
    {
        return "";
    }
}

QString parsingTool::getBeforeLastPageOfTopic(const QString& topicLink)
{
    QRegularExpressionMatch matchForBeforeLastPage = expForTopicLinkNumber.match(topicLink);
    QString pageNumber = matchForBeforeLastPage.captured(5);

    if(pageNumber.isEmpty() == false && pageNumber != "1")
    {
        return matchForBeforeLastPage.captured(1) + QString::number(pageNumber.toInt() - 1) + matchForBeforeLastPage.captured(6);
    }
    else
    {
        return "";
    }
}

QString parsingTool::getNameOfTopic(const QString& source)
{
    return expForNameOfTopic.match(source).captured(1).replace("&amp;", "&").replace("&quot;", "\"").replace("&#039;", "\'").replace("&lt;", "<").replace("&gt;", ">");
}

QString parsingTool::getNumberOfConnected(const QString& source)
{
    return expForNumberOfConnected.match(source).captured(1);
}

int parsingTool::getNumberOfMp(const QString& source)
{
    QRegularExpressionMatch matchMpNumber = expForMpJvc.match(source);

    if(matchMpNumber.hasMatch() == true)
    {
        return matchMpNumber.captured(1).trimmed().toInt();
    }
    else
    {
        return 0;
    }
}

QList<messageStruct> parsingTool::getListOfEntireMessagesWithoutMessagePars(const QString& source)
{
    QList<QString> listOfEntireMessage;
    QList<messageStruct> listOfMessages;
    long lastIdOfMessage = 0;
    bool sortNeeded = false;

    listOfEntireMessage = getListOfThisCapNumber(source, expForEntireMessage, 1, false);

    for(const QString& thisMessage : listOfEntireMessage)
    {
        listOfMessages.push_back(messageStruct());
        listOfMessages.back().idOfMessage = expForMessageID.match(thisMessage).captured(1).toLong();
        listOfMessages.back().avatarLink = expForAvatars.match(thisMessage).captured(2);
        listOfMessages.back().pseudoInfo.pseudoName = expForPseudo.match(thisMessage).captured(2);
        listOfMessages.back().pseudoInfo.pseudoType = expForPseudo.match(thisMessage).captured(1);
        listOfMessages.back().date = expForDate.match(thisMessage).captured(3);
        listOfMessages.back().wholeDate = expForDate.match(thisMessage).captured(2);
        listOfMessages.back().message = expForMessage.match(thisMessage).captured(1);
        listOfMessages.back().lastTimeEdit = expForEdit.match(thisMessage).captured(1);
        listOfMessages.back().signature = expForSignature.match(thisMessage).captured(1);

        if(listOfMessages.back().pseudoInfo.pseudoName.isEmpty() == true)
        {
            listOfMessages.back().pseudoInfo.pseudoName = "Pseudo supprimé";
        }

        if(lastIdOfMessage > listOfMessages.back().idOfMessage)
        {
            sortNeeded = true;
        }
        lastIdOfMessage = listOfMessages.back().idOfMessage;
    }

    if(sortNeeded == true)
    {
        qSort(listOfMessages);
    }

    return listOfMessages;
}

QList<topicStruct> parsingTool::getListOfTopic(const QString& source, const QString& website)
{
    QList<topicStruct> listOfTopic;
    QList<QString> listOfEntireTopic;

    listOfEntireTopic = getListOfThisCapNumber(source, expForEntireTopic, 0, false);

    for(const QString& thisTopic : listOfEntireTopic)
    {
        QString topicInfo = expForTopicNameAndLink.match(thisTopic).captured(1);
        QString link = "http://" + website + topicInfo.left(topicInfo.indexOf("\""));
        QString name = topicInfo.right(topicInfo.size() - topicInfo.indexOf("title=\"") - 7);
        name.replace("&amp;", "&").replace("&quot;", "\"").replace("&#039;", "\'").replace("&lt;", "<").replace("&gt;", ">");
        listOfTopic.append(topicStruct());
        listOfTopic.back().name = name;
        listOfTopic.back().link = link;
        listOfTopic.back().numberOfMessage = expForTopicNumberMessage.match(thisTopic).captured(1);
        listOfTopic.back().pseudoInfo.pseudoType = expForTopicPseudoInfo.match(thisTopic).captured(1);
        listOfTopic.back().topicType = expForTopicType.match(thisTopic).captured(1);
    }

    return listOfTopic;
}

QString parsingTool::getForumOfTopic(const QString& topicLink)
{
    QRegularExpressionMatch infosMatcher = expForTopicLinkNumber.match(topicLink);

    if(infosMatcher.hasMatch() == true)
    {
        return "http://" + infosMatcher.captured(2) + "/forums/0-" + infosMatcher.captured(3) + "-0-1-0-1-0-respawn-irc.htm";
    }
    else
    {
        return "";
    }
}

QString parsingTool::getForumName(const QString& source)
{
    QString forumName = expForForumName.match(source).captured(1);

    while(forumName.endsWith(" "))
    {
        forumName.remove(forumName.size() - 1, 1);
    }

    return forumName;
}

QString parsingTool::jvfLinkToJvcLink(const QString& jvfTopicLink)
{
    QRegularExpressionMatch matchForJvfLink = expForJvfLink.match(jvfTopicLink);
    QString forumNumber = matchForJvfLink.captured(1);
    QString topicNumber = matchForJvfLink.captured(2);
    QString nameOfTopic = matchForJvfLink.captured(3);

    if(forumNumber.isEmpty() == false && topicNumber.isEmpty() == false)
    {
        QString tempo = "42";

        if(topicNumber.startsWith("0") == true)
        {
            tempo = "1";
            topicNumber.remove(0, 1);
        }

        return "http://www.jeuxvideo.com/forums/" + tempo + "-" + forumNumber + "-" + topicNumber + "-1-0-1-0-" + nameOfTopic + ".htm";
    }
    else
    {
        return "";
    }
}

QString parsingTool::parsingMessages(QString thisMessage, infoForMessageParsingStruct infoForParsing, bool reallyDownloadStickers)
{
    QString extraTableStyle;

    if(infoForParsing.betterQuote == true)
    {
        extraTableStyle += "background: " + styleTool::getColorInfo().quoteBackgroundColor + ";color: " + styleTool::getColorInfo().quoteTextColor + ";";
    }

    thisMessage.remove(expForAd);

    if(infoForParsing.betterCodeTag == true)
    {
        replaceWithCapNumber(thisMessage, expForCodeBlock, 1, "<table border=\"0\" cellspacing=\"0\" cellpadding=\"5\" style=\"margin-top: 5px;margin-bottom: 5px;background-color: " + styleTool::getColorInfo().codeTagBackgroundColor + ";\"><tr><td><code style=\"white-space: pre-wrap\">", "</code></td></tr></table>", -1, "", [](QString baseString)->QString{return baseString.replace("\n", "<br />");});
        replaceWithCapNumber(thisMessage, expForCodeLine, 1, "<span style=\"background-color: " + styleTool::getColorInfo().codeTagBackgroundColor + ";\"> <code style=\"white-space: pre-wrap\">", "</code> </span>", -1, "");
    }
    else
    {
        replaceWithCapNumber(thisMessage, expForCodeBlock, 1, "<p><code style=\"white-space: pre-wrap\">", "</code></p>", -1, "", [](QString baseString)->QString{return baseString.replace("\n", "<br />");});
        replaceWithCapNumber(thisMessage, expForCodeLine, 1, " <code style=\"white-space: pre-wrap\">", "</code> ", -1, "");
    }

    thisMessage.replace("\n", "");
    thisMessage.replace("\r", "");

    shortcutTool::transformMessage(&thisMessage, "noLangageSticker");
    if(infoForParsing.stickerToSmiley == true)
    {
        shortcutTool::transformMessage(&thisMessage, "stickerToSmiley");
    }

    if(infoForParsing.showStickers == false)
    {
        replaceWithCapNumber(thisMessage, expForStickers, 1, "<a style=\"color: " + styleTool::getColorInfo().linkColor + ";\" href=\"", "\">", 1, "</a>");
    }
    else
    {
        if(infoForParsing.listOfStickersUsed != nullptr && reallyDownloadStickers == true)
        {
            infoForParsing.listOfStickersUsed->append(getListOfThisCapNumber(thisMessage, expForStickers, 2, false));
        }

        replaceWithCapNumber(thisMessage, expForStickers, 2, "<img width=" + QString::number(infoForParsing.stickersSize) + " height=" + QString::number(infoForParsing.stickersSize) + " src=\"resources/stickers/", ".png\" />");
    }

    replaceWithCapNumber(thisMessage, expForSmiley, 2, "<img src=\"resources/smileys/", "\" />");
    replaceWithCapNumber(thisMessage, expForYoutubeVideo, 2, "<a style=\"color: " + styleTool::getColorInfo().linkColor + ";\" href=\"http://youtu.be/", "\">http://youtu.be/", 2, "</a>");
    replaceWithCapNumber(thisMessage, expForJvcLink, 1, "<a style=\"color: " + styleTool::getColorInfo().linkColor + ";\" href=\"", "\">", 1, "</a>");
    replaceWithCapNumber(thisMessage, expForShortLink, 1, "<a style=\"color: " + styleTool::getColorInfo().linkColor + ";\" href=\"", "\">", 1, "</a>");
    replaceWithCapNumber(thisMessage, expForLongLink, 1, "<a style=\"color: " + styleTool::getColorInfo().linkColor + ";\" href=\"", "\">", 1, "</a>");

    if(infoForParsing.hideUglyImages == true)
    {
        replaceWithCapNumber(thisMessage, expForNoelshack, 0, "", "", -1, "", std::bind(stringModificatorHideUglyImages, std::placeholders::_1));
    }

    if(infoForParsing.listOfNoelshackImageUsed != nullptr)
    {
        infoForParsing.listOfNoelshackImageUsed->append(getListOfThisCapNumber(thisMessage, expForNoelshack, 3, false));

        for(QString& thisNoelshackImage : *infoForParsing.listOfNoelshackImageUsed)
        {
            if(thisNoelshackImage.startsWith("http://") == false)
            {
                thisNoelshackImage = "http://" + thisNoelshackImage;
            }
        }

        replaceWithCapNumber(thisMessage, expForNoelshack, 1, "<a href=\"", "\"><img width=" + QString::number(infoForParsing.noelshackImageWidth) + " height=" + QString::number(infoForParsing.noelshackImageHeight) + " src=\"img/", 3, "\" /></a>");
    }
    else
    {
        replaceWithCapNumber(thisMessage, expForNoelshack, 1, "<a style=\"color: " + styleTool::getColorInfo().linkColor + ";\" href=\"", "\">", 1, "</a>");
    }

    removeAllOverlySpoils(thisMessage);
    replaceWithCapNumber(thisMessage, expForSpoilLine, 1, "<span style=\"color: " + styleTool::getColorInfo().spoilColor + "; background-color: " + styleTool::getColorInfo().spoilColor + ";\">", "</span>", -1, "", std::bind(stringModificatorRemoveFirstAndLastP, std::placeholders::_1));
    replaceWithCapNumber(thisMessage, expForSpoilBlock, 1, "<p><span style=\"color: " + styleTool::getColorInfo().spoilColor + "; background-color: " + styleTool::getColorInfo().spoilColor + ";\">", "</span></p>", -1, "", std::bind(stringModificatorRemoveFirstAndLastP, std::placeholders::_1));
    replaceWithCapNumber(thisMessage, expForAllJVCare, 1, "", "", -1, "", std::bind(stringModificatorMakeLinkIfPossible, std::placeholders::_1));

    thisMessage.replace("<blockquote class=\"blockquote-jv\">", "<blockquote>");
    removeAllOverlyQuote(thisMessage, infoForParsing.nbMaxQuote);

    thisMessage.replace("<ul class=\"liste-default-jv\">", "<br /><br /><ul>");
    thisMessage.replace("<ol class=\"liste-default-jv\">", "<br /><br /><ol>");
    thisMessage.replace("</ul>", "<ul><br /><br />");
    thisMessage.replace("</ol>", "<ol><br /><br />");

    thisMessage.replace(QRegularExpression(R"rgx((<br /> *){0,2}</p> *<p>( *<br />){0,2})rgx"), "<br /><br />");
    thisMessage.replace(QRegularExpression(R"rgx(<br /> *<(/)?p> *<br />)rgx"), "<br /><br />");
    thisMessage.replace(QRegularExpression(R"rgx((<br /> *){1,2}<(/)?p>)rgx"), "<br /><br />");
    thisMessage.replace(QRegularExpression(R"rgx(<(/)?p>(<br /> *){1,2})rgx"), "<br /><br />");
    thisMessage.replace(QRegularExpression(R"rgx(<(/)?p>)rgx"), "<br /><br />");
    thisMessage.replace(QRegularExpression(R"rgx((<br /> *)*(<(/)?blockquote>)( *<br />)*)rgx"), "\\2");

    if(infoForParsing.betterCodeTag == true)
    {
        thisMessage.replace(QRegularExpression(R"rgx((<br /> *)*(<(/)?table[^>]*>)( *<br />)*)rgx"), "\\2");
    }

    thisMessage.replace("<br /><br /><ul>", "<ul>");
    thisMessage.replace("<br /><br /><ol>", "<ol>");
    thisMessage.replace("<ul><br /><br />", "</ul>");
    thisMessage.replace("<ol><br /><br />", "</ol>");

    thisMessage.replace("<blockquote>", "<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\" style=\"margin-top: 5px;margin-bottom: 5px;border-color: " + styleTool::getColorInfo().quoteBorderColor + ";" + extraTableStyle + "\"><tr><td>");
    thisMessage.replace("</blockquote>", "</td></tr></table>");

    thisMessage.remove("</div>");
    thisMessage = thisMessage.trimmed();
    while(thisMessage.startsWith("<br />") == true)
    {
        thisMessage.remove(0, 6);
        thisMessage = thisMessage.trimmed();
    }
    while(thisMessage.endsWith("<br />") == true)
    {
        thisMessage.remove(thisMessage.size() - 6, 6);
        thisMessage = thisMessage.trimmed();
    }

    return thisMessage;
}

QString parsingTool::parsingAjaxMessages(QString thisMessage)
{
    thisMessage.remove("\n");
    thisMessage.remove("\r");
    thisMessage.replace(QRegularExpression(R"rgx((?<!\\)\\r)rgx"), "");
    thisMessage.replace(QRegularExpression(R"rgx((?<!\\)\\")rgx"), "\"");
    thisMessage.replace(QRegularExpression(R"rgx((?<!\\)\\/)rgx"), "/");
    thisMessage.replace(QRegularExpression(R"rgx((?<!\\)\\n)rgx"), "\n");
    thisMessage.replace(QRegularExpression(R"rgx((?<!\\)\\t)rgx"), "\t");
    thisMessage.replace("\\\\", "\\");

    QRegularExpressionMatchIterator matchIterator = expForUnicodeInText.globalMatch(thisMessage);
    int lenghtChanged = 0;
    while(matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        thisMessage.replace(match.capturedStart(0) + lenghtChanged, match.capturedLength(0), QChar(match.captured(1).toUpper().toUInt(0, 16)));
        lenghtChanged -= match.capturedLength(0);
        lenghtChanged += 1;
    }

    return thisMessage;
}

QString parsingTool::specialCharToNormalChar(QString thisMessage)
{
    return thisMessage.replace("&amp;", "&").replace("&quot;", "\"").replace("&#039;", "\'").replace("&lt;", "<").replace("&gt;", ">");
}

QNetworkRequest parsingTool::buildRequestWithThisUrl(QString url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", userAgentToUse.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Cookie", "");
    request.setRawHeader("Connection", "Keep-Alive");

    return request;
}

QList<QString> parsingTool::getListOfThisCapNumber(const QString& source, const QRegularExpression& exp, int capNumber, bool globalMatch)
{
    QList<QString> listOfString;
    if(globalMatch == true)
    {
        QRegularExpressionMatchIterator matchIterator = exp.globalMatch(source);
        while(matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            listOfString.push_back(match.captured(capNumber));
        }
    }
    else
    {
        QRegularExpressionMatch match = exp.match(source);
        while(match.hasMatch() == true)
        {
            listOfString.push_back(match.captured(capNumber));
            match = exp.match(source, match.capturedEnd(capNumber));
        }
    }

    return listOfString;
}

void parsingTool::removeAllOverlyQuote(QString& source, int maxNumberQuote)
{
    QRegularExpressionMatch match = expForOverlyQuote.match(source);
    ++maxNumberQuote;
    while(match.hasMatch() == true)
    {
        if(match.captured() == "<blockquote>")
        {
            --maxNumberQuote;
        }
        else if(match.captured() == "</blockquote>")
        {
            ++maxNumberQuote;
        }

        if(maxNumberQuote <= 0)
        {
            int tmpNumberQuote = 0;
            QRegularExpressionMatch secMatch = expForOverlyQuote.match(source, match.capturedEnd());
            while(secMatch.hasMatch() == true)
            {
                if(secMatch.captured() == "<blockquote>")
                {
                    ++tmpNumberQuote;
                }
                else if(secMatch.captured() == "</blockquote>")
                {
                    --tmpNumberQuote;
                }

                if(tmpNumberQuote < 0)
                {
                    break;
                }

                secMatch = expForOverlyQuote.match(source, secMatch.capturedEnd());
            }

            if(secMatch.capturedStart() != -1)
            {
                source.replace(match.capturedEnd(), (secMatch.capturedStart() - match.capturedEnd()), "[...]");
            }
        }

        match = expForOverlyQuote.match(source, match.capturedEnd());
    }
}

void parsingTool::removeAllOverlySpoils(QString& source)
{
    QRegularExpressionMatch spoilOverlyMatcher = expForOverlySpoils.match(source);
    int currentSpoilTagDeepness = 0;
    int lastOffsetOfTag = 0;

    while(spoilOverlyMatcher.hasMatch() == true)
    {
        bool itsEndingTag = (spoilOverlyMatcher.captured() == "</span></span>");

        if(itsEndingTag == false)
        {
            ++currentSpoilTagDeepness;
        }

        if(currentSpoilTagDeepness > 1)
        {
            lastOffsetOfTag = spoilOverlyMatcher.capturedStart();
            source.remove(spoilOverlyMatcher.capturedStart(), spoilOverlyMatcher.capturedLength());
        }
        else
        {
            lastOffsetOfTag = spoilOverlyMatcher.capturedEnd();
        }

        if(itsEndingTag == true)
        {
            --currentSpoilTagDeepness;

            if(currentSpoilTagDeepness < 0)
            {
                currentSpoilTagDeepness = 0;
            }
        }

        spoilOverlyMatcher = expForOverlySpoils.match(source, lastOffsetOfTag);
    }
}

void parsingTool::replaceWithCapNumber(QString& source, const QRegularExpression& exp, int capNumber, QString stringBefore, QString stringAfter, int secondCapNumber,
                                            QString stringAfterAfter, std::function<QString(QString)> capModificator)
{
    QRegularExpressionMatch match = exp.match(source);
    QString newString;

    while(match.hasMatch())
    {
        newString = stringBefore;
        if(capModificator != nullptr)
        {
            newString += capModificator(match.captured(capNumber));
        }
        else
        {
            newString += match.captured(capNumber);
        }
        newString += stringAfter;

        if(secondCapNumber != -1)
        {
            if(capModificator != nullptr)
            {
                newString += capModificator(match.captured(secondCapNumber));
            }
            else
            {
                newString += match.captured(secondCapNumber);
            }
            newString += stringAfterAfter;
        }

        source.replace(match.capturedStart(0), match.capturedLength(0), newString);
        match = exp.match(source, match.capturedStart(0) + newString.size());
    }
}
