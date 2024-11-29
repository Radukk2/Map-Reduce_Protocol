#include <iostream>
#include <set>
#include <string>
#include <sstream>
#include <cctype>
using namespace std;

int main() {
    set<string> set;
    string a = "In the peaceful evening, the stars shine brightly in the quiet sky";
    stringstream s(a);
    string word;
    while (s >> word) {
        string str;
        for (int i = 0; i < word.size(); i++) {
            if (word[i] != '.' and word[i] != 'j') {
                str += tolower(word[i]);
            }
        }
        set.insert(str);
    }
    cout << set.size();
    return 0;
}