// g++ Facemash.cpp -o Facemash

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <assert.h>
#include <sstream>

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

using namespace std;

template<typename T>
inline string numToStr(T number) {

    ostringstream oss;
    oss << number;
    return oss.str();
};

//
#define MIN_ENTRIES_COUNT   2
#define NO_DATA             -1

template<typename T>
class Facemash {

public:
    virtual ~Facemash() {
        for (typename List::iterator iter = mList.begin(); iter != mList.end(); ++iter)
            delete (*iter);

        mList.clear();
    }
    typedef vector<T*> List;

    enum Choice {
        UNDEFINED = NO_DATA,
        LESS = 0,
        MORE = 1
    };

private:
    bool mStarted;
    List mList;

    inline char** init() const { // Return new ranking
        char** res = new char*[mList.size()];
        for (int i = 0; i < mList.size(); ++i) {
            res[i] = new char[mList.size()];

            for (int j = 0; j < mList.size(); ++j)
                res[i][j] = static_cast<char>(UNDEFINED);
        }
        return res;
    }
    inline int indexOf(T data) const { // Return index of data in list
        for (int res = 0; res < mList.size(); ++res)
            if (*mList[res] == data)
                return res;

        return NO_DATA;
    }
    inline int missing(char** ranking) const { // Return missing first choice entry index (return NO_DATA if done)
        for (int res = 0; res < (mList.size() - 1); ++res)
            if (ranking[res][res + 1] == static_cast<char>(UNDEFINED))
                return res;

        return NO_DATA;
    }
    inline bool done(char** ranking) const { // Return done flag (sort completed)
        for (int i = 1; i < mList.size(); ++i)
            if (ranking[0][i] == static_cast<char>(UNDEFINED))
                return false;
        for (int i = 0; i < (mList.size() - 1); ++i)
            if (ranking[i][mList.size() - 1] == static_cast<char>(UNDEFINED))
                return false;

        return true;
    }





    /*
    void display(char** ranking) {
        cout << endl;
        for (int i = 0; i < mList.size(); ++i) {
            cout << endl;
            for (int j = 0; j < mList.size(); ++j) {
                if (j == i)
                    cout << 'X' << " ";
                else
                    cout << static_cast<int>(ranking[i][j]) << " ";
            }
        }
        cout << endl;
    }
    */





public:
    enum AddResult {

        AR_SUCCEEDED = 0, // Element added
        AR_ALREADY_EXISTS, // Element already exists
        AR_ALREADY_STARTED // Process already started (unable to add element)
    };

    inline AddResult add(T* e) {
        if (mStarted)
            return AR_ALREADY_STARTED;

        if (indexOf(*e) != NO_DATA)
            return AR_ALREADY_EXISTS;

        mList.push_back(e);
        return AR_SUCCEEDED;
    }
    inline const List& get() const { return mList; }
    inline const T* get(int index) const { return (index < mList.size())? mList[index]:NULL; }
    inline int size() const { return mList.size(); }

    static void destroy(List* list) { // Destroy sorted list
        if (list == NULL)
            return;

        int size;
        while (size = list->size()) {
            delete list->at(size - 1);
            list->pop_back();
        }
        list->clear();
        delete list;
    }
    void destroy(char** ranking) const { // Destroy ranking
        if (ranking == NULL)
            return;

        for (int i = 0; i < mList.size(); ++i)
            delete [] ranking[i];
        
        delete [] ranking;
    }

    //
    bool next(char** &ranking, T* &choice, bool selection) { // Update ranking and set next choice if any (return false if none)

        // Check start
        if (ranking == NULL) {
            ranking = init();
            
            assert(choice == NULL);
            choice = new T[2];

            choice[0] = *mList[0];
            choice[1] = *mList[1];
            return true;
        }
        if (done(ranking))
            return false; // Done

        // Implement choice into ranking
        assert(choice != NULL);

        bool starting = (ranking[mList.size() - 2][mList.size() - 1] == static_cast<char>(UNDEFINED));

        int idxA = indexOf(choice[0]);
        int idxB = indexOf(choice[1]);
        ranking[idxA][idxB] = static_cast<char>((selection)? MORE:LESS);
        ranking[idxB][idxA] = static_cast<char>((selection)? LESS:MORE);

        if (done(ranking))
            return false; // Done

        // Fill unnecessary choices (if any):

        // X 1 ? = 1     X 0 0 ? = 0
        //   X 1*     &    X 0 ? = 0
        //     X             X 0*
        //                     X
        bool dir = (idxA > idxB);
        int keepA = (dir)? idxA:idxB;
        int keepB = (dir)? idxB:idxA;
        if (starting) {

            while ((idxA > 0) && (idxB > 0) && (ranking[idxA][idxB] == ranking[idxA - 1][idxB - 1])) {
                --idxA;
                --idxB;

                if (dir) {
                    ranking[keepA][idxB] = ranking[idxA][idxB];
                    ranking[idxB][keepA] = ranking[idxB][idxA];
                } else {
                    ranking[keepA][idxA] = ranking[idxB][idxA];
                    ranking[idxA][keepA] = ranking[idxA][idxB];
                }
            }
            idxA = keepA;
            idxB = keepB;
            while ((idxA < (mList.size() - 1)) && (idxB < (mList.size() - 2)) && (ranking[idxA][idxB] == ranking[idxA + 1][idxB + 1])) {
                ranking[idxA + 1][idxB] = ranking[idxA][idxB];
                ranking[idxB][idxA + 1] = ranking[idxB][idxA];

                ++idxA;
            }
        }
        swap<int>(keepA, keepB);

        // 1 1*? = 1     0 0*? = 0
        //   0        &    1
        //     1             0
        if ((keepA < (mList.size() - 3)) && (keepB > 1) && (keepB < (mList.size() - 1)) &&
            (ranking[keepA + 1][keepB + 1] == static_cast<char>(UNDEFINED)) &&
            (ranking[keepA][keepB] == ranking[keepA][keepB - 1]) && (ranking[keepA][keepB] == ranking[keepA + 2][keepB + 1]) &&
            (ranking[keepA][keepB] != ranking[keepA + 1][keepB])) {

            ranking[keepA][keepB + 1] = ranking[keepA][keepB];
            ranking[keepB + 1][keepA] = ranking[keepB][keepA];

        }

        // 0 1*? = 0     1 0*? = 1     1 0*? = 0     0 1*? = 1
        //   1 0      &    0 1      &    0 0      &    1 1
        //     0             1             0             1
        if ((keepA < (mList.size() - 3)) && (keepB > 1) && (keepB < (mList.size() - 1)) &&
            (ranking[keepA + 1][keepB + 1] != static_cast<char>(UNDEFINED))) {
            ++keepA;
            ++keepB; // Case below
        }
        if ((keepA > 0) && (keepA < (mList.size() - 2)) && (keepB > 2)) {
            if (ranking[keepA][keepB] == ranking[keepA + 1][keepB]) {

                // 0 1 ? = 0     1 0 ? = 1
                //   1 0*     &    0 1*
                //     0             1
                if (((ranking[keepA][keepB] == ranking[keepA - 1][keepB - 2]) &&
                     (ranking[keepA][keepB] != ranking[keepA][keepB - 1]) && (ranking[keepA][keepB] != ranking[keepA - 1][keepB - 1])) ||

                    // 1 0 ? = 0     0 1 ? = 1
                    //   0 0*     &    1 1*
                    //     0             1
                    ((ranking[keepA][keepB] != ranking[keepA - 1][keepB - 2]) &&
                     (ranking[keepA][keepB] == ranking[keepA][keepB - 1]) && (ranking[keepA][keepB] == ranking[keepA - 1][keepB - 1]))) {

                    do {
                        ranking[keepA - 1][keepB] = ranking[keepA][keepB];
                        ranking[keepB][keepA - 1] = ranking[keepB][keepA];

                        // 1 0 0 ? ? = 0     0 1 1 ? ? = 1
                        //   0 0*0 0           1 1*1 1
                        //     0 0 0      &      1 1 1
                        //       0 0               1 1
                        //         0                 1
                        if (++keepB > (mList.size() - 1))
                            break;

                    } while (ranking[keepA][keepB] == ranking[keepA][keepB - 1]);
                }
            } else {

                // 1 1 ? = 1     0 0 ? = 0
                //   1 1*     &    0 0*
                //     0             1
                if ((ranking[keepA][keepB] == ranking[keepA][keepB - 1]) && (ranking[keepA][keepB] == ranking[keepA - 1][keepB - 2])) {

                    ranking[keepA - 1][keepB] = ranking[keepA][keepB];
                    ranking[keepB][keepA - 1] = ranking[keepB][keepA];
                }
            }
        }

        if (done(ranking))
            return false; // Done

        // Check missing first choices
        int idx = missing(ranking);
        if (idx != NO_DATA) {

            choice[0] = *mList[idx];
            choice[1] = *mList[idx + 1];
            return true;
        }

        // Get next choice
        idx = 1;

        idxA = 0;
        idxB = 1;
        while (ranking[idxA][idxB] != static_cast<char>(UNDEFINED)) {
            ++idxA;
            ++idxB;

            if (idxB == mList.size()) {
                idxA = 0;
                idxB = ++idx;
            }
        }
        if ((choice[0] == *mList[idxA]) || (choice[1] == *mList[idxB])) {
            choice[1] = *mList[idxA];
            choice[0] = *mList[idxB];
        } else {
            choice[0] = *mList[idxA];
            choice[1] = *mList[idxB];
        }
        return true;
    }
    List* sort(char** ranking) const { // Return sorted list according ranking
        assert(ranking != NULL);
        assert(done(ranking));

        List* res = new List();
        res->resize(mList.size(), NULL);

        for (int i = 0; i < mList.size(); ++i) {
            int rank = 0;

            for (int j = 0; j < mList.size(); ++j) {
                if (j == i)
                    continue;

                if (ranking[i][j] == static_cast<char>(MORE))
                    ++rank;
            }
            (*res)[mList.size() - 1 - rank] = new T(*mList[i]);
        }
        return res;
    }
};

//////
//#define TEST

void display(const Facemash<int>::List& list) {
    for (int i = 0; i < list.size(); ++i)
        cout << (*list[i]) << " ";

    cout << endl;
}
/*
void display(const Facemash<int>::Sorted& sorted) {
    cout << endl;
    for (int i = 0; i < sorted.size(); ++i)
        display(*sorted[i]);
}
*/

int main() {

    cout << "*** Facemash ***" << endl;
    Facemash<int>* facemash = new Facemash<int>();

    // Fill list
    string entry;
    while ((!entry.empty()) || (facemash->size() < MIN_ENTRIES_COUNT)) {
        cout << "Fill list by entering new integer entry or nothing when full (or 'q' to quit): ";

        getline(cin, entry);
        if (entry.empty()) {
            if (facemash->size() < MIN_ENTRIES_COUNT)
                cout << "Not enough entries in list!" << endl;

        } else if (entry.at(0) == 'q') {
            delete facemash;
            return 0;

        } else switch(facemash->add(new int(atoi(entry.c_str())))) {

            case Facemash<int>::AR_SUCCEEDED: {
                cout << "Integer entry added." << endl;
                break;
            }
            case Facemash<int>::AR_ALREADY_EXISTS: {
                cout << "Integer entry already exists!" << endl;
                break;
            }
            case Facemash<int>::AR_ALREADY_STARTED: {
                cout << "Unable to add entry! Sorting process already started." << endl;
                break;
            }
        }
    }
    cout << "The list is now ready to be sorted: ";
    display(facemash->get());
    cout << endl << "Sort stared..." << endl;

    // Start sorting
    int* choice = NULL;
    char** ranking = NULL;
    bool selection = false;

#ifndef TEST
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    char reply;
    while (facemash->next(ranking, choice, selection)) {

        cout << "-> CHOICE: " << numToStr<int>(choice[0]) << " ? " << numToStr<int>(choice[1]) << endl;
        cout << "Replace '?' character by entering '>' or '<' (or 'q' to quit)" << endl;
        do {
            reply = getchar();
        } while ((reply != '<') && (reply != '>') && (reply != 'q'));

        if (reply == 'q')
            break;
        selection = (reply == '>')? true:false;
        cout << "* Your choice was: " << numToStr<int>(choice[0]) << ((selection)? " > ":" < ") << numToStr<int>(choice[1]) << endl;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    delete [] choice;

    if (reply != 'q') { // Display result
        Facemash<int>::List* sorted = facemash->sort(ranking);
        cout << endl << "=> Sorted list result: ";
        display(*sorted);

        Facemash<int>::destroy(sorted);
    }
#else
    assert(facemash->size() < 32); // < Bits count in integer (see mask variable in loop below)
    int count = facemash->size() * facemash->size();

    for (unsigned int test = 0; test < count; ++test) {
        if (ranking != NULL)
            facemash->destroy(ranking);
        if (choice != NULL)
            delete [] choice;

        choice = NULL;
        ranking = NULL;
        int mask = 1;

        cout << "=> Sorted list result #" << test << ": ";

        // Next test
        while (facemash->next(ranking, choice, selection)) {
            selection = test & mask;
            mask <<= 1;
        }

        // Display result
        Facemash<int>::List* sorted = facemash->sort(ranking);
        display(*sorted);

        Facemash<int>::destroy(sorted);
    }
#endif
    facemash->destroy(ranking);
    delete facemash;

    return 0;
}



