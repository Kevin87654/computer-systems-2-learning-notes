#include <iostream>
#include <vector>
#include <windows.h>
#include <iomanip>

using namespace std;

double get_time_seconds() {
    static LARGE_INTEGER freq;
    static bool initialized = false;
    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = true;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<double>(counter.QuadPart) / freq.QuadPart;
}
int main() {
    const int arrayMB = 256;
    const size_t arrayBytes = static_cast<size_t>(arrayMB) * 1024 * 1024;
    const size_t elementCount = arrayBytes / sizeof(int);

    vector<int> data(elementCount, 1);

    vector<int> stridesBytes = {
        4, 8, 16, 32, 64, 128, 256, 512, 1024
    };

    cout << "Cache line size bandwidth test" << endl;
    cout << "Array size: " << arrayMB << " MB" << endl;
    cout << "Sequential scan with different strides" << endl;
    cout << endl;

    cout << left << setw(16) << "Stride(Bytes)"
        << setw(20) << "Access Count"
        << setw(20) << "Time(s)"
        << setw(20) << "ns/access"
        << setw(20) << "Effective MB/s" << endl;

    cout << "--------------------------------------------------------------------------------" << endl;

    volatile long long sum = 0;

    for (int strideBytes : stridesBytes) {
        int strideInts = strideBytes / sizeof(int);
        long long accessCount = 0;

        // Repeat several rounds to make timing stable
        int rounds = 16;

        double start = get_time_seconds();

        for (int r = 0; r < rounds; r++) {
            for (size_t i = 0; i < elementCount; i += strideInts) {
                sum += data[i];
                accessCount++;
            }
        }

        double end = get_time_seconds();

        double elapsed = end - start;
        double nsPerAccess = elapsed * 1e9 / accessCount;

        // 实际读取的有效数据量：每次只使用一个 int，即 4 bytes
        double effectiveMB = accessCount * sizeof(int) / 1024.0 / 1024.0;
        double effectiveMBps = effectiveMB / elapsed;

        cout << left << setw(16) << strideBytes
            << setw(20) << accessCount
            << setw(20) << fixed << setprecision(6) << elapsed
            << setw(20) << fixed << setprecision(3) << nsPerAccess
            << setw(20) << fixed << setprecision(2) << effectiveMBps
            << endl;
    }

    cout << endl;
    cout << "Result checksum: " << sum << endl;
    cout << "Test finished." << endl;

    return 0;
}
