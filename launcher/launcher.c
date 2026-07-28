/* Lanceur posé à la racine du dossier distribuable de Windows, qui ne fait que démarrer
 * app\RespawnIRC.exe.
 *
 * Il existe parce que les DLL ne peuvent pas être rangées ailleurs que dans le dossier de
 * l'exécutable qu'elles servent : RespawnIRC.exe a des imports statiques sur Qt5Core.dll et les
 * autres, que le chargeur de Windows résout au démarrage du processus, avant que la moindre ligne
 * de notre code tourne, et il les cherche à côté de l'exécutable. Ni qt.conf ni AddDllDirectory n'y
 * changent quoi que ce soit. L'Universal CRT est encore plus strict : avant Windows 8, les
 * redirecteurs api-ms-win-* ne savent pas résoudre ucrtbase.dll ailleurs que dans ce dossier.
 *
 * Tout descend donc dans app\, et ce lanceur reste seul à la racine pour que l'utilisateur ait une
 * seule chose à cliquer dans l'Explorateur au lieu d'une soixantaine de fichiers.
 *
 * Il est compilé par dist-windows.ps1 avec /MT, sans quoi il dépendrait lui-même de vcruntime140.dll
 * qui se trouve dans app\, et ne démarrerait pas. Il ne dépend ainsi que de USER32 et KERNEL32.
 */

#include <windows.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR commandLine, int showCommand)
{
    wchar_t pathOfLauncher[MAX_PATH];
    wchar_t pathOfApp[MAX_PATH];
    wchar_t pathOfAppDir[MAX_PATH];
    DWORD lengthOfPath;
    STARTUPINFOW infosForStart;
    PROCESS_INFORMATION infosOfProcess;

    (void)instance;
    (void)previousInstance;
    (void)commandLine;
    (void)showCommand;

    /* Le chemin est demandé au système plutôt que déduit du dossier courant : c'est ce qui permet de
     * déplacer le dossier n'importe où après décompression.
     */
    lengthOfPath = GetModuleFileNameW(NULL, pathOfLauncher, MAX_PATH);

    if(lengthOfPath == 0 || lengthOfPath == MAX_PATH)
    {
        return 1;
    }

    /* On retire le nom du lanceur pour ne garder que son dossier, séparateur compris. */
    while(lengthOfPath > 0 && pathOfLauncher[lengthOfPath - 1] != L'\\')
    {
        --lengthOfPath;
    }

    pathOfLauncher[lengthOfPath] = L'\0';

    if(FAILED(StringCchCopyW(pathOfAppDir, MAX_PATH, pathOfLauncher)) ||
       FAILED(StringCchCatW(pathOfAppDir, MAX_PATH, L"app")))
    {
        return 1;
    }

    if(FAILED(StringCchCopyW(pathOfApp, MAX_PATH, pathOfAppDir)) ||
       FAILED(StringCchCatW(pathOfApp, MAX_PATH, L"\\RespawnIRC.exe")))
    {
        return 1;
    }

    ZeroMemory(&infosForStart, sizeof(infosForStart));
    infosForStart.cb = sizeof(infosForStart);
    ZeroMemory(&infosOfProcess, sizeof(infosOfProcess));

    /* Le dossier courant est mis dans app\ : le programme trouve resources\ et themes\ par le chemin
     * de son exécutable et non par le dossier courant, mais autant que tout ce qu'il ouvrirait par un
     * chemin relatif y tombe aussi.
     */
    if(CreateProcessW(pathOfApp, NULL, NULL, NULL, FALSE, 0, NULL, pathOfAppDir, &infosForStart, &infosOfProcess) == 0)
    {
        MessageBoxW(NULL, L"Impossible de démarrer app\\RespawnIRC.exe.", L"RespawnIRC", MB_ICONERROR);
        return 1;
    }

    CloseHandle(infosOfProcess.hProcess);
    CloseHandle(infosOfProcess.hThread);

    return 0;
}
