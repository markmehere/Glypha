#include "GLPrefs.h"
#ifdef GLYPHA_QT
#include <QSettings>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#elif defined(_WIN32)
#include <windows.h>
#endif
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#endif

bool GL::Prefs::load(PrefsInfo& thePrefs)
{
#ifdef GLYPHA_QT
    QSettings settings;
    QByteArray data = settings.value("prefs", QByteArray()).toByteArray();
    if (data.size() != sizeof(thePrefs)) {
        return false;
    }
    memcpy(&thePrefs, data.data(), sizeof(thePrefs));
    return true;
#elif defined(__APPLE__)
    CFDataRef data = (CFDataRef)CFPreferencesCopyAppValue(CFSTR("prefs"), kCFPreferencesCurrentApplication);
    if (!data) {
        return false;
    }
    if (CFGetTypeID(data) != CFDataGetTypeID() || CFDataGetLength(data) != (CFIndex)sizeof(thePrefs)) {
        CFRelease(data);
        return false;
    }
    CFDataGetBytes(data, CFRangeMake(0, CFDataGetLength(data)), (UInt8*)&thePrefs);
    CFRelease(data);
    return true;
#elif defined(_WIN32)
    DWORD size = 0;
    LPCWSTR subkey = L"SOFTWARE\\" GL_GAME_NAME_W;
    LPCWSTR value = L"Prefs";
    if (RegGetValueW(HKEY_CURRENT_USER, subkey, value, RRF_RT_REG_BINARY, nullptr, nullptr, &size) != ERROR_SUCCESS || size != sizeof(thePrefs)) {
        return false;
    }
    return RegGetValueW(HKEY_CURRENT_USER, subkey, value, RRF_RT_REG_BINARY, nullptr, (PVOID)&thePrefs, &size) == ERROR_SUCCESS;
#elif defined(EMSCRIPTEN)
    FILE *fptr;
    fptr = fopen("/glypha/highscores.bin", "rb");
    if (fptr) {
        fread(&thePrefs, sizeof(PrefsInfo), 1, fptr);
        fclose(fptr);
        return true;
    }
    else {
        SDL_Log("No high scores found!");
    }
    return false;
#else
    (void)thePrefs;
    return false;
#endif
}

void GL::Prefs::save(const PrefsInfo& thePrefs)
{
#ifdef GLYPHA_QT
    QSettings settings;
    settings.setValue("prefs", QByteArray((const char*)&thePrefs, sizeof(thePrefs)));
#elif defined(__APPLE__)
    CFDataRef data = CFDataCreate(kCFAllocatorDefault, (const UInt8*)&thePrefs, sizeof(thePrefs));
    if (!data) {
        printf("Failed to create CFData!\n");
    } else {
        CFPreferencesSetAppValue(CFSTR("prefs"), data, kCFPreferencesCurrentApplication);
        CFRelease(data);
    }
#elif defined(_WIN32)
    LPCWSTR subkey = L"SOFTWARE\\" GL_GAME_NAME_W;
    LPCWSTR value = L"Prefs";
    HKEY key = nullptr;
    LSTATUS status = RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key, nullptr);
    if (status == ERROR_SUCCESS) {
        RegSetValueExW(key, value, 0, REG_BINARY, (const BYTE*)&thePrefs, (DWORD)sizeof(thePrefs));
        RegCloseKey(key);
    }
#elif defined(EMSCRIPTEN)
    /*
        To view:
N
        const dbName = '/glypha';
        const storeName = 'FILE_DATA';
        const key = '/glypha/highscores.bin';

        const request = indexedDB.open(dbName);
        request.onsuccess = function(event) {
        const db = event.target.result;
        const transaction = db.transaction(storeName, 'readonly');
        const objectStore = transaction.objectStore(storeName);
        const getRequest = objectStore.getAll();

        getRequest.onsuccess = function(event) {
            const entries = event.target.result;
            let str = '';
            for (let i = 0; i < entries[0].contents.length; i++) str += String.fromCharCode(entries[0].contents[i]);
            console.log(str);
        };
        };
    */

    FILE *fptr;
    fptr = fopen("/glypha/highscores.bin", "wb");
    fwrite(&thePrefs, sizeof(PrefsInfo), 1, fptr);
    fclose(fptr);
    EM_ASM(
        FS.syncfs(function (err) {
            if (err) console.error("Cannot save prefs", err);
            else console.log("High scores saved to virtual file system");
        });
    );
#else
    (void)thePrefs;
#endif
}
