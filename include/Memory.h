#pragma once

#include <list>
#include <algorithm>
#include "../lib/minhook/include/MinHook.h"
#include "Base.h"
#include "Data.h"

#ifdef __X86_ARCH__
  #define BASE_ADDRESS 0x400000h
#else
  #define BASE_ADDRESS 0x140000000h
#endif

namespace scriptwrapper
{

namespace memory
{

/* TODO: Document this class */
template<typename T, typename... Args>
class Patch
{
public:
  data::List<T (*)(Args...)> before;
  data::List<T (*)(Args...)> replace;
  data::List<T (*)(Args...)> after;

  Patch(UINT_PTR address)
  {
    _target = address;
    _detour = [this](Args&&... args) {
      auto iterator = [&](T (*func)(Args...)) {
        func(std::forward<Args>(args));
      };

      before(iterator);
      if (replace.empty())
        _trampoline(std::forward<Args>(args));
      else
        replace(iterator);
      after(iterator);
    };

    MH_CreateHookCast(_target, _detour, &_trampoline);
  }

  ~Patch()
  {
    MH_RemoveHookCast(_target);
  }

private:
  UINT_PTR _target;
  T (*_detour)(Args...);
  T (*_trampoline)(Args...);
};

/* TODO: Document this class */
class Protection
{
public:
  Protection(PVOID address, CONST SIZE_T size, CONST WORD mode)
  {
    _address = address;
    _size = size;
    if (_validSize = size > 0)
      VirtualProtect(address, size, mode, &_oldProtect);
  }

  ~Protection()
  {
    if (_validSize)
      VirtualProtect(_address, _size, _oldProtect, &_oldProtect);
  }

private:
  PVOID _address;
  DWORD _oldProtect;
  SIZE_T _size;
  BOOLEAN _validSize;
};

/* TODO: Document this function */
template<typename T, UINT_PTR address, typename... Args>
inline T Call(Args&&... args)
{
  return reinterpret_cast<T (*)(Args...)>(address)(std::forward<Args>(args)...);
};

/* TODO: Document this function */
template<typename T, typename... Args>
inline T CallDynamic(UINT_PTR address, Args&&... args)
{
  return reinterpret_cast<T (*)(Args...)>(GetDynamicAddress(address))(std::forward<Args>(args)...);
};

/* TODO: Document this function */
inline VOID Fill(UINT_PTR address, BYTE value, SIZE_T size, BOOLEAN vp = FALSE)
{
  PVOID ptr = reinterpret_cast<PVOID>(address);
  Protection protect(ptr, vp ? size : 0);
  memset(ptr, value, size);
};

/* TODO: Document this function */
inline PTR_DIFF GetDynamicAddress(CONST UINT_PTR address)
{
  return GetModuleHandle(nullptr) - BASE_ADDRESS + address;
};

/* TODO: Document this function */
inline PVOID* GetVmt(PVOID self)
{
  return *reinterpret_cast<PVOID**>(self);
};

inline PVOID GetVmt(PVOID self, CONST SIZE_T index)
{
  return GetVmt(self)[index];
};

/* TODO: Document this function */
inline VOID Nop(UINT_PTR address, CONST SIZE_T size, BOOLEAN vp = FALSE)
{
  Fill(address, data::Opcode::NOP, size, vp);
};

/* TODO: Document this function */
template<class T>
inline T& Read(UINT_PTR address, CONST T& value, BOOLEAN vp = FALSE)
{
  PVOID ptr = reinterpret_cast<PVOID>(address);
  Protection protect(ptr, vp ? sizeof(value) : 0);
  return *static_cast<T*>ptr;
};

/* TODO: Document this function */
inline VOID ReadRaw(UINT_PTR address, PVOID buffer, SIZE_T size, BOOLEAN vp = FALSE)
{
  PVOID ptr = reinterpret_cast<PVOID>(address);
  Protection protect(ptr, vp ? size : 0);
  memcpy(buffer, ptr, size);
};

/* TODO: Document this function */
template<class T>
inline VOID Write(UINT_PTR address, CONST T& value, BOOLEAN vp = FALSE)
{
  PVOID ptr = reinterpret_cast<PVOID>(address);
  Protection protect(ptr, vp ? sizeof(value) : 0);
  *static_cast<T*>ptr = value;
};

/* TODO: Document this function */
inline VOID WriteRaw(UINT_PTR address, PVOID buffer, SIZE_T size, BOOLEAN vp = FALSE)
{
  PVOID ptr = reinterpret_cast<PVOID>(address);
  Protection protect(ptr, vp ? size : 0);
  memcpy(ptr, buffer, size);
};

}

}