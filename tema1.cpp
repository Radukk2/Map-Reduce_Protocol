#include <iostream>
#include <pthread.h>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <queue>
#include <set>
#include<map>
#include <filesystem>

using namespace std;


struct CompareFiles {
    bool operator()(pair<string,int> a, pair<string,int> b) {
        return filesystem::file_size(a.first) < filesystem::file_size(b.first);
    }
};

struct Payload {
    pthread_barrier_t *barrier, *reducer_barrier;
    pthread_mutex_t *mutex, *mutex_1;
    int id, num_threads_mapper, num_threads_reducer, type;
    vector<vector<pair<string,int>>> *all_words;
    priority_queue<pair<string, int>, vector<pair<string, int>>, CompareFiles> *q;
    map<string,vector<int>> *myMap;
    vector<vector<pair<string,vector<int>>>> thread_vector;
};


void* thread_function(void* arg) {
    Payload payload = *static_cast<Payload*>(arg);
    if (payload.type == 0) {
        set<pair<string, int>> local_data;
        while (1) {
            pthread_mutex_lock(payload.mutex);
            string filename;
            int file_num;
            if (!payload.q->empty()) {

            filename = payload.q->top().first;
            file_num = payload.q->top().second;
            payload.q->pop();
            } else {
                pthread_mutex_unlock(payload.mutex);
                break;
            }
            pthread_mutex_unlock(payload.mutex);
            ifstream file(filename);
            string word;
            while (file >> word) {
                string str;
                for (char ch : word) {
                    if (isalpha(ch))
                        str += tolower(ch);
                }
                local_data.insert({str,file_num});
            }
            file.close();
            pthread_mutex_lock(payload.mutex_1);
            vector<pair<string,int>> v1;
            v1.insert(v1.end(), local_data.begin(), local_data.end());
            payload.all_words->push_back(v1);
            pthread_mutex_unlock(payload.mutex_1);
            local_data.clear();
            v1.clear();
        }
        pthread_barrier_wait(payload.barrier);
    } 
    if (payload.type == 1) {
        pthread_barrier_wait(payload.barrier);
        pthread_mutex_lock(payload.mutex);
        int P = payload.num_threads_reducer;
        int ID = payload.id;
        size_t N = payload.all_words->size();
        int start = ID * (double)N / P;
        int end = min(static_cast<size_t>((ID + 1) * (double)N / P), N);
        pthread_mutex_unlock(payload.mutex);
        map<string, vector<int>> localMap;
        for (int i = start; i < end; i++) {
            for (const auto &pair : payload.all_words->at(i)) {
                localMap[pair.first].push_back(pair.second);
            }
        }
        pthread_mutex_lock(payload.mutex_1);
        for (const auto &[key, val] : localMap) {
            (*payload.myMap)[key].insert((*payload.myMap)[key].end(), val.begin(), val.end());
        }
        N = 26;
        start = ID * (double)N / P;
        end = min(static_cast<size_t>((ID + 1) * (double)N / P), N);
        pthread_mutex_unlock(payload.mutex_1);
        pthread_barrier_wait(payload.reducer_barrier);
        for (int i = start; i < end; i++) {
            vector<pair<string,vector<int>>> a;
            for (const auto &it : *payload.myMap) {
                if (it.first[0] == (char) i + 'a') {
                    vector<int> sortedVector = it.second;
                    sort(sortedVector.begin(), sortedVector.end());
                    a.push_back({it.first, sortedVector});
                }
            }
            payload.thread_vector.push_back(a);
        }
        for (size_t i = 0; i < payload.thread_vector.size(); i++) {
            auto &it = payload.thread_vector[i];
            sort(it.begin(), it.end(), [](pair<string,vector<int>> a, pair<string,vector<int>>b) {
                if (a.second.size() != b.second.size()) {
                    return a.second.size() > b.second.size();
                } else {
                    return a.first < b.first;
                }
            });
            ofstream fout(string(1, static_cast<char>(i + start +'a')) + ".txt");
            for (size_t j = 0; j < it.size(); j++) {
                fout << it[j].first << ":[";
                for (size_t  k= 0; k < it[j].second.size() - 1; k++) {

                    fout << it[j].second[k] << " ";
                }
                fout << it[j].second[it[j].second.size() - 1] << "]\n";
            }
            fout.close();
        }
    }
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Invalid number of arguments\n";
        exit(0);
    }
    int num_threads_mapper = stoi(argv[1]), num_threads_reducer = stoi(argv[2]);
    int num_threads = num_threads_mapper + num_threads_reducer, r;
    string name = argv[3], line;
    ifstream file(name);
    if (!file) {
        cerr << "No such file or directory\n";
        exit(0);
    }
    getline(file, line);
    // vector<string> files;
    priority_queue<pair<string, int>, vector<pair<string, int>>, CompareFiles> q;
    int i = 1;
    while (getline(file, line)) {
        // cout << filesystem::file_size(line) <<  "\n";
        q.push({line, i});
        i++;
    }
    // cout << q.size();
    file.close();

    //initialize synchronization details
    pthread_barrier_t barrier, reducer_barrier;
    pthread_mutex_t mutex, mutex_1;
    pthread_mutex_init(&mutex, nullptr);
    pthread_mutex_init(&mutex_1, nullptr);
    pthread_barrier_init(&barrier, nullptr, num_threads);
    pthread_barrier_init(&reducer_barrier, nullptr, num_threads_reducer);
    pthread_t threads[num_threads];
    vector<Payload> payloads(num_threads);
    vector<vector<pair<string,int>>> all_words;
    map<string, vector<int>> myMap;
    vector<vector<pair<string,vector<int>>>> thread_vector;
    // create threads
    for (int id = 0; id < num_threads; id++) {
        int type;
        if (id < num_threads_mapper) {
            type = 0;
        } else {
            type = 1;
        }
        payloads[id] = Payload{&barrier, &reducer_barrier, &mutex, &mutex_1, id - (type == 1 ? num_threads_mapper : 0), num_threads_mapper , num_threads_reducer, type, &all_words, &q, &myMap, thread_vector};
        r = pthread_create(&threads[id], NULL, thread_function, &payloads[id]);
        if (r) {
            cerr << "Error in thread creation\n";
            exit(0);
        }
    }

    //join all threads and end the code
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], nullptr);
    }
    pthread_barrier_destroy(&barrier);
    pthread_barrier_destroy(&reducer_barrier);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_1);
    file.close();
    return 0;
}
