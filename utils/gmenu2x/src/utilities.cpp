/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo   *
 *   massimiliano.torromeo@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//for browsing the filesystem
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>

//senquack - SD stuff
#include <sys/time.h>	// for gettimeofday
#include <cstdio>
#include <fcntl.h>

#include "utilities.h"

using namespace std;

bool case_less::operator()(string const &left, string const &right) const {
	return strcasecmp(left.c_str(), right.c_str()) < 0;
}

// General tool to strip spaces from both ends:
string trim(const string& s) {
  if(s.length() == 0)
    return s;
  int b = s.find_first_not_of(" \t\r");
  int e = s.find_last_not_of(" \t\r");
  if(b == -1) // No non-spaces
    return "";
  return string(s, b, e - b + 1);
}

void string_copy(string s, char **cs) {
	*cs = (char*)malloc(s.length());
	strcpy(*cs, s.c_str());
}

char * string_copy(string s) {
	char *cs = NULL;
	string_copy(s, &cs);
	return cs;
}

////SD stuff
////senquack - new function to determine whether SD is currently mounted to /mnt/sd/
//bool mountPointUsed(string path) {
//	static bool return_val = false;
//	static string last_path = "";
//	static struct timeval lastread = { 0, 0 };
//	static struct timeval now = { 0, 0 };
//
//	if (path != last_path)
//	{
//		last_path = path;
//		string command;
//		command = "grep \"" + path + " \" /proc/mounts > /dev/null";
//		return_val = (system(command.c_str()) == 0);
//		gettimeofday(&lastread, NULL);
//	} 
//	else
//	{
//		// Already recently handled this path
//		gettimeofday(&now, NULL);
//		// Only poll for same mount status twice a second max.
////		if (((now.tv_sec * 1000000 + now.tv_usec) -
////									(lastread.tv_sec * 1000000 + lastread.tv_usec)) > 500000)
//		if (((now.tv_sec * 1000000 + now.tv_usec) -
//									(lastread.tv_sec * 1000000 + lastread.tv_usec)) > 1000000)
//		{
//			string command;
//			command = "grep \"" + path + " \" /proc/mounts > /dev/null";
//			return_val = (system(command.c_str()) == 0);
//			gettimeofday(&lastread, NULL);
//		}
//	}
//
//	return return_val;
//}

//senquack - new function to see if SD is currently inserted (and presumably mounted):
bool isSDInserted(void) {
	static bool isInserted = false;	
	static struct timeval lastread = { 0, 0 };
	static struct timeval now = { 0, 0 };

	// Already recently handled this path
	gettimeofday(&now, NULL);
	// Don't launch shells more often than necessary
	if ((unsigned int)((now.tv_sec * 1000000 + now.tv_usec) -
								(lastread.tv_sec * 1000000 + lastread.tv_usec)) > 250000)
	{
		gettimeofday(&lastread, NULL);
		FILE *f;
		char buf[8192];
		
		f = fopen("/proc/driver/mmcsd", "r");

		if (f)
		{
			if (!feof(f))
			{
				buf[0] = 0;
				// we only need the very first line, so read it in and analyze it:
				// For reference the first line looks like this:
				// "card inserted   : no"

				fscanf(f, "%8192[^\n^\r]", buf);
				fclose(f);
				int length = strlen(buf);
				if (length > 0)
				{
					if (buf[length-1] == 's')
					{
						// looks like the last word on line was "yes" so it is saying SD is inserted
						isInserted = true;
						return isInserted;
					}
					else
						isInserted = false;	// default case
						return isInserted;	
				}
			}
			else
			{
				fclose(f);
				isInserted=false;	// File was empty, return false as default (this shouldn't happen)
				return isInserted;
			}
		} 
		else
		{
			// since file didn't exist, return false (this might happen if SD is not inserted when GP2X is started)
			isInserted=false;
			return isInserted;
		}
	}
	return isInserted;		// return saved value from last update if we get here
}


// senquack - moved this from the GMenu2X class to here.  Also takes the place of
// mountPointUsed("") because we only ever care about /mnt/sd and if this function
// returns an empty string then we know /mnt/sd is not mounted.
// RETURNS string xxxx/xxxxMB or an empty string if /mnt/sd not mounted
string getDiskFree() {
	static string df = "";
	static struct timeval lastread = { 0, 0 };
	static struct timeval now = { 0, 0 };

	// Already recently handled this path
	gettimeofday(&now, NULL);
	// Don't launch shells more often than necessary
	if ((unsigned int)((now.tv_sec * 1000000 + now.tv_usec) -
								(lastread.tv_sec * 1000000 + lastread.tv_usec)) > 1000000)
	{
		FILE *f;
		stringstream ss;
		char devicepath[100];
		unsigned int fs_capacity = 0;
		unsigned int fs_used = 0;
		unsigned int fs_free = 0;
		
		f = popen("df /mnt/sd | grep /mnt/sd", "r");

		if (f)
		{
			if (!feof(f))
			{
				if (fscanf(f, "%s %u %u %u", devicepath, &fs_capacity, &fs_used, &fs_free) == 4)
				{
					ss << (fs_free / 1024) << "/" << (fs_capacity / 1024) << "MB";
					ss >> df;	
				}
				else
				{
					df = "";
				}
			}

			pclose(f);
		}
		gettimeofday(&lastread, NULL);
	}
	return df;
}

bool fileExists(string file) {
	fstream fin;
	fin.open(file.c_str() ,ios::in);
	bool exists = fin.is_open();
	fin.close();

	return exists;
}

bool rmtree(string path) {
	DIR *dirp;
	struct stat st;
	struct dirent *dptr;
	string filepath;

#ifdef DEBUG
	cout << "RMTREE: " << path << endl;
#endif

	if ((dirp = opendir(path.c_str())) == NULL) return false;
	if (path[path.length()-1]!='/') path += "/";

	while ((dptr = readdir(dirp))) {
		filepath = dptr->d_name;
		if (filepath=="." || filepath=="..") continue;
		filepath = path+filepath;
		int statRet = stat(filepath.c_str(), &st);
		if (statRet == -1) continue;
		if (S_ISDIR(st.st_mode)) {
			if (!rmtree(filepath)) return false;
		} else {
			if (unlink(filepath.c_str())!=0) return false;
		}
	}

	closedir(dirp);
	return rmdir(path.c_str())==0;
}

int max (int a, int b) {
	return a>b ? a : b;
}
int min (int a, int b) {
	return a<b ? a : b;
}
int constrain (int x, int imin, int imax) {
	return min( imax, max(imin,x) );
}

//Configuration parsing utilities
int evalIntConf (int val, int def, int imin, int imax) {
	if (val==0 && (val<imin || val>imax))
		return def;
	val = constrain(val, imin, imax);
	return val;
}
int evalIntConf (int *val, int def, int imin, int imax) {
	*val = evalIntConf(*val, def, imin, imax);
	return *val;
}

string evalStrConf (string val, string def) {
	return val.empty() ? def : val;
}
string evalStrConf (string *val, string def) {
	*val = evalStrConf(*val, def);
	return *val;
}

float max (float a, float b) {
	return a>b ? a : b;
}
float min (float a, float b) {
	return a<b ? a : b;
}
float constrain (float x, float imin, float imax) {
	return min( imax, max(imin,x) );
}

bool split (vector<string> &vec, const string &str, const string &delim, bool destructive) {
	vec.clear();

	if (delim.empty()) {
		vec.push_back(str);
		return false;
	}

	std::string::size_type i = 0;
	std::string::size_type j = 0;

	while(1) {
		j = str.find(delim,i);
		if (j==std::string::npos) {
			vec.push_back(str.substr(i));
			break;
		}

		if (!destructive)
			j += delim.size();

		vec.push_back(str.substr(i,j-i));

		if (destructive)
			i = j + delim.size();

		if (i==str.size()) {
			vec.push_back(std::string());
			break;
		}
	}

	return true;
}

string strreplace (string orig, string search, string replace) {
	string::size_type pos = orig.find( search, 0 );
	while (pos != string::npos) {
		orig.replace(pos,search.length(),replace);
		pos = orig.find( search, pos+replace.length() );
	}
	return orig;
}

string cmdclean (string cmdline) {
	string spchars = "\\`$();|{}&'\"*?<>[]!^~-#\n\r ";
	for (uint i=0; i<spchars.length(); i++) {
		string curchar = spchars.substr(i,1);
		cmdline = strreplace(cmdline, curchar, "\\"+curchar);
	}
	return cmdline;
}

int intTransition(int from, int to, long tickStart, long duration, long tickNow) {
	if (tickNow<0) tickNow = SDL_GetTicks();
	float elapsed = (float)(tickNow-tickStart)/duration;
	//                    elapsed                 increments
	return constrain(round(elapsed*(to-from)),from,to);
}
