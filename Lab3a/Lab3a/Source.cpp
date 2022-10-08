#include <iostream>
#include "Command.cpp"
#include "FileExplorer.cpp"
#include <fstream>
#include <windows.h>
#include <map>
#include <mutex>
#include <chrono>
#include <algorithm>
using namespace std::chrono;
CommandExtractor extractor = CommandExtractor();
FileExplorer fileExplorer = FileExplorer();
HANDLE hThreadArray[1000];
bool threadExecuting[1000];
long currentThreads = 0;
long threadsAtTime = 1;
CRITICAL_SECTION CriticalSection;
CRITICAL_SECTION UICriticalSection;
std::map<long, long> o;
vector<map<long, long>> maps;
vector<long> progress;
long progressMax;
map<std::filesystem::path, vector<long>> files;
typedef struct ThreadData1{
    vector<long>* l;
    long initial;
    long threads;
    ThreadData1(vector<long> *l, long initial, long threads) {
        this->l = l;
        this->initial = initial;
        this->threads = threads;
    }
} TD, *PTD;
typedef struct OccurenceResult {
    pair<long, long> occurence;
    milliseconds alg_time;
public:
    OccurenceResult(pair<long, long> occurence, milliseconds alg_time) {
        this->alg_time = alg_time;
        this->occurence = occurence;
    }
} OR, *POR;
OccurenceResult* previousResult;
void resetThreads() {
    for (long i = 0; i < currentThreads; i++) {
        CloseHandle(hThreadArray[i]);
    }
    currentThreads = 0;
}
Command getCommand() {
    cout << fileExplorer.path.string() << ">";
    string command;
    getline(cin, command);
    Command cmd = extractor.extract(command);
    return cmd;
}
void resetThreadExecuting() {
    long threads = currentThreads == 0 ? 1000 : currentThreads;
        for (long i = 0; i < threads; i++) {
            threadExecuting[i] = false;
        }
}
vector<long> generateArray(long n) {
    srand((unsigned)time(NULL));
    vector<long> result(n);
    for (long i = 0; i < n; i++) {
        result[i] = rand() % 1000;
    }
    return result;
}
vector<long>* readArray(string path){
    ifstream s(path);
    vector<long> *r = new vector<long>();
    if (files.find(path) != files.end()) {
        return &files[path];
    }
    if (s.is_open()) {
        while (!s.eof()) {
            long el;
            s >> el;
           (*r).push_back(el);
        }
        files[path] = (*r);
        s.close();
        return r;
    }
    else {
        cout << "File has not been opened, try again" << endl;
        return r;
    }

}
void writeArray(vector<long> r, string path){
    ofstream s(path);
    if (s.is_open()){
        for (auto& e : r) {
            s << e << " ";
        }
        s.close();
        files[std::filesystem::path(path)] = r;
    }
    else {
        s.close();
        throw new invalid_argument("Path should be valid");
    }
    
}
DWORD popular(LPVOID param) {
    PTD d = (PTD)param;
    auto v = d->l;
    long initial, threads;
    initial = d->initial;
    threads = d->threads;
    delete d;
    auto local = &maps[initial];
    auto i = progress.begin()+initial;
    for (*i = 0; (*i)*threads+initial < (*v).size(); (*i)++) 
        (*local)[(*v)[(*i) * threads + initial]]++;
    
    EnterCriticalSection(&CriticalSection);
    for (auto& e : (*local))
        o[e.first] += e.second;
        //InterlockedAdd();
    LeaveCriticalSection(&CriticalSection);
    return 0;
}
void parallelPopular(vector<long> *r, long th_n) {
    o.clear();
    maps.clear();
    progress.clear();
    delete previousResult;
    previousResult = NULL;
    currentThreads = th_n;
    maps = vector<map<long, long>>(th_n, map<long, long>());
    progress = vector<long>(th_n);
    progressMax = (*r).size() / th_n;
    for (long i = 0; i < th_n; i++) {
        auto td = new ThreadData1(r, i, th_n);
        hThreadArray[i] = CreateThread(NULL, 0, popular, td, 0, NULL);
        SuspendThread(hThreadArray[i]);
        threadExecuting[i] = false;
    }
}
DWORD statusBar(LPVOID param) {
    while (true) {
        Sleep(1000);
        pair<long, long> max;
        EnterCriticalSection(&CriticalSection);
        for (long i = 0; i < 1000; i++) {
            long occs = 0;
            for (long j = 0; j < maps.size(); j++) occs += maps[j][i];
            if (occs > max.second)
                max = pair<long, long>(i, occs);
        }
        LeaveCriticalSection(&CriticalSection);
        long avg = 0;
        for (auto& e : progress) {
            avg += e;
        }
        avg = avg / currentThreads;
        double percentage = (((double)avg) / ((double)progressMax));
        EnterCriticalSection(&UICriticalSection);
        cout << avg << " \\ " << progressMax << " " << " | " << percentage * 100.0f << "%" << " | " << max.first << ":" << max.second << endl;
        LeaveCriticalSection(&UICriticalSection);
    }
}
void resume(long i) {
    if (threadExecuting[i] == false) {
        ResumeThread(hThreadArray[i]);
        threadExecuting[i] = true;
    }
    
}
void rush() {
    auto start = high_resolution_clock::now();
    HANDLE statusBarThreadh = CreateThread(NULL, 0, statusBar, NULL, 0, NULL);
    SetThreadPriority(statusBarThreadh, THREAD_PRIORITY_HIGHEST);
    for (long i = 0; i < currentThreads; i += threadsAtTime) {
        for (long j = i; j < i + threadsAtTime && j < currentThreads; j++) {
            ResumeThread(hThreadArray[j]);
            threadExecuting[j] = true;
        }
        EnterCriticalSection(&UICriticalSection);
        cout << "Executing from " << i << " to " << min(i + threadsAtTime, currentThreads) << endl;
        LeaveCriticalSection(&UICriticalSection);
        for (long j = i; j < i + threadsAtTime && j < currentThreads; j++)
            WaitForSingleObject(hThreadArray[j], INFINITE);
    }
    TerminateThread(statusBarThreadh, 0);
    CloseHandle(statusBarThreadh);
    auto stop = high_resolution_clock::now();
    pair<long, long> max = *o.begin();
    for (auto &e : o) {
        if (e.second > max.second)
            max = e;
    }
    
    previousResult = new OccurenceResult(max, duration_cast<milliseconds>(stop - start));
}
void assortey(string path) {
    vector<long>* v;
    v = readArray(path);
    vector<long> ths{ 1,2,4,10,20,100,1000 };
    for (auto& e : ths) {
        parallelPopular(v, e);
        threadsAtTime = e;
        rush();
        if (previousResult != NULL) {
            cout << "Num of Threads: " << e << endl << previousResult->occurence.first << ":" << previousResult->occurence.second << endl;
            cout << "Running time " << previousResult->alg_time.count() << " ms" << endl;
        }
    }
}
void fileSystemCommands(Command command) {
    if (!command.compare("dir")) {
        cout << "\t" << "Type";
        cout << "\t" << "Name" << endl;
        cout << "\t" << "----" << "\t" << "---------" << endl;

        for (std::filesystem::directory_iterator next(fileExplorer.path), end; next != end; ++next) {
            if (std::filesystem::is_directory(next->path())) {
                cout << "\t" << "d";
            }
            else {
                cout << "\t" << "f";
            }
            cout << "\t" + (next->path()).filename().string() << endl;
        }
    }
    if (!command.compare("cd")) {
        string s_path = command.arguments.back();
        if (s_path != ".") {
            std::filesystem::path cdpath;
            if (s_path == "..") {
                cdpath = fileExplorer.path.parent_path();
            }
            else {
                cdpath = std::filesystem::path(s_path);
            }

            if (is_directory(cdpath)) {
                fileExplorer.goTo(absolute(cdpath));
            }
            else {
                // give it a second chance
                cdpath = std::filesystem::path(fileExplorer.path.string() + "\\" + cdpath.string());
                if (std::filesystem::is_directory(cdpath)) {
                    fileExplorer.goTo(std::filesystem::absolute(cdpath));
                }
            }
        }
    }
}
void threadsCommand(Command command) {
    if (!command.compare("threads")) {
        if (currentThreads == 0)
            cout << "No threads have been created!" << endl;
        else {
            for (long i = 0; i < currentThreads; i++) {
                cout << "Thread " << i << " ";
                DWORD exitCode;
                GetExitCodeThread(hThreadArray[i], &exitCode);
                cout << exitCode << " " << (threadExecuting[i] ? "Not suspended" : "Suspended")
                    << " " << GetThreadPriority(hThreadArray[i]) << " priority" << endl;
            }
        }

    }
}
void occsCommand(Command command) {
    if (!command.compare("occs")) {
        resetThreads();
        if (command.arguments.size() == 1) {
            string path = command.arguments.front();
            cout << "Running on 1, 2, 4, 10, 100, 1000 threads in a row: " << endl;
            assortey(path);
        }
        else if (command.arguments.size() == 2) {
            string path = command.arguments.front();
                long th_n = stoi(command.arguments.back());
                auto v = readArray(path);
                threadsAtTime = th_n;
                parallelPopular(v, th_n);
        }
        else if (command.arguments.size() == 3) {
            string path = command.arguments.front();
            long th_n = stoi(command.arguments[1]);
            auto v = readArray(path);
            threadsAtTime = stoi(command.arguments[2]); 
            parallelPopular(v, th_n);
        }
    }
    if (!command.compare("result")) {
        if (previousResult == NULL) {
            cout << "We haven't finished yet or we haven't even started!" << endl;
        }
        else {
            resetThreads();
            cout << "Result is " << previousResult->occurence.first << "(number of occurences is "
                << previousResult->occurence.second << ")" << endl;
            cout << "Running time: " << previousResult->alg_time.count() << " ms" << endl;
            ofstream s("D:\\o.txt");
            if (s.is_open()) {
                s << previousResult->occurence.first << endl;
                s << previousResult->occurence.second << endl;
            }
            else {
                cout << "File cannot be opened." << endl;
            }
        }
    }
    if (!command.compare("resall")) {
        rush();
    }
    if (!command.compare("resume")) {
        if (command.arguments.size() == 1) {
            long i = stoi(command.arguments.front());
            if (i < currentThreads && i >= 0) {
                resume(i);
            }
        }
    }
}
void generationCommand(Command command) {
    if (!command.compare("generate")) {
        if (command.arguments.size() == 2) {
            string path = command.arguments.front();
            long n = stoi(command.arguments.back());
            writeArray(generateArray(n), path);
        }
        else {
            cout << "Invalid arguments" << endl;
        }
    }
}
void priorityCommand(Command command) {
    if (!command.compare("priority")) {
        if (command.arguments.front() == "-h") {
            cout << "List threads first with 'threads' command first. Then use: priority <num_of_thread> <1-5>" << endl;
        }
        else if (command.arguments.size() == 2) {
            long i = stoi(command.arguments.front());
            long priority = stoi(command.arguments.back());
            if (i >= 0 && i < currentThreads) {
                switch (priority) {
                case 1:
                    priority = THREAD_PRIORITY_IDLE;
                    break;
                case 2:
                    priority = THREAD_PRIORITY_BELOW_NORMAL;
                    break;
                case 3:
                    priority = THREAD_PRIORITY_NORMAL;
                    break;
                case 4:
                    priority = THREAD_PRIORITY_ABOVE_NORMAL;
                    break;
                default:
                    priority = THREAD_PRIORITY_HIGHEST;
                    break;
                }
                SetThreadPriority(hThreadArray[i], priority);
                cout << "Priority of Thread #" << i << " has been changed to priority " << priority << endl;
            }
                
        }
    }
}
int main() {
    if (!InitializeCriticalSectionAndSpinCount(&CriticalSection,
        0x00000400))
        return 1;
    if (!InitializeCriticalSectionAndSpinCount(&UICriticalSection,
        0x00000400))
        return 1;
    Command command = getCommand();
    while (command.compare("exit")) {
        fileSystemCommands(command);
        threadsCommand(command);
        occsCommand(command);
        generationCommand(command);
        priorityCommand(command);
        command = getCommand();
    }
    
}