#include "PinOverlay.h"
#include <shlwapi.h>
#include <fstream>
#include <algorithm>
#include <vector>
#include <sstream>

#pragma comment(lib, "shlwapi.lib")

long PinOverlay::g_refCount = 0;
static HMODULE g_hModule = nullptr;

// ---------- IUnknown ----------

STDMETHODIMP PinOverlay::QueryInterface(REFIID riid, void** ppv) {
    if (riid == IID_IUnknown || riid == IID_IShellIconOverlayIdentifier) {
        *ppv = static_cast<IShellIconOverlayIdentifier*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

// ---------- Helpers ----------

static std::wstring GetPinnedFilePath() {
    wchar_t appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appdata))) {
        return std::wstring(appdata) + L"\\PinExtension\\pinned.txt";
    }
    return L"";
}

static std::wstring ToLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), towlower);
    return s;
}

bool PinOverlay::IsPinned(const std::wstring& path) {
    const std::wstring pinnedFile = GetPinnedFilePath();
    if (pinnedFile.empty()) return false;

    const HANDLE hFile = CreateFileW(pinnedFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
                               nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    const DWORD size = GetFileSize(hFile, nullptr);
    if (size == 0 || size == INVALID_FILE_SIZE) { CloseHandle(hFile); return false; }

    std::string buf(size, '\0');
    DWORD read = 0;
    ReadFile(hFile, &buf[0], size, &read, nullptr);
    CloseHandle(hFile);

    // Convert UTF-8 content to wstring
    const int wlen = MultiByteToWideChar(CP_UTF8, 0, buf.c_str(), static_cast<int>(read), nullptr, 0);
    std::wstring content(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, buf.c_str(), static_cast<int>(read), &content[0], wlen);

    std::wstring needle = ToLower(path);
    while (!needle.empty() && (needle.back() == L'\\' || needle.back() == L'/'))
        needle.pop_back();

    // Split by newline and compare
    std::wistringstream ss(content);
    std::wstring line;
    while (std::getline(ss, line)) {
        while (!line.empty() && (line.back() == L'\r' || line.back() == L'\n' ||
               line.back() == L'\\' || line.back() == L'/'))
            line.pop_back();
        if (ToLower(line) == needle) return true;
    }
    return false;
}

// ---------- IShellIconOverlayIdentifier ----------

STDMETHODIMP PinOverlay::IsMemberOf(const LPCWSTR pwszPath, DWORD /*dwAttrib*/) {
    if (!pwszPath) return S_FALSE;
    return IsPinned(pwszPath) ? S_OK : S_FALSE;
}

STDMETHODIMP PinOverlay::GetOverlayInfo(const LPWSTR pwszIconFile, const int cchMax,
                                         int* pIndex, DWORD* pdwFlags) {
    if (!pwszIconFile || !pIndex || !pdwFlags) return E_INVALIDARG;

    // Use our own DLL as icon source (icon index 0)
    GetModuleFileNameW(g_hModule, pwszIconFile, cchMax);
    *pIndex  = 0;
    *pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
    return S_OK;
}

STDMETHODIMP PinOverlay::GetPriority(int* pPriority) {
    if (!pPriority) return E_INVALIDARG;
    *pPriority = 0; // highest priority
    return S_OK;
}

// ---------- COM Factory ----------

class PinOverlayFactory : public IClassFactory {
public:
    virtual ~PinOverlayFactory() = default;
    PinOverlayFactory() : m_ref(1) {}

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppv = static_cast<IClassFactory*>(this);
            AddRef(); return S_OK;
        }
        *ppv = nullptr; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef()  override { return InterlockedIncrement(&m_ref); }
    STDMETHODIMP_(ULONG) Release() override {
        const ULONG r = InterlockedDecrement(&m_ref);
        if (r == 0) delete this;
        return r;
    }
    STDMETHODIMP CreateInstance(IUnknown* pOuter, REFIID riid, void** ppv) override {
        if (pOuter) return CLASS_E_NOAGGREGATION;
        auto* p = new (std::nothrow) PinOverlay();
        if (!p) return E_OUTOFMEMORY;
        const HRESULT hr = p->QueryInterface(riid, ppv);
        p->Release();
        return hr;
    }
    STDMETHODIMP LockServer(BOOL /*lock*/) override { return S_OK; }

private:
    long m_ref;
};

// ---------- DLL Exports ----------

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (rclsid != CLSID_PinOverlay) return CLASS_E_CLASSNOTAVAILABLE;
    auto* f = new (std::nothrow) PinOverlayFactory();
    if (!f) return E_OUTOFMEMORY;
    const HRESULT hr = f->QueryInterface(riid, ppv);
    f->Release();
    return hr;
}

STDAPI DllCanUnloadNow() {
    return PinOverlay::g_refCount == 0 ? S_OK : S_FALSE;
}

STDAPI DllRegisterServer() {
    wchar_t dllPath[MAX_PATH];
    GetModuleFileNameW(g_hModule, dllPath, MAX_PATH);

    wchar_t clsidStr[64];
    StringFromGUID2(CLSID_PinOverlay, clsidStr, 64);

    // HKCR\CLSID\{...}
    const std::wstring keyBase = std::wstring(L"SOFTWARE\\Classes\\CLSID\\") + clsidStr;
    HKEY hKey;
    RegCreateKeyExW(HKEY_LOCAL_MACHINE, keyBase.c_str(), 0, nullptr,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
    RegSetValueExW(hKey, nullptr, 0, REG_SZ,
                   (BYTE*)L"PinOverlay", sizeof(L"PinOverlay"));
    RegCloseKey(hKey);

    // InprocServer32
    const std::wstring inproc = keyBase + L"\\InprocServer32";
    RegCreateKeyExW(HKEY_LOCAL_MACHINE, inproc.c_str(), 0, nullptr,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
    RegSetValueExW(hKey, nullptr, 0, REG_SZ,
                   (BYTE*)dllPath, (wcslen(dllPath) + 1) * sizeof(wchar_t));
    RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ,
                   (BYTE*)L"Apartment", sizeof(L"Apartment"));
    RegCloseKey(hKey);

    // ShellIconOverlayIdentifiers — spaces before name = high priority
    const std::wstring overlayKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer"
                              L"\\ShellIconOverlayIdentifiers\\  PinOverlay";
    RegCreateKeyExW(HKEY_LOCAL_MACHINE, overlayKey.c_str(), 0, nullptr,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
    RegSetValueExW(hKey, nullptr, 0, REG_SZ,
                   (BYTE*)clsidStr, (wcslen(clsidStr) + 1) * sizeof(wchar_t));
    RegCloseKey(hKey);

    return S_OK;
}

STDAPI DllUnregisterServer() {
    wchar_t clsidStr[64];
    StringFromGUID2(CLSID_PinOverlay, clsidStr, 64);

    const std::wstring keyBase = std::wstring(L"SOFTWARE\\Classes\\CLSID\\") + clsidStr;
    SHDeleteKeyW(HKEY_LOCAL_MACHINE, keyBase.c_str());

    const std::wstring overlayKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer"
                              L"\\ShellIconOverlayIdentifiers\\  PinOverlay";
    SHDeleteKeyW(HKEY_LOCAL_MACHINE, overlayKey.c_str());

    return S_OK;
}
