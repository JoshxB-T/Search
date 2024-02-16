// Functions' declarations and definitions
#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;

/**
 * Cleans a specific token by:
 * - Removing leading and trailing punctuation
 * - Converting to lowercase
 * If the token has no letters, returns the empty string.
 */
string cleanToken(string s) {
    string cleanS; // String that will be returned
    bool hasSpaceOrPunc = true; // Track punctuation
    bool hasOneLetter = false; // Track lettering

    // Loop string
    for (char &c : s) {
        if (isalpha(c)) {
            hasOneLetter = true; // Assign true when we have atleast one letter
            c = tolower(c); // Make string lowercase
        }
    }

    if (!hasOneLetter) {
        return "";
    }

    // Loop string
    for (char c : s) {
        if (hasSpaceOrPunc && (isspace(c) || ispunct(c))) { // Skip over leading punct/space
            continue;
        } else { // Concatnate letters to return string
            hasSpaceOrPunc = false;
            cleanS += c;
        }
    }

    if (cleanS.empty()) {
        return cleanS;
    }

    // Use reverse iterators to loop back of the string and remove tailing punct/space
    for (auto i = cleanS.rbegin(); i != cleanS.rend(); ++i) {
        if (isspace(*i) || ispunct(*i)) {
            cleanS.pop_back();
        } else {
            break; // Stop when we reach anything not punct/space
        }
    }
    
    return cleanS;
}

/**
 * Returns a set of the cleaned tokens in the given text.
 */
set<string> gatherTokens(string text) {
    set<string> gatheredTokens; // Declare set to be returned
    istringstream iss(text); // Using istringstream
    string word; // Read in istringstream

    while (iss >> word) {
        word = cleanToken(word); // Clean the word
        if (word != "") {
            gatheredTokens.insert(word); // Only insert non-empty strings
        }
    }

    return gatheredTokens;
}

/**
 * Builds the inverted index from the given data file. The existing contents
 * of `index` are left in place.
 */
int buildIndex(string filename, map<string, set<string>>& index) {
    ifstream dataFile; // Declare input file stream
    
    dataFile.open(filename); // Open file with given name
    
    if (!dataFile) { // Print error message if failed
        cout << "Invalid filename.\n";
        return 0;
    }

    string sentence, webLink, word; // Declare strings to handle file reading
    set<string> tokenCollection; // Will hold each cleaned token
    int pages = 0; // Number that will be returned

    // Loop while we are able to get two lines of input from file
    while (getline(dataFile, webLink) && getline(dataFile, sentence)) {
        pages++; // Increment pages on each successful read
        tokenCollection = gatherTokens(sentence); // Clean file line

        // Insert each token into index with it's URL
        for (const string &token: tokenCollection) {
            index[token].insert(webLink);
        }
    }
    dataFile.close(); // Close file

    return pages; // Return number of successful pages processed
}

/**
 * Appropriately prints the searchResults with userText assistance
*/
void Print(const set<string> &sR, const string &uT) {
    if (sR.size() == 0 && uT != "") { // Check for user hitting enter
        cout << "Found 0 matching pages\n\n";
    } else if (sR.size() > 0) { // Otherwise print results of the search
        cout << "Found " << sR.size() << " matching pages";
        for (const string page : sR) {
            cout << '\n' << page;
        }
        cout << "\n\n";
    }
}

/**
 * Runs a search query on the provided index, and returns a set of results.
 *
 * Search terms are processed using set operations (union, intersection, difference).
 */
set<string> findQueryMatches(map<string, set<string>>& index, string sentence) {
    set<string> results; // Will be the returned set
    set<string> tempResults; // Will be used in set functions to avoid bugs

    istringstream iss(sentence); // Using istringstream 
    string word; // Read in istringstream

    while (iss >> word) {
        bool modifier = true; // Track what modifier current word has
        bool singleWord = true; // Track if the word does not have any modifiers

        // Check if we have a word and presence of a modifier
        if (!word.empty() && (word[0] == '+' || word[0] == '-')) {
            modifier = (word[0] == '+'); // True will represent '+' modifier
            word = word.substr(1); // Assign word with it's non-modifier portion
            singleWord = false; // Word does have modifiers
        }

        string cleanWord = cleanToken(word); // Clean the word before searching
        auto iterator = index.find(cleanWord); // Get an iterator to the location of cleanWord
        tempResults = iterator->second; // Assign tempVector with the URLs of the word

        // Current word has + mod
        if (modifier && !singleWord)
        { // Want to perform intersection
            set<string> temp;
            set_intersection(tempResults.begin(), tempResults.end(), results.begin(), results.end(), inserter(temp, temp.begin()));
            results = temp;
        }
        // Current word has - mod 
        else if (!modifier && !singleWord)
        { // Want to perform difference
            set<string> temp;
            set_difference(results.begin(), results.end(), tempResults.begin(), tempResults.end(), inserter(temp, temp.begin()));
            results = temp;
        } 
        // Current word does not have any mods
        else
        { // Want to perform union but will use intersection instead  
            set_intersection(tempResults.begin(), tempResults.end(), iterator->second.begin(), iterator->second.end(), inserter(results, results.begin()));
        }
    }
    
    return results; // return the results of our search
}

/**
 * Runs the main command loop for the search program
 */
void searchEngine(string filename) {
    map<string, set<string>> grandWeb; // Declared data structure to hold all file info
    string userText; // Read in user text
   
    int pagesProccessed = buildIndex(filename, grandWeb); // Call buildIndex

    // Print some data before prompting user for search query
    cout << "Stand by while building index...\n";
    cout << "Indexed " << pagesProccessed << " pages containing " << grandWeb.size() << " unique terms\n\n";

    do { // Loop until user hits enter
        cout << "Enter query sentence (press enter to quit): "; // Prompt for input
        getline(cin, userText); // Get input
        set<string> searchResults = findQueryMatches(grandWeb, userText); // Get results
        Print(searchResults, userText); // Print results
    } while(userText != "");

    cout << "Thank you for searching!"; // Print after loop terminates
}
