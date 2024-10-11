#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <filesystem>
#include <condition_variable>
#include <windows.h>
#include <cstdlib>
#include <tchar.h>
#include <chrono>
#include <mutex>
#include <map>
#include <queue>

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

int screen_width = GetSystemMetrics(SM_CXSCREEN);  // get screen width
int screen_height = GetSystemMetrics(SM_CYSCREEN);  // get screen height
double currentAudioTime = 0.0;
mutex audioTimeMutex;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// load boxes from file and scale them
map<int, vector<vector<int>>> scaleBox(double xscale, double yscale, const string& inputPath){
    ifstream file(inputPath);
    if(!file.is_open()){
        cerr << "Error: Unable to open boxes file" << endl;
        return {};
    }

    map<int, vector<vector<int>>> box;
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        int frameIndex, x, y, width, height;
        if (!(iss >> frameIndex >> x >> y >> width >> height)) {
            cerr << "Error: Invalid boxes file" << endl;
            return box;
        }
        box[frameIndex].push_back({int(x * xscale), int(y * yscale), int(width * xscale), int(height * yscale)});
    }
    return box;
}

map<int, vector<vector<int>>> removeSomeBoxes(const map<int, vector<vector<int>>>& boxes, int index){
    // debug use
    // remove some boxes
    map<int, vector<vector<int>>> box = boxes;
    for(auto it = box.begin(); it != box.end();){
        if(it->first % index != 0){
            it = box.erase(it);
        }else{
            it++;
        }
    }
    return box;
}

// a buffer to store windows
class WindowBuffer {
public:
    WindowBuffer(int size) : size(size) {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        WNDCLASS wc = { };

        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = _T("Bad Apple");

        RegisterClass(&wc);

        for (int i = 0; i < size; ++i) {
            HWND hwnd = CreateWindowEx(
                0,
                _T("Bad Apple"),
                _T("Bad Apple"),
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                NULL,
                NULL,
                hInstance,
                NULL
            );

            if (hwnd != NULL) {
                windows.push(hwnd);
            }
        }
    }

    ~WindowBuffer() {
        while (!windows.empty()) {
            DestroyWindow(windows.front());
            windows.pop();
        }
    }

    HWND getWindow() {
        unique_lock<mutex> lock(mtx);
        if (windows.empty()) {
            return NULL;
        }
        HWND hwnd = windows.front();
        windows.pop();
        return hwnd;
    }

    void returnWindow(HWND hwnd) {
        unique_lock<mutex> lock(mtx);
        windows.push(hwnd);
    }

private:
    int size;
    queue<HWND> windows;
    mutex mtx;
};

void displaySingleBox(int x, int y, int width, int height, condition_variable& cv, int& counter, mutex& mtx, int total, WindowBuffer& windowBuffer) {
    HWND hwnd = windowBuffer.getWindow();
    if (hwnd == NULL) {
        return;
    }

    SetWindowPos(hwnd, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Simulate drawing
    this_thread::sleep_for(milliseconds(1000 / 30));

    ShowWindow(hwnd, SW_HIDE);
    windowBuffer.returnWindow(hwnd);

    {
        lock_guard<mutex> lock(mtx);
        counter++;
        if (counter == total) {
            cv.notify_one();
        }
    }
}

void playAudioAndUpdateTime(const string& videoPath, const string& audioPath) {
    // check if audio file exists
    if (!fs::exists(audioPath)) {
        // mp4 to wav
        string command = "ffmpeg -i \"" + videoPath + "\" -ab 160k -ac 2 -ar 44100 -vn \"" + audioPath + "\"";
        system(command.c_str());
    }

    auto startTime = chrono::high_resolution_clock::now();

    // play audio
    string command = "ffplay -nodisp -autoexit \"" + audioPath + "\"";
    thread audioThread([&command]() {
        system(command.c_str());
    });

    // update audio time
    while (true) {
        auto currentTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = currentTime - startTime;

        {
            lock_guard<mutex> lock(audioTimeMutex);
            currentAudioTime = elapsed.count();
        }

        if (currentAudioTime >= 219.1) {
            break;
        }

        this_thread::sleep_for(milliseconds(100));
    }

    audioThread.join();
}

double getAudioTime(){
    lock_guard<mutex> lock(audioTimeMutex);
    return currentAudioTime;
}

void messageLoop() {
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void displayBox(map<int, vector<vector<int>>> boxes, WindowBuffer& windowBuffer) {
    int frameRate = 30; // the video of bad apple is 30fps
    double frameDuration = 1000.0 / frameRate;

    for (auto& [frameIndex, box] : boxes) {
        double audioTime = getAudioTime();
        int expectedFrameIndex = static_cast<int>(audioTime * frameRate); // calculate expected frame index

        if (frameIndex < expectedFrameIndex) {
            continue; // jump to the next frame
        }

        auto frameStart = high_resolution_clock::now();

        vector<thread> threads;
        mutex mtx;
        condition_variable cv;
        int counter = 0;
        int total = box.size();

        for (size_t i = 0; i < box.size(); ++i) {
            auto& b = box[i];
            threads.emplace_back(displaySingleBox, b[0], b[1], b[2], b[3], ref(cv), ref(counter), ref(mtx), total, ref(windowBuffer));
        }

        // wait for all threads to finish
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [&counter, total] { return counter == total; });
        }

        for (auto& t : threads) {
            t.join();
        }

        // calculate sleep time to keep frame rate
        auto frameEnd = high_resolution_clock::now();
        auto elapsed = duration_cast<milliseconds>(frameEnd - frameStart).count();
        if (elapsed < frameDuration) {
            this_thread::sleep_for(milliseconds(static_cast<int>(frameDuration - elapsed)));
        }
    }
}

int main(int argc, char* argv[]){
    if(argc != 2){
        cerr << "Usage: " << argv[0] << " <scale>" << endl;
        return 1;
    }

    string videoPath = "../video/video.mp4";  // DO NOT change the video path
    string audioPath = "../video/audio.wav";  // DO NOT change the audio path

    double scale_width = screen_width / 640.0;
    double scale_height = screen_height / 480.0;
    double scalemax = min(scale_width, scale_height);
    double scale = stod(argv[1]);

    scale = max(0.0, min(scale, scalemax));

    WindowBuffer windowBuffer(100);

    thread displayThread(displayBox, removeSomeBoxes(scaleBox(scale, scale, "../doc/boxes.txt"), 1), ref(windowBuffer));
    thread audioThread(playAudioAndUpdateTime, videoPath, audioPath);

    messageLoop();

    displayThread.join();
    audioThread.join();

    return 0;
}