#pragma once
#include <windows.h>
#include <shlobj.h>
#include <string>

// {B1C2D3E4-F5A6-7890-BCDE-F12345678901}
static constexpr CLSID CLSID_PinOverlay =
{ 0xB1C2D3E4, 0xF5A6, 0x7890, { 0xBC, 0xDE, 0xF1, 0x23, 0x45, 0x67, 0x89, 0x01 } };

class PinOverlay : public IShellIconOverlayIdentifier {
public:
    PinOverlay() : m_ref(1) { InterlockedIncrement(&g_refCount); }
    virtual ~PinOverlay() { InterlockedDecrement(&g_refCount); }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    STDMETHODIMP_(ULONG) AddRef()  override { return InterlockedIncrement(&m_ref); }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG r = InterlockedDecrement(&m_ref);
        if (r == 0) delete this;
        return r;
    }

    // IShellIconOverlayIdentifier
    STDMETHODIMP IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib) override;
    STDMETHODIMP GetOverlayInfo(LPWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags) override;
    STDMETHODIMP GetPriority(int* pPriority) override;

    static bool IsPinned(const std::wstring& path);
    static long g_refCount;

private:
    long m_ref;
};
