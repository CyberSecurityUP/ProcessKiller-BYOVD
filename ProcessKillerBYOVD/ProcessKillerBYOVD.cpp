#include <iostream>
#include <string>
#include <Windows.h>
#include <tchar.h>
#include <vector>

#define MAX_PATH 260
#define IOCTL_CODE 0x82730030
#define DEVICE_PATH L"\\\\.\\viragtlt"

struct BYOVD_TEMPLATEIoctlStruct {
    char process_name[500];
};

class BYOVD {
public:
    HANDLE hDevice;
    SC_HANDLE hService;
    SC_HANDLE hSCManager;

    BYOVD() : hService(nullptr), hSCManager(nullptr), hDevice(INVALID_HANDLE_VALUE) {
        hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
        if (!hSCManager) {
            std::cerr << "[!] Failed to open service manager." << std::endl;
            return;
        }

        hService = OpenService(hSCManager, _T("viragt64"), SERVICE_ALL_ACCESS);
        if (!hService) {
            std::cerr << "[!] Service not found, trying to create it." << std::endl;
            createService();
        }

        openDevice();
    }

    ~BYOVD() {
        cleanUp();
    }

    bool startDriver() {
        SERVICE_STATUS_PROCESS status;
        DWORD bytesNeeded;

        if (!QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&status,
            sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
            std::cerr << "[X] Failed to query service status. Error: " << GetLastError() << std::endl;
            return false;
        }

        if (status.dwCurrentState == SERVICE_RUNNING) {
            std::cerr << "[!] Service is already running. Current state: " << status.dwCurrentState << std::endl;
            return true;  // Continue even if the service is running
        }

        if (!StartService(hService, 0, nullptr)) {
            std::cerr << "[X] Failed to start the driver. Error: " << GetLastError() << std::endl;
            return false;
        }

        std::cout << "[!] Driver started successfully." << std::endl;
        return true;
    }

    void openDevice() {
        hDevice = CreateFile(DEVICE_PATH, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hDevice == INVALID_HANDLE_VALUE) {
            std::cerr << "[X] Failed to open device. Error: " << GetLastError() << std::endl;
        }
        else {
            std::cout << "[!] Device opened successfully." << std::endl;
        }
    }

    void cleanUp() {
        if (hDevice != INVALID_HANDLE_VALUE) {
            CloseHandle(hDevice);
        }
        if (hService) {
            CloseServiceHandle(hService);
        }
        if (hSCManager) {
            CloseServiceHandle(hSCManager);
        }
    }

    bool stopDriver() {
        SERVICE_STATUS status;
        if (!ControlService(hService, SERVICE_CONTROL_STOP, &status)) {
            std::cerr << "[X] Failed to stop the driver." << std::endl;
            return false;
        }

        std::cout << "[!] Driver stopped successfully." << std::endl;
        return true;
    }

    void killProcessByName(const std::string& processName) {
        if (hDevice == INVALID_HANDLE_VALUE) {
            std::cerr << "[X] Device handle is invalid. Trying to reopen..." << std::endl;
            openDevice();
            if (hDevice == INVALID_HANDLE_VALUE) return;
        }

        BYOVD_TEMPLATEIoctlStruct ioctlData;
        strncpy_s(ioctlData.process_name, processName.c_str(), sizeof(ioctlData.process_name) - 1);
        ioctlData.process_name[sizeof(ioctlData.process_name) - 1] = '\0';

        DWORD bytesReturned;
        BOOL result = DeviceIoControl(hDevice, IOCTL_CODE, &ioctlData, sizeof(ioctlData), nullptr, 0, &bytesReturned, nullptr);
        if (!result) {
            std::cerr << "[X] IOCTL failed. Error: " << GetLastError() << std::endl;
            return;
        }

        std::cout << "[!] IOCTL sent successfully." << std::endl;
    }

private:
    void createService() {
        TCHAR driverPath[MAX_PATH];
        if (!GetCurrentDirectory(MAX_PATH, driverPath)) {
            std::cerr << "[X] Failed to get current directory. Error: " << GetLastError() << std::endl;
            return;
        }

        std::wstring fullPath = std::wstring(driverPath) + L"\\viragt64.sys";

        DWORD fileAttr = GetFileAttributes(fullPath.c_str());
        if (fileAttr == INVALID_FILE_ATTRIBUTES) {
            std::wcout << L"[!] Driver file not found: " << fullPath << std::endl;
            return;
        }

        hService = CreateService(hSCManager, _T("viragt64"), _T("viragt64"), SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
            fullPath.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr);
        if (!hService) {
            std::cerr << "[!] Failed to create service. Error: " << GetLastError() << std::endl;
            return;
        }

        std::cout << "[!] Service created successfully." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <process_name>" << std::endl;
        return 1;
    }

    std::string processName(argv[1]);
    BYOVD driver;

    if (driver.startDriver()) {
        driver.killProcessByName(processName);
    }

    return 0;
}
