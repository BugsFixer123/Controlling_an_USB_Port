#include <windows.h>
#include <winusb.h>
#include <setupapi.h>
#include <initguid.h>
#include <tchar.h>
#include <iostream>

#pragma comment(lib, "winusb.lib")
#pragma comment(lib, "setupapi.lib")

DEFINE_GUID(GUID_DEVINTERFACE_MyDevice,
    0xa5dcbf10, 0x6530, 0x11d2,
    0x90, 0x1f, 0x00, 0xc0,
    0x4f, 0xb9, 0x51, 0xed);

bool GetDevicePath(wchar_t* devicePath, size_t bufSize) {
    HDEVINFO deviceInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_MyDevice,
        nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (deviceInfo == INVALID_HANDLE_VALUE)
        return false;

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    if (!SetupDiEnumDeviceInterfaces(deviceInfo, nullptr, &GUID_DEVINTERFACE_MyDevice, 0, &deviceInterfaceData)) {
        SetupDiDestroyDeviceInfoList(deviceInfo);
        return false;
    }

    DWORD requiredSize = 0;
    SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);
    auto deviceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);
    deviceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    if (!SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, deviceDetail, requiredSize, nullptr, nullptr)) {
        free(deviceDetail);
        SetupDiDestroyDeviceInfoList(deviceInfo);
        return false;
    }

    wcsncpy_s(devicePath, bufSize, deviceDetail->DevicePath, _TRUNCATE);
    free(deviceDetail);
    SetupDiDestroyDeviceInfoList(deviceInfo);
    return true;
}

int main() {
    wchar_t devicePath[MAX_PATH];

    if (!GetDevicePath(devicePath, MAX_PATH)) {
        std::cerr << "Device not found." << std::endl;
        return 1;
    }

    std::wcout << L"Device path: " << devicePath << std::endl;

    HANDLE deviceHandle = CreateFileW(devicePath,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        nullptr);

    if (deviceHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open device." << std::endl;
        return 1;
    }

    WINUSB_INTERFACE_HANDLE usbHandle;
    if (!WinUsb_Initialize(deviceHandle, &usbHandle)) {
        std::cerr << "WinUSB initialization failed." << std::endl;
        CloseHandle(deviceHandle);
        return 1;
    }

    std::cout << "WinUSB initialized successfully." << std::endl;

    // TODO: Read/write data using WinUsb_ReadPipe / WinUsb_WritePipe

    WinUsb_Free(usbHandle);
    CloseHandle(deviceHandle);
    return 0;
}
