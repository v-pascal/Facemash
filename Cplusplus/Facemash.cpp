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
    typedef vector<List*> Sorted;

    enum Choice {
        UNDEFINED = NO_DATA,
        LESS = 0,
        MORE = 1
    };

private:
    bool mStarted;
    List mList;

    char** init() const { // Return initialized sorted list
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
    int missing(Sorted* sorted) const { // Return index of entry not present in sorted list (-1)
        int maxIdx = NO_DATA;

        for (typename Sorted::iterator it = sorted->begin(); it != sorted->end(); ++it) {
            for (typename List::iterator iter = (*it)->begin(); iter != (*it)->end(); ++iter) {
                int idx = indexOf(*(*iter));
                if (idx > maxIdx)
                    maxIdx = idx;
            }
        }
        return (maxIdx == (mList.size() - 1))? NO_DATA:maxIdx;
    }

    /*
    static List* last(Sorted* sorted, T data) { // Return list that contains data at last position
        for (typename Sorted::reverse_iterator iter = sorted->rbegin(); iter != sorted->rend(); ++iter) {
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
    static bool choosed(Sorted* sorted, T* choice) { // Return if choice already done
        bool res = false;
        for (int i = 0; i < sorted->size(); ++i) {
            for (int j = 0; j < (sorted->at(i)->size() - 1); ++j) {
                if (((choice[0] == *sorted->at(i)->at(j)) || (choice[0] == *sorted->at(i)->at(j + 1))) &&
                    ((choice[1] == *sorted->at(i)->at(j)) || (choice[1] == *sorted->at(i)->at(j + 1)))) {

                    res = true;
                    break;
                }
            }
        }
        return res;
    }
    static List* find(Sorted* sorted, typename Sorted::iterator mergeA, typename Sorted::iterator mergeB) {

        // Return list containing entries defined in 'mergeA' following or preceding entries defined in 'mergeB' (excepted bounds)
        int size = (*mergeA)->size() + (*mergeB)->size() - 4; // - 4 -> To remove bounds in the size calculation

        for (typename Sorted::iterator it = sorted->begin(); it != sorted->end(); ++it) {
            if ((it != mergeA) && (it != mergeB) && ((*it)->size() == size)) {
                int idx = NO_DATA;

                if (*(*mergeA)->at(1) == *(*it)->at(0)) {
                    int idxA = 1;
                    for (int i = 1; i < ((*mergeA)->size() - 2); ++i)
                        if (*(*mergeA)->at(++idxA) != *(*it)->at(i))
                            break;
                    if (idxA != ((*mergeA)->size() - 2))
                        continue;
                    idx = idxA;

                    int idxB = 0;
                    for ( ; idx < (*it)->size(); ++idx)
                        if (*(*mergeB)->at(++idxB) != *(*it)->at(idx))
                            break;
                    if (idxB != ((*mergeB)->size() - 2))
                        continue;

                } else if (*(*mergeB)->at(1) == *(*it)->at(0)) {
                    int idxB = 1;
                    for (int i = 1; i < ((*mergeB)->size() - 2); ++i)
                        if (*(*mergeB)->at(++idxB) != *(*it)->at(i))
                            break;
                    if (idxB != ((*mergeB)->size() - 2))
                        continue;
                    idx = idxB;

                    int idxA = 0;
                    for ( ; idx < (*it)->size(); ++idx)
                        if (*(*mergeA)->at(++idxA) != *(*it)->at(idx))
                            break;
                    if (idxA != ((*mergeA)->size() - 2))
                        continue;
                }
                if (idx == (*it)->size())
                    return (*it);
            }
        }
        return NULL;
    }

    int find(T data) const { // Return index position of data in the list (or NO_DATA if not found)
        for (int i = 0; i < mList.size(); ++i)
            if (*mList[i] == data)
                return i;

        return NO_DATA;
    }
    bool done(Sorted* sorted) const { // Return sort completed flag
        if (sorted->at(0)->size() == mList.size()) {
            while (sorted->size() > 1) {
                int size;
                while (size = (*(sorted->end() - 1))->size()) {
                    delete (*(sorted->end() - 1))->at(size - 1);
                    (*(sorted->end() - 1))->pop_back();
                }
                (*(sorted->end() - 1))->clear();
                sorted->erase(sorted->end() - 1);
            }
            return true;
        }
        if (sorted->at(sorted->size() - 1)->size() == mList.size()) {
            while (sorted->size() > 1) {
                int size;
                while (size = (*sorted->begin())->size()) {
                    delete (*sorted->begin())->at(size - 1);
                    (*sorted->begin())->pop_back();
                }
                (*sorted->begin())->clear();
                sorted->erase(sorted->begin());
            }
            return true;
        }
        return false;
    }

    static void sort(Sorted* &sorted, T* choice, bool selection) { // Take in account new choice into sorted list
        if (sorted == NULL)
            sorted = new Sorted();

        int idx = sorted->size();
        if (idx > 0) {
            List* tmp = last(sorted, choice[(selection)? 0:1]);
            if (tmp != NULL) { // ... B + B>C -> ... B C
                tmp->push_back(new int(choice[(selection)? 1:0]));
                return;
            }
            tmp = first(sorted, choice[(selection)? 1:0]);
            if (tmp != NULL) { // B ... + C>B -> C B ...
                tmp->insert(tmp->begin(), new int(choice[(selection)? 0:1]));
                return;
            }
        }
        sorted->push_back(new List());
        sorted->at(idx)->push_back(new int(choice[(selection)? 0:1]));
        sorted->at(idx)->push_back(new int(choice[(selection)? 1:0]));
    }
    T* merge(Sorted* sorted) const { // Merge sorted list if possible and return next choice (if any)
        assert(sorted != NULL);

        // Reply choice with new entry
        int idx = missing(sorted);
        if (idx != NO_DATA) {
            T* res = new T[2];

            res[0] = *mList[idx];
            res[1] = *mList[idx + 1];
            return res;
        }

        // Merge
        for (typename Sorted::iterator toMerge = sorted->begin(); toMerge != (sorted->end() - 1); ++toMerge) {

            bool merged = false;
            for (typename Sorted::iterator it = toMerge + 1; it != sorted->end(); ++it) {

                if ((*(*toMerge)->at(0) == *(*it)->at(0)) &&
                    (*(*toMerge)->at((*toMerge)->size() - 1) == *(*it)->at((*it)->size() - 1))) {

                    // A B, A ... B -> A ... B | C ... B, C B -> C ... B (first & last ==)
                    if ((*toMerge)->size() == 2) sorted->erase(toMerge);
                    else if ((*it)->size() == 2) sorted->erase(it);
                    else { // A ..1.. B, A ..2.. B, ..1|2.. ..2|1.. -> A ..1|2.. ..2|1.. B (first & last ==)

                        List* toKeep = find(sorted, toMerge, it);
                        assert(toKeep != NULL);

                        toKeep->insert(toKeep->begin(), (*toMerge)->at(0));
                        toKeep->insert(toKeep->end(), (*toMerge)->at((*toMerge)->size() - 1));

                        sorted->erase(it);
                        sorted->erase(toMerge);
                    }
                    merged = true;
                    break;
                }
                // A C ..., A ... C (first ==) -> A ... C ...
                if ((*(*toMerge)->at(0) == *(*it)->at(0)) && (*(*toMerge)->at(1) == *(*it)->at((*it)->size() - 1))) {

                    (*it)->insert((*it)->end(), (*toMerge)->begin() + 2, (*toMerge)->end());
                    sorted->erase(toMerge);
                    merged = true;
                    break;
                }
                //  A ... C, A C ... (first ==) -> A ... C ...
                if ((*(*it)->at(0) == *(*toMerge)->at(0)) && (*(*it)->at(1) == *(*toMerge)->at((*toMerge)->size() - 1))) {

                    (*toMerge)->insert((*toMerge)->end(), (*it)->begin() + 2, (*it)->end());
                    sorted->erase(it);
                    merged = true;
                    break;
                }
                // ... B D, B ... D (last ==) -> ... B ... D
                if ((*(*toMerge)->at((*toMerge)->size() - 1) == *(*it)->at((*it)->size() - 1)) &&
                    (*(*toMerge)->at((*toMerge)->size() - 2) == *(*it)->at(0))) {

                    (*it)->insert((*it)->begin(), (*toMerge)->begin(), (*toMerge)->end() - 2);
                    sorted->erase(toMerge);
                    merged = true;
                    break;
                }
                // B ... D, ... B D (last ==) -> ... B ... D
                if ((*(*it)->at((*it)->size() - 1) == *(*toMerge)->at((*toMerge)->size() - 1)) &&
                    (*(*it)->at((*it)->size() - 2) == *(*toMerge)->at(0))) {

                    (*toMerge)->insert((*toMerge)->begin(), (*it)->begin(), (*it)->end() - 2);
                    sorted->erase(it);
                    merged = true;
                    break;
                }
            }
            if (merged)
                break;
        }

        // Check sort done
        if (done(sorted))
            return NULL;

        // Find next choice
        assert(sorted->size() > 1);

        T* res = new T[2];
        for (int i = 0; i < (sorted->size() - 1); ++i) {
            for (int m = i + 1; m < sorted->size(); ++m) {

                for (int j = 0; j < sorted->at(i)->size(); ++j) {
                    for (int k = 0; k < sorted->at(m)->size(); ++k) {

                        if (*sorted->at(i)->at(j) == *sorted->at(m)->at(k)) {
                            if ((!j) || (!k)) { // ... B A ..., ... B C ... -> A ? C
                                assert((j + 1) < sorted->at(i)->size());
                                assert((k + 1) < sorted->at(m)->size());

                                res[0] = *sorted->at(i)->at(j + 1);
                                res[1] = *sorted->at(m)->at(k + 1);

                            } else { // ... A B ..., ... C B ... -> A ? C
                                assert((j - 1) >= 0);
                                assert((k - 1) >= 0);

                                res[0] = *sorted->at(i)->at(j - 1);
                                res[1] = *sorted->at(m)->at(k - 1);
                            }

                            // Check not already choosed
                            if (!choosed(sorted, res))
                                return res;
                        }
                    }
                }
            }
        }
        assert(NULL);
        return NULL;
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

    //
    static void destroy(Sorted* list) { // Destroy sorted list
        while (list->size()) {
            int size;
            while (size = list->at(0)->size()) {
                delete list->at(0)->at(size - 1);
                list->at(0)->pop_back();
            }
            list->at(0)->clear();
            list->erase(list->begin());
        }    
        list->clear();
        delete list;
    }
    void destroy(char** ranking) const { // Destroy ranking
        for (int i = 0; i < mList.size(); ++i)
            delete [] ranking[i];
        
        delete [] ranking;
    }

    T* next(char** &ranking, T* choice, bool selection) { // Update ranking and return next choice (if any)
        if (!mStarted) {
            





            mStarted = true;
        }
        if ((ranking == NULL) && (choice == NULL)) {
            T* res = new T[2];

            res[0] = *mList[0];
            res[1] = *mList[1];
            return res;
        }
        if (ranking == NULL)
            ranking = init();

        // Implement choice into ranking
        ranking[indexOf(choice[0])][indexOf(choice[1])] = (selection)? MORE:LESS;
        ranking[indexOf(choice[1])][indexOf(choice[0])] = (selection)? LESS:MORE;
        delete choice;

        // Sort list according ranking
        Facemash<int>::Sorted* sorted = sort(ranking);
        if ((sorted->size() == 1) && (sorted->at(0)->size() == mList.size())) {

            destroy(sorted);
            return NULL; // DONE
        }

        // Reply choice with missing entry (if any)
        int idx = missing(sorted);
        if (idx != NO_DATA) {
            T* res = new T[2];

            res[0] = *mList[idx];
            res[1] = *mList[idx + 1];
            return res;
        }





        /*
        assert(choice != NULL);
        sort(sorted, choice, selection);
        delete choice;

        return (!done(sorted))? merge(sorted):NULL;
        */
        






        return NULL;
    }
    Sorted* sort(char** ranking) { // Return sorted list according choices






        return NULL;
    }
};

//////
#define TEST

void display(const Facemash<int>::List& list) {
    for (int i = 0; i < list.size(); ++i)
        cout << (*list[i]) << " ";

    cout << endl;
}
void display(const Facemash<int>::Sorted& sorted) {
    cout << endl;
    for (int i = 0; i < sorted.size(); ++i)
        display(*sorted[i]);
}

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
    char** list = NULL;
    bool selection = false;

#ifndef TEST
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    while (choice = facemash->next(list, choice, selection)) {
        char reply;





        cout << endl;
        if (list != NULL)
            display(*list);
        cout << endl;





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
        delete list;
    }
#else
    assert(facemash->size() < 32); // < Bits count in integer (see mask variable in loop below)
    int count = facemash->size() * facemash->size();

    for (unsigned int test = 0; test < count; ++test) {
        if (list != NULL)
            facemash->destroy(list);

        choice = NULL;
        list = NULL;
        int mask = 1;

        cout << "=> Sorted list result #" << test << ": ";

        // Next test
        while (choice = facemash->next(list, choice, selection)) {
            selection = test & mask;
            mask <<= 1;
        }

        // Display result
        Facemash<int>::Sorted* sorted = facemash->sort(list);
        if ((sorted != NULL) && (sorted->size() == 1) && (sorted->at(0)->size() == facemash->size()))
            display(*sorted->at(0));
        else {
            cout << endl << "=> Invalid list result for test #" << test << endl;
            if (sorted != NULL)
                Facemash<int>::destroy(sorted);
            break;
        }
        Facemash<int>::destroy(sorted);
    }
#endif
    facemash->destroy(list);
    delete facemash;

    return 0;
}



