RespawnIRC — licences des composants distribués
===============================================

RespawnIRC lui-même
-------------------
Licence zlib, au nom de FranckRJ. Le texte est dans LICENSE-RespawnIRC.txt.
Code source : https://github.com/Spriana/RespawnIRC
Projet d'origine : https://github.com/franckrj/respawnirc


Qt 6 — LGPL v3
--------------
Les bibliothèques Qt distribuées avec ce programme (Qt6Core.dll, Qt6Gui.dll,
Qt6Widgets.dll, Qt6Network.dll, Qt6Multimedia.dll, Qt6WebEngineCore.dll et les
autres Qt6*.dll, ainsi que les greffons des sous-dossiers) sont utilisées sous
les termes de la GNU Lesser General Public License version 3, dont le texte est
dans LGPL-3.0.txt. La LGPLv3 s'applique par-dessus la GNU General Public License
version 3, dont le texte est dans GPL-3.0.txt.

Ces bibliothèques ne sont pas modifiées : ce sont les binaires publiés par The Qt
Company, en version 6.11.2 pour Windows x64 (jeu d'outils MSVC 2022). Leur code
source est disponible chez leur éditeur :

    https://download.qt.io/archive/qt/6.11/6.11.2/single/

La LGPL demande que l'utilisateur puisse remplacer ces bibliothèques par une
version modifiée. C'est le cas ici sans rien faire de particulier : l'édition de
liens est dynamique et les DLL sont posées à côté de l'exécutable, il suffit donc
de les remplacer par d'autres binaires compatibles.


QtWebEngine et Chromium
-----------------------
Qt6WebEngineCore.dll contient Chromium, qui porte ses propres licences (BSD à
trois clauses pour l'essentiel, et une longue liste de licences tierces pour ses
composants). Le détail est publié avec les sources de QtWebEngine, à l'adresse
ci-dessus, dans src/3rdparty.


FFmpeg — LGPL v2.1 ou ultérieure
--------------------------------
avcodec-61.dll, avformat-61.dll, avutil-59.dll, swresample-5.dll et
swscale-8.dll accompagnent QtMultimedia. Ce sont les binaires publiés par The Qt
Company, non modifiés, utilisés sous les termes de la GNU Lesser General Public
License version 2.1 ou ultérieure. Leur source accompagne celle de QtMultimedia,
à l'adresse ci-dessus.


Hunspell et zlib
----------------
Hunspell et zlib sont liés statiquement dans RespawnIRC.exe.

Hunspell est sous triple licence GPL/LGPL/MPL ; c'est la LGPL qui est retenue
ici, texte dans LGPL-3.0.txt pour la v3 — Hunspell étant distribué en « LGPL 2.1
ou ultérieure », la v3 s'applique. Source : https://github.com/hunspell/hunspell

zlib est sous licence zlib, la même que RespawnIRC. Source :
https://github.com/madler/zlib


Dictionnaires
-------------
resources/fr.aff et resources/fr.dic sont le « Dictionnaire orthographique
français toutes variantes » v5.6 d'Olivier R., publié par Dicollecte, sous
licence Mozilla Public License 2.0 — la notice est en tête de fr.aff, et le
texte de la licence est dans MPL-2.0.txt.
Source : http://www.dicollecte.org/
