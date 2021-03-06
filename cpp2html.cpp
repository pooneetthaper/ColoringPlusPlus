/*
 * CSc103 Project 5: Syntax highlighting, part two.
 * See readme.html for details.
 * Please list all references you made use of in order to complete the
 * assignment: your classmates, websites, etc.  Aside from the lecture notes
 * and the book, please list everything.  And remember- citing a source does
 * NOT mean it is okay to COPY THAT SOURCE.  What you submit here **MUST BE
 * YOUR OWN WORK**.
 * References:
 *
 *
 * Finally, please indicate approximately how many hours you spent on this:
 * #hours: 6
 */

#include "fsm.h"
using namespace cppfsm;
#include <iostream>
using std::cin;
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <set>
using std::set;
#include <map>
using std::map;
#include <initializer_list> // for setting up maps without constructors.

// enumeration for our highlighting tags:
enum {
	hlstatement,  // used for "if,else,for,while" etc...
	hlcomment,    // for comments
	hlstrlit,     // for string literals
	hlpreproc,    // for preprocessor directives (e.g., #include)
	hltype,       // for datatypes and similar (e.g. int, char, double)
	hlnumeric,    // for numeric literals (e.g. 1234)
	hlescseq,     // for escape sequences
	hlerror,      // for parse errors, like a bad numeric or invalid escape
	hlident       // for other identifiers.  Probably won't use this.
};

// usually global variables are a bad thing, but for simplicity,
// we'll make an exception here.
// initialize our map with the keywords from our list:
map<string, short> hlmap = {
#include "res/keywords.txt"
};
// note: the above is not a very standard use of #include...

// map of highlighting spans:
map<int, string> hlspans = {
	{hlstatement, "<span class='statement'>"},
	{hlcomment, "<span class='comment'>"},
	{hlstrlit, "<span class='strlit'>"},
	{hlpreproc, "<span class='preproc'>"},
	{hltype, "<span class='type'>"},
	{hlnumeric, "<span class='numeric'>"},
	{hlescseq, "<span class='escseq'>"},
	{hlerror, "<span class='error'>"}
};
// note: initializing maps as above requires the -std=c++0x compiler flag,
// as well as #include<initializer_list>.  Very convenient though.
// to save some typing, store a variable for the end of these tags:
string spanend = "</span>";

string translateHTMLReserved(char c) {
	switch (c) {
		case '"':
			return "&quot;";
		case '\'':
			return "&apos;";
		case '&':
			return "&amp;";
		case '<':
			return "&lt;";
		case '>':
			return "&gt;";
		case '\t': // make tabs 4 spaces instead.
			return "&nbsp;&nbsp;&nbsp;&nbsp;";
		default:
			char s[2] = {c,0};
			return s;
	}
}

short highlighter(string s)
{
	map<string, short>::iterator it;
	it = hlmap.find(s);
	if (it != hlmap.end())
		return hlmap[s];
	return -1;
}

string createSpan(string str, int state){
	string output="";
	switch(state){
		case 0:
			output+=str;
			break;
		case 1: //scanid
			if (hlspans[highlighter(str)] != "") {
				output+=hlspans[highlighter(str)] + str + spanend;
			}
			else {
				output+=str;
			}
			break;
		case 2: //comment
			output+=str + spanend;
			break;
		case 3: //strlit
			output+=hlspans[hlstrlit] + str + spanend;
			break;
		case 4: //readfs
			output+=hlspans[hlcomment] + str;
			break;
			//not sure if correct, since a forward slash within a string would be 
			//highlighted as a string, right?
		case 5: //readesc
			output+=hlspans[hlescseq] + str + spanend;
			break;
			//it higlights the "\" correctly, but the character following it is 
			//not highlighted correctly
		case 6: //scannum
			output+=hlspans[hlnumeric] + str + spanend;
			break;
		case 7: //error
			output+=hlspans[hlerror] + str + spanend;
			break;
	}
	return output;
}

string htmler(string s){
	string output;
	string temp="";
	int cstate = start;
	for (unsigned long i = 0; i < s.length(); i++) {
		int laststate=updateState(cstate,s[i]);
		if (cstate!=laststate){
			if (cstate == 5) {
				//escape char
				output+=createSpan(temp,laststate);
				temp=s[i];
			}
			else if (temp!=""){
				if (laststate != 5) {
					if(i!=s.length()-1){
						output+=createSpan(temp,laststate);
						temp="";
						temp+=translateHTMLReserved(s[i]);
					}
					else{
						if (s[i] == ';') {
							//endline semicolons aren't in spans
							output+=createSpan(temp,laststate);
							output+=s[i];	
						}
						else {
							output+=createSpan(temp+translateHTMLReserved(s[i]),laststate);
						}
					}
				}
				else {
					if (cstate == 3) {
						//1 char escape
						output+=createSpan(temp+translateHTMLReserved(s[i]),laststate);
						temp="";
					}
					else {
						//error
						temp += s[i];
					}
				}
			}else{
				if(i!=s.length()-1) {
					if (s[i] == '"' and laststate == 3) {
						output+=createSpan(translateHTMLReserved(s[i]),laststate);
					}
					else{
						temp+=translateHTMLReserved(s[i]);
					}
				}
				else{
					output+=createSpan(translateHTMLReserved(s[i]),laststate);
				}
			}
		}else{
			if(i!=s.length()-1){
				temp+=translateHTMLReserved(s[i]);
			}else{
				output+=createSpan(temp+translateHTMLReserved(s[i]),laststate);
			}
		}
	}
	return output;
}

int main() {
	string input;
	string output;
	while(getline(cin,input)){
		output+=htmler(input) + "\n";
	}
	cout << output <<"\n";
	return 0;
}