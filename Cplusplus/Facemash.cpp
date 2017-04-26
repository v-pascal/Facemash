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
#define MIN_ENTRIES_COUNT   3
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
    typedef vector<List*> Sorted;

private:
    static bool started;
    List mList;

    static List* last(Sorted* sorted, T data) { // Return list that contains data at last position
        for (typename Sorted::iterator iter = sorted->begin(); iter != sorted->end(); ++iter) {
            if ((*(*iter)->at((*iter)->size() - 1)) == data)
                return (*iter);
        }
        return NULL;
    }
    static List* first(Sorted* sorted, T data) { // Return list that contains data at first position
        for (typename Sorted::iterator iter = sorted->begin(); iter != sorted->end(); ++iter) {
            if ((*(*iter)->at(0)) == data)
                return (*iter);
        }
        return NULL;
    }
    static void sort(Sorted* &sorted, T* choice, bool selection) { // Take in account new choice into sorted list
        if (sorted == NULL)
            sorted = new Sorted();

        int idx = sorted->size();
        if (idx > 0) {
            List* tmp = last(sorted, choice[(selection)? 0:1]);
            if (tmp != NULL) {
                tmp->push_back(new int(choice[1]));
                return;
            }
            tmp = first(sorted, choice[(selection)? 1:0]);
            if (tmp != NULL) {
                tmp->insert(tmp->begin(), new int(choice[1]));
                return;
            }
        }
        sorted->push_back(new List());
        sorted->at(idx)->push_back(new int(choice[(selection)? 0:1]));
        sorted->at(idx)->push_back(new int(choice[(selection)? 1:0]));
    }
    int find(T data) const { // Return index position of data in the list (or NO_DATA if not found)
        for (int i = 0; i < mList.size(); ++i)
            if (*mList[i] == data)
                return i;

        return NO_DATA;
    }
    int missing(Sorted* sorted) const { // Return index of entry not present in sorted list (-1)
        int maxIdx = NO_DATA;

        for (typename Sorted::iterator it = sorted->begin(); it != sorted->end(); ++it) {
            for (typename List::iterator iter = (*it)->begin(); iter != (*it)->end(); ++iter) {
                int idx = find(*(*iter));
                if (idx > maxIdx)
                    maxIdx = idx;
            }
        }
        return (maxIdx == (mList.size() - 1))? NO_DATA:maxIdx;
    }
    T* merge(Sorted* sorted) const { // Merge sorted list if possible and return next choice (if any)
        assert(sorted != NULL);

        int idx = missing(sorted);
        if (idx != NO_DATA) { // Entry missing in sorted list
            T* res = new T[2];

            res[0] = *mList[idx];
            res[1] = *mList[idx + 1];
            return res;
        }








        return NULL; // Sort done
    }

public:
    enum AddResult {

        AR_SUCCEEDED = 0, // Element added
        AR_ALREADY_EXISTS, // Element already exists
        AR_ALREADY_STARTED // Process already started (unable to add element)
    };

    inline AddResult add(T* e) {
        if (started)
            return AR_ALREADY_STARTED;

        if (find(*e) != NO_DATA)
            return AR_ALREADY_EXISTS;

        mList.push_back(e);
        return AR_SUCCEEDED;
    }
    inline const List& get() const { return mList; }
    inline const T* get(int index) const { return (index < mList.size())? mList[index]:NULL; }
    inline int size() const { return mList.size(); }

    //
    T* next(Sorted* &sorted, T* choice, bool selection) const { // Update sorted list and return next choice (if any)
        started = true;

        if ((sorted == NULL) && (choice == NULL)) {
            T* res = new T[2];

            res[0] = *mList[0];
            res[1] = *mList[1];
            return res;
        }
        assert(choice != NULL);
        sort(sorted, choice, selection);
        delete choice;

        return merge(sorted);
    }
};

//////
template<>
bool Facemash<int>::started = false;

void display(const Facemash<int>::List& list) {
    for (int i = 0; i < list.size(); ++i)
        cout << (*list[i]) << " ";

    cout << endl;
}

int main() {

    cout << "*** Facemash ***" << endl;
    Facemash<int>* facemash = new Facemash<int>();

    // Fill list
    string entry;
    while ((!entry.empty()) || (facemash->size() < MIN_ENTRIES_COUNT)) {
        cout << "Fill list by entering new integer entry or nothing when full: ";

        getline(cin, entry);
        if (entry.empty()) {
            if (facemash->size() < MIN_ENTRIES_COUNT)
                cout << "Not enough entries in list!" << endl;
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

    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    // Start sorting
    int* choice = NULL;
    Facemash<int>::Sorted* list = NULL;
    bool selection = false;
    while (choice = facemash->next(list, choice, selection)) {
        char reply;

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

    // Display result
    if ((list != NULL) && (list->size() == 1) && (list->at(0)->size() == facemash->size())) {
        cout << endl << "=> Sorted list result: ";
        display(*list->at(0));
    }
    delete facemash;

    return 0;
}



