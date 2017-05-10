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
        if (ranking[0][mList.size() - 1] != static_cast<char>(UNDEFINED))
            return false; // Done

        // Implement choice into ranking
        assert(choice != NULL);

        int idxA = indexOf(choice[0]);
        int idxB = indexOf(choice[1]);
        ranking[idxA][idxB] = static_cast<char>((selection)? MORE:LESS);
        ranking[idxB][idxA] = static_cast<char>((selection)? LESS:MORE);

        if (ranking[0][mList.size() - 1] != static_cast<char>(UNDEFINED))
            return false; // Done

        // Fill unnecessary choices (if any)
        bool dir = (idxA > idxB);
        int idx = (dir)? idxA:idxB;
        while ((idxA > 0) && (idxB > 0) && (ranking[idxA][idxB] == ranking[idxA - 1][idxB - 1])) {
            --idxA;
            --idxB;

            if (dir) {
                ranking[idx][idxB] = ranking[idxA][idxB];
                ranking[idxB][idx] = ranking[idxB][idxA];
            } else {
                ranking[idx][idxA] = ranking[idxB][idxA];
                ranking[idxA][idx] = ranking[idxA][idxB];
            }
        }




        // Check missing first choices
        idx = missing(ranking);
        if (idx != NO_DATA) {

            choice[0] = *mList[idx];
            choice[1] = *mList[idx + 1];
            return true;
        }





        //display(ranking);






        if (ranking[0][mList.size() - 1] != static_cast<char>(UNDEFINED))
            return false; // Done

        // Get next choice
        idxA = 0;
        while (ranking[0][++idxA] != static_cast<char>(UNDEFINED));
        idxB = 0;
        while (ranking[idxB + 1][idxA] == static_cast<char>(UNDEFINED))
            ++idxB;

        choice[0] = *mList[idxA];
        choice[1] = *mList[idxB];

        return true;
    }
    List* sort(char** ranking) const { // Return sorted list according ranking
        assert(ranking != NULL);
        assert(ranking[0][mList.size() - 1] != static_cast<char>(UNDEFINED));

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
    cout << endl << "=> Sorted list result: ";
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

    while (facemash->next(ranking, choice, selection)) {
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
    Facemash<int>::List* sorted = facemash->sort(ranking);
    display(*sorted);

    Facemash<int>::destroy(sorted);
    delete [] choice;

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


            /*
            if (list != NULL) {
                Facemash<int>::Sorted* sorted = facemash->sort(list);
                if (sorted != NULL)
                    display(*sorted->at(0));
            }
            */



        }

        // Display result
        Facemash<int>::Sorted* sorted = facemash->sort(list);
        if ((sorted != NULL) && (sorted->size() == 1) && (sorted->at(0)->size() == facemash->size()))
            display(*sorted->at(0));
        else {
            cout << endl << "=> Invalid list result for test #" << test << endl;
            //Facemash<int>::destroy(sorted);




            if (sorted != NULL) {
                for (int u = 0; u < sorted->size(); ++u)
                    display(*sorted->at(u));
            } else
                cout << "Empty";




            break;
        }
        Facemash<int>::destroy(sorted);




        /*
        if (test == 2) {
            cout << "OLA";
            Facemash<int>::Sorted* sorted = facemash->sort(list);
            if (sorted != NULL) {
                for (int u = 0; u < sorted->size(); ++u)
                    display(*sorted->at(u));
            } else
                cout << "Empty";
            break;
        } 
        */  





    }
#endif
    facemash->destroy(ranking);
    delete facemash;

    return 0;
}



