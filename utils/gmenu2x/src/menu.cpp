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

#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
#include <math.h>

#include "gmenu2x.h"
#include "linkapp.h"
#include "menu.h"
#include "filelister.h"
#include "utilities.h"
#include "pxml.h"

//senquack
//SD stuff
#include "messagebox.h"
#include <sys/statvfs.h>
#include <errno.h>

using namespace std;

Menu::Menu(GMenu2X *gmenu2x) {
	this->gmenu2x = gmenu2x;
	iFirstDispSection = 0;

	DIR *dirp;
	struct stat st;
	struct dirent *dptr;
	string command_line;
	string filepath;

	//SD stuff
	//senquack
	if (gmenu2x->fwType=="open2x" && gmenu2x->o2x_store_links_on_SD)
	{
		int statRet;
		bool import_success = false;

		// Is an SD inserted at this very moment?
//		if (mountPointUsed("/mnt/sd"))
//		if (getDiskFree() != "")
		if (isSDInserted())
		{
			// Test if we have a Payback SD inserted. Never, ever write anything to
			// the Payback SD because it will screw up the copy protection.
			if ((dirp = opendir("/mnt/sd/Payback")) != NULL)
			{
				// Payback SD inserted
				gmenu2x->o2x_SD_read_only = true;
				closedir(dirp);		// close Payback directory we tested for and still have open
			}
			
			// Test for low disk space on mounted SD:
			struct statvfs b;

			int ret = statvfs("/mnt/sd", &b);
			if (ret==0)
			{
				double mb_free = (double)b.f_bfree * (double)b.f_bsize / 1048576.0;
				if (mb_free < 0.2)
				{
					// We are below .2MB free space on SD, do not write more to it.
					gmenu2x->o2x_SD_read_only = true;
					cout << "Detected less than 500KB free space on SD, disallowing further writes." << endl;
//					MessageBox mb(gmenu2x, gmenu2x->tr["Warning: SD has very little free space. Further writes disabled until more is made free."], "icons/section.png");
//					mb.exec();
				}
			}
				
			// Since an SD is currently mounted, check to see if it has a special
			// 	folder containing gmenu2x links called .open2x_gmenu2x/
			filepath = gmenu2x->o2x_links_folder + "/sections/";
			if ((dirp=opendir(filepath.c_str())) != NULL)
			{
				while ((dptr = readdir(dirp)))
				{
					if (dptr->d_name[0]=='.') continue;

					// Ignore any section folders on SDs with name of "applications"
					// 	or "settings" as these are stored only on Open2X NAND
					if (strcasecmp(dptr->d_name, "applications") == 0) continue;
					if (strcasecmp(dptr->d_name, "settings") == 0) continue;

					filepath = gmenu2x->o2x_links_folder + "/sections/" + dptr->d_name;
					statRet = stat(filepath.c_str(), &st);
					if (!S_ISDIR(st.st_mode)) continue;
					if (statRet != -1)
					{
						sections.push_back((string)dptr->d_name);
						linklist ll;
						links.push_back(ll);
					}
				}
				closedir(dirp);
			}
			else
			{
				// There is no special open2x links folder on the SD yet.
				if (!gmenu2x->o2x_SD_read_only)
				{
					// check if there is an old gmenu2x links section on sd
					if ((dirp=opendir("/mnt/sd/gmenu2x/sections")) != NULL)
					{
						closedir(dirp);

						cout << "Detected old gmenu2x links section." << endl;
						fflush(NULL);
						
						// If /mnt/sd/gmenu2x/sections does exist, we seem to have a SD now inserted that
						// hasn't been used with open2x before but does have gmenu2x links we can 
						// import.
						if (mkdir(gmenu2x->o2x_links_folder.c_str(), 0777) == 0)
						{
							cout << "Created o2x links folder" << endl;
							fflush(NULL);

							filepath = gmenu2x->o2x_links_folder + (string)"/sections";
							if (mkdir(filepath.c_str(), 0777) == 0) 
							{
								cout << "Created o2x_links_folder/sections/" << endl;
								fflush(NULL);

//								MessageBox mb(gmenu2x, gmenu2x->tr["Import old GMenu2X links found on this SD?"], "icons/section.png");
//								mb.buttons[ACTION_B] = gmenu2x->tr["Yes"];
//								mb.buttons[ACTION_X] = gmenu2x->tr["No"];
//
//								if (mb.exec() == ACTION_B)
								if (gmenu2x->o2x_auto_import_links)
								{
									gmenu2x->ledOn();
									cout << "Importing old GMenu2X links.." << endl;
									command_line = "cp -R /mnt/sd/gmenu2x/sections/* " +
															gmenu2x->o2x_links_folder + "/sections/";
									if (system(command_line.c_str()) == 0)
									{

										// No need to keep the applications or settings folders, we ignore
										// 	any found on the SD anyway.
										command_line = "rm -rf " + gmenu2x->o2x_links_folder + 
																"/sections/applications " + gmenu2x->o2x_links_folder +
																"/sections/settings " + gmenu2x->o2x_links_folder +
																"/sections/Applications " + gmenu2x->o2x_links_folder + 
																"/sections/Settings ";
										system(command_line.c_str());
										import_success = true;
									}
									sync();
									gmenu2x->ledOff();

									if (import_success)
									{
										cout << "Links imported successfully." << endl;
										fflush(NULL);

										// Now, read the imported links
										filepath = gmenu2x->o2x_links_folder + "/sections/";
										if ((dirp=opendir(filepath.c_str())) != NULL)
										{
											while ((dptr = readdir(dirp)))
											{
												if (dptr->d_name[0]=='.') continue;

												// Ignore any section folders on SDs with name of "applications"
												// 	or "settings" as these are stored only on Open2X NAND
												if (strcasecmp(dptr->d_name, "applications") == 0) continue;
												if (strcasecmp(dptr->d_name, "settings") == 0) continue;

												filepath = gmenu2x->o2x_links_folder + "/sections/" + dptr->d_name;
												statRet = stat(filepath.c_str(), &st);
												if (!S_ISDIR(st.st_mode)) continue;
												if (statRet != -1)
												{
													sections.push_back((string)dptr->d_name);
													linklist ll;
													links.push_back(ll);
												}
											}
											closedir(dirp);
										}
//										MessageBox mb(gmenu2x, gmenu2x->tr["Links imported successfully."], "icons/section.png");
//										mb.buttons[ACTION_B] = gmenu2x->tr["OK"];
//										mb.exec();
									}
									else
									{
//										MessageBox mb(gmenu2x, gmenu2x->tr["Links failed to import successfully. Creating "
//												"new links folder on SD."], "icons/section.png");
//										mb.buttons[ACTION_B] = gmenu2x->tr["OK"];
//										mb.exec();

										gmenu2x->ledOn();

										cout << "Failed to import old GMenu2X links. Creating new links folder"
											<< endl << "on SD under " << gmenu2x->o2x_links_folder << endl;
										fflush(NULL);
										
										command_line = "rm -rf " + gmenu2x->o2x_links_folder;
										system(command_line.c_str());
										command_line = "mkdir -p  " + gmenu2x->o2x_links_folder + "/sections";
										if (system(command_line.c_str()) != 0)
										{
											cout << "Failed to create new links folder." << endl;
											fflush(NULL);
										}
										sync();
										gmenu2x->ledOff();
									}
								} 
							}
						}
					}
					else
					{
						// There is no old gmenu2x links folder, so just create a new open2x links
						// folder on the SD.
						command_line = "mkdir -p  " + gmenu2x->o2x_links_folder + "/sections";
						if (system(command_line.c_str()) != 0)
						{
							cout << "Failed to create new links folder:" << gmenu2x->o2x_links_folder + 
								"/sections" << endl;
							fflush(NULL);
						}
						sync();
					}
				}
				else
				{
					cout << "Very little free space remaining on SD, writes disabled." << endl;
					fflush(NULL);
//					MessageBox mb(gmenu2x, gmenu2x->tr["Warning: very little free space on SD. Link/section creation disabled."], "icons/section.png");
//					mb.exec();
				}
			}
		}

		// senquack - SD stuff
		// Now we load the two sections that are always stored on NAND,
		// settings & applications:
		if ((dirp = opendir("sections")) != NULL)
		{
			while ((dptr = readdir(dirp)))
			{
				if ((strcasecmp(dptr->d_name, "settings") != 0) &&
						(strcasecmp(dptr->d_name, "applications") != 0)) continue;
				filepath = (string)"sections/"+dptr->d_name;
				statRet = stat(filepath.c_str(), &st);
				if (!S_ISDIR(st.st_mode)) continue;
				if (statRet != -1)
				{
					sections.push_back((string)dptr->d_name);
					linklist ll;
					links.push_back(ll);
				}
			}
			closedir(dirp);
		}
	}
	else
	{
		// Not running on open2x firmware or are but are set to using on-NAND links only.
		// Just load all available sections from current directory:
	
		if ((dirp = opendir("sections")) == NULL) return;

		while ((dptr = readdir(dirp))) {
			if (dptr->d_name[0]=='.') continue;
			filepath = (string)"sections/"+dptr->d_name;
			int statRet = stat(filepath.c_str(), &st);
			if (!S_ISDIR(st.st_mode)) continue;
			if (statRet != -1) {
				sections.push_back((string)dptr->d_name);
				linklist ll;
				links.push_back(ll);
			}
		}
		closedir(dirp);
	}

	sort(sections.begin(),sections.end(),case_less());
	setSectionIndex(0);
	readLinks();
}

Menu::~Menu() {
	freeLinks();
}

uint Menu::firstDispRow() {
	return iFirstDispRow;
}

void Menu::loadIcons() {
	//reload section icons
	for (uint i=0; i<sections.size(); i++) {

		// senquack - SD stuff
//		string sectionIcon = "sections/"+sections[i]+".png";
		string sectionIcon;

		if (gmenu2x->fwType != "open2x") 
		{
			// Not running on open2x firmware, so all sections icons are beneath current directory
			sectionIcon =  "sections/" + sections[i] + ".png";
		} 
		else  
		{
			// We are running on open2x firmware, so we store some section icons on  SD (usually) 
			//		in a special hidden folder, unless disabled by user.
			if ((strcasecmp((sections[i]).c_str(), "applications") != 0) &&
				 (strcasecmp((sections[i]).c_str(), "settings") != 0) &&
				 gmenu2x->o2x_store_links_on_SD)
			{
				sectionIcon = gmenu2x->o2x_links_folder + "/sections/" + sections[i] + ".png";
			} else
			{
				sectionIcon = "sections/" + sections[i] + ".png";
			}
		}

		if (!gmenu2x->sc.getSkinFilePath(sectionIcon).empty())
			gmenu2x->sc.add("skin:"+sectionIcon);

		//check link's icons
		string linkIcon;
		for (uint x=0; x<sectionLinks(i)->size(); x++) {
			linkIcon = sectionLinks(i)->at(x)->getIcon();
			LinkApp *linkapp = dynamic_cast<LinkApp*>(sectionLinks(i)->at(x));

			if (linkIcon.substr(0,5)=="skin:") {
				linkIcon = gmenu2x->sc.getSkinFilePath(linkIcon.substr(5,linkIcon.length()));
				if (linkapp != NULL && !fileExists(linkIcon))
					linkapp->searchIcon();
				else
					sectionLinks(i)->at(x)->setIconPath(linkIcon);

			} else if (!fileExists(linkIcon)) {
				if (linkapp != NULL) linkapp->searchIcon();
			}
		}
	}
}

/*====================================
   SECTION MANAGEMENT
  ====================================*/
void Menu::freeLinks() {
	for (vector<linklist>::iterator section = links.begin(); section<links.end(); section++)
		for (linklist::iterator link = section->begin(); link<section->end(); link++)
			free(*link);
}

linklist *Menu::sectionLinks(int i) {
	if (i<0 || i>(int)links.size())
		i = selSectionIndex();

	if (i<0 || i>(int)links.size())
		return NULL;

	return &links[i];
}

void Menu::decSectionIndex() {
	setSectionIndex(iSection-1);
}

void Menu::incSectionIndex() {
	setSectionIndex(iSection+1);
}

uint Menu::firstDispSection() {
	return iFirstDispSection;
}

int Menu::selSectionIndex() {
	return iSection;
}

string Menu::selSection() {
	return sections[iSection];
}

void Menu::setSectionIndex(int i) {
	if (i<0)
		i=sections.size()-1;
	else if (i>=(int)sections.size())
		i=0;
	iSection = i;

	if (i>(int)iFirstDispSection+4)
		iFirstDispSection = i-4;
	else if (i<(int)iFirstDispSection)
		iFirstDispSection = i;

	iLink = 0;
	iFirstDispRow = 0;
}

string Menu::sectionPath(int section) {
	if (section<0 || section>(int)sections.size()) section = iSection;
	//	senquack - SD stuff
//	return "sections/"+sections[section]+"/";

//	string linkpath = "sections/"+section+"/"+title;

	//senquack
	//SD stuff - allow storage of most links on SD while gmenu2x is installed on NAND
	//	NOTE: "applications" and "settings" sections are reserved for open2x and are always
	//			stored and retrieved from the NAND.
	if (gmenu2x->fwType != "open2x") {
		// Not running on open2x firmware, so all sections are beneath current directory
		return "sections/" + sections[section] + "/";
	} else  {
		// We are running on open2x firmware, so we store some links on SD (usually) 
		//		in a special hidden folder, unless disabled by user, and some links on
		//		the NAND.
		if ((strcasecmp((sections[section]).c_str(), "applications") != 0) &&
			 (strcasecmp((sections[section]).c_str(), "settings") != 0) &&
			 gmenu2x->o2x_store_links_on_SD)
		{
			return gmenu2x->o2x_links_folder + "/sections/" + sections[section] + "/";
		} else
		{
			return "sections/" + sections[section] + "/";
		}
	}
}

/*====================================
   LINKS MANAGEMENT
  ====================================*/
bool Menu::addActionLink(uint section, string title, LinkRunAction action, string description, string icon) {
	if (section>=sections.size()) return false;

	LinkAction *linkact = new LinkAction(gmenu2x,action);
	linkact->setSize(gmenu2x->skinConfInt["linkWidth"],gmenu2x->skinConfInt["linkHeight"]);
	linkact->setTitle(title);
	linkact->setDescription(description);
	if (gmenu2x->sc.exists(icon) || (icon.substr(0,5)=="skin:" && !gmenu2x->sc.getSkinFilePath(icon.substr(5,icon.length())).empty()) || fileExists(icon))
	linkact->setIcon(icon);

	sectionLinks(section)->push_back(linkact);
	return true;
}

//senquack
// SD stuff - altered function
bool Menu::addLink(string path, string file, string section) {
	if (section=="")
		section = selSection();
	else if (find(sections.begin(),sections.end(),section)==sections.end()) {
		//section directory doesn't exists
		if (!addSection(section))
			return false;
	}
	if (path[path.length()-1]!='/') path += "/";

	//if the extension is not equal to gpu or gpe then enable the wrapepr by default
	bool wrapper = true, pxml = false;

	//strip the extension from the filename
	string title = file;
	string::size_type pos = title.rfind(".");
	if (pos!=string::npos && pos>0) {
		string ext = title.substr(pos, title.length());
		transform(ext.begin(), ext.end(), ext.begin(), (int(*)(int)) tolower);
		if (ext == ".gpu" || ext == ".gpe") wrapper = false;
		else if (ext == ".pxml") pxml = true;
		title = title.substr(0, pos);
	}

	if ( gmenu2x->fwType=="open2x" )
	{
		if	((strcasecmp(section.c_str(), "applications") == 0) &&
					((strcasecmp(title.c_str(), "ebook") == 0) 			||
					(strcasecmp(title.c_str(), "filemanager") == 0) 	||
					(strcasecmp(title.c_str(), "image") == 0) 			||
					(strcasecmp(title.c_str(), "music") == 0) 			||
					(strcasecmp(title.c_str(), "pdf") == 0) 				||
					(strcasecmp(title.c_str(), "video") == 0)))
		{
			cout << "Links of that name in this section not allowed." << endl;
			// No new links are allowed in settings folder
			return false;
		}

		if (strcasecmp(section.c_str(), "settings") == 0)
		{
			cout << "Additional links in this section not allowed." << endl;
			// No new links are allowed in settings folder
			return false;
		}

		if ((strcasecmp(section.c_str(), "applications") != 0) &&
					gmenu2x->o2x_SD_read_only && gmenu2x->o2x_store_links_on_SD)
		{
			cout << "SD is not writable, free up more space." << endl;
			return false;
		}

	}

	string linkpath = "sections/"+section+"/"+title;

	//senquack
	//SD stuff - allow storage of most links on SD while gmenu2x is installed on NAND
	//	NOTE: "applications" and "settings" sections are reserved for open2x and are always
	//			stored and retrieved from the NAND.
	if ((strcasecmp(section.c_str(), "applications") != 0) ||
		 (strcasecmp(section.c_str(), "settings") != 0))
	{

		if (gmenu2x->fwType=="open2x" && gmenu2x->o2x_store_links_on_SD) 
		{
			// prepend path to special .open2x_gmenu2x/ links folder in root of SD
			linkpath = gmenu2x->o2x_links_folder + "/" + linkpath;
		}
	}

	int x=2;
	while (fileExists(linkpath)) 
	{
		stringstream ss;
		linkpath = "";
		ss << x;
		ss >> linkpath;
		linkpath = "sections/"+section+"/"+title+linkpath;

		//senquack - SD stuff
		if ((strcasecmp(section.c_str(), "applications") != 0) && 
			 (strcasecmp(section.c_str(), "settings") != 0))
		{
			if (gmenu2x->fwType=="open2x" && gmenu2x->o2x_store_links_on_SD)
			{
				// prepend path to special .open2x_gmenu2x/ links folder in root of SD
				linkpath = gmenu2x->o2x_links_folder + "/" + linkpath;
			}
		}

		x++;
	}

#ifdef DEBUG
	cout << "\033[0;34mGMENU2X:\033[0m Adding link: " << linkpath << endl;
#endif

	//search for a manual
	pos = file.rfind(".");
	string exename = path+file.substr(0,pos);
	string manual = "";
	if (fileExists(exename+".man.png")) {
		manual = exename+".man.png";
	} else if (fileExists(exename+".man.jpg")) {
		manual = exename+".man.jpg";
	} else if (fileExists(exename+".man.jpeg")) {
		manual = exename+".man.jpeg";
	} else if (fileExists(exename+".man.bmp")) {
		manual = exename+".man.bmp";
	} else if (fileExists(exename+".man.txt")) {
		manual = exename+".man.txt";
	} else {
		//scan directory for a file like *readme*
		FileLister fl(path, false);
		fl.setFilter(".txt");
		fl.browse();
		bool found = false;
		for (uint x=0; x<fl.size() && !found; x++) {
			string lcfilename = fl[x];

			if (lcfilename.find("readme") != string::npos) {
				found = true;
				manual = path+fl.files[x];
			}
		}
	}
#ifdef DEBUG
	cout << "\033[0;34mGMENU2X:\033[0m Manual: " << manual << endl;
#endif

	// Read pxml
	string shorttitle="", description="", exec="", icon="";
	if (pxml) {
		PXml pxmlDoc(path+file);
		if (pxmlDoc.isValid()) {
			shorttitle = pxmlDoc.getTitle();
			description = pxmlDoc.getDescription();
			exec = pxmlDoc.getExec();
			if (!exec.empty() && exec[0]!='/')
				exec = path+exec;
			icon = pxmlDoc.getIcon();
			if (!icon.empty() && icon[0]!='/')
				icon = path+icon;
		} else {
#ifdef DEBUG
			cout << "\033[0;34mGMENU2X:\033[0m Error loading pxml " << file << ": " << pxmlDoc.getError() << endl;
#endif
			return false;
		}
	} else {
		shorttitle = title;
		exec = path+file;
	}
	
	//Reduce title length to fit the link width
	if (gmenu2x->font->getTextWidth(shorttitle)>gmenu2x->skinConfInt["linkWidth"]) {
		while (gmenu2x->font->getTextWidth(shorttitle+"..")>gmenu2x->skinConfInt["linkWidth"])
			shorttitle = shorttitle.substr(0,shorttitle.length()-1);
		shorttitle += "..";
	}

	ofstream f(linkpath.c_str());
	if (f.is_open()) {
		f << "title=" << shorttitle << endl;
		f << "exec=" << exec << endl;
		if (!description.empty()) f << "description=" << description << endl;
		if (!icon.empty()) f << "icon=" << icon << endl;
		if (!manual.empty()) f << "manual=" << manual << endl;
		if (wrapper) f << "wrapper=true" << endl;
		f.close();

		int isection = find(sections.begin(),sections.end(),section) - sections.begin();
		if (isection>=0 && isection<(int)sections.size()) {
#ifdef DEBUG
			cout << "\033[0;34mGMENU2X:\033[0m Section: " << sections[isection] << "(" << isection << ")" << endl;
#endif

			//senquack - fixing bug where adding a new link does not allow the new link
			//					to be displayed correctly until menu is reloaded:
			//senquack - orig. line:
//			links[isection].push_back( new LinkApp(gmenu2x, linkpath.c_str()) );

			//senquack - new code (pulled from Menu::readLinks())
			LinkApp *link = new LinkApp(gmenu2x, linkpath.c_str());
			link->setSize(gmenu2x->skinConfInt["linkWidth"],gmenu2x->skinConfInt["linkHeight"]);
			if (link->targetExists())
				links[isection].push_back( link );
			else
				free(link);
			
		}
	} else {
#ifdef DEBUG
		cout << "\033[0;34mGMENU2X:\033[0;31m Error while opening the file '" << linkpath << "' for write\033[0m" << endl;
#endif
		return false;
	}

	return true;
}

bool Menu::addSection(string sectionName) {
	//senquack
	//SD stuff - allow storage of most links on SD while gmenu2x is installed on NAND
	string sectiondir = "sections/"+sectionName;

	if (gmenu2x->fwType=="open2x" )
	{
		if ((strcasecmp(sectionName.c_str(), "applications") != 0)
		  		&&	(strcasecmp(sectionName.c_str(), "settings") != 0))
		{
			if (gmenu2x->o2x_store_links_on_SD)
			{
				if (gmenu2x->o2x_SD_read_only)
				{
					cout << "Cannot add section, SD out of room. Free up more space." << endl;
					return false;
				}
				// prepend special open2x links folder path
				sectiondir = gmenu2x->o2x_links_folder + "/" + sectiondir;
			}
		}
		else
		{
			// never allow removal of open2x's "applications" or "settings" sections.
			cout << "Creation of sections with this name not allowed." << endl;
			return false;	
		}
	} 

	if (mkdir(sectiondir.c_str(),0777)==0) {
		sections.push_back(sectionName);
		linklist ll;
		links.push_back(ll);
		sync();
		return true;
	}
	return false;
}

void Menu::deleteSelectedLink() {
#ifdef DEBUG
	cout << "\033[0;34mGMENU2X:\033[0m Deleting link " << selLink()->getTitle() << endl;
#endif
	//senquack
	//SD stuff - allow storage of links on SD while gmenu2x is installed on NAND
	if (selLinkApp()!=NULL)
	{
		// Never allow deletion of links in these sections under open2x:
		if (gmenu2x->fwType == "open2x" && 
				(((strcasecmp(selSection().c_str(), "applications") == 0)			&&
					((strcasecmp((selLink()->getTitle()).c_str(), "e-book") == 0) 			||
					(strcasecmp((selLink()->getTitle()).c_str(), "file manager") == 0) 	||
					(strcasecmp((selLink()->getTitle()).c_str(), "images") == 0) 			||
					(strcasecmp((selLink()->getTitle()).c_str(), "music") == 0) 			||
					(strcasecmp((selLink()->getTitle()).c_str(), "pdf") == 0) 				||
					(strcasecmp((selLink()->getTitle()).c_str(), "video") == 0)))
				||
				(strcasecmp(selSection().c_str(), "settings") == 0)))
		{
			cout << "Removal of this link is not allowed." << endl;
		}
		else
		{
			gmenu2x->ledOn();
			unlink(selLinkApp()->file.c_str());
			gmenu2x->sc.del(selLink()->getIconPath());
			sectionLinks()->erase( sectionLinks()->begin() + selLinkIndex() );
			setLinkIndex(selLinkIndex());
			sync();
			gmenu2x->ledOff();
		}
	}
}

void Menu::deleteSelectedSection() {
#ifdef DEBUG
	cout << "\033[0;34mGMENU2X:\033[0m Deleting section " << selSection() << endl;
#endif
	//senquack - SD stuff
	string surface_path= "sections/" + selSection() + ".png";

	//On Open2X, applications and settings sections are protected and are read-only.
	if (gmenu2x->fwType == "open2x")
	{
		if ((strcasecmp(selSection().c_str(), "applications") == 0) ||
				(strcasecmp(selSection().c_str(), "settings") == 0))
		{
			cout << "Deletion of applications or settings sections disallowed." << endl;
			fflush(NULL);

			return;
		}
		else
		{
			surface_path = gmenu2x->o2x_links_folder + "/" + surface_path;
		}
	}

//	gmenu2x->sc.del("sections/"+selSection()+".png");
	gmenu2x->sc.del(surface_path);
	links.erase( links.begin()+selSectionIndex() );
	sections.erase( sections.begin()+selSectionIndex() );
	setSectionIndex(0); //reload sections
}

bool Menu::linkChangeSection(uint linkIndex, uint oldSectionIndex, uint newSectionIndex) {
	if (oldSectionIndex<sections.size() && newSectionIndex<sections.size() && linkIndex<sectionLinks(oldSectionIndex)->size()) {
		sectionLinks(newSectionIndex)->push_back( sectionLinks(oldSectionIndex)->at(linkIndex) );
		sectionLinks(oldSectionIndex)->erase( sectionLinks(oldSectionIndex)->begin()+linkIndex );
		//Select the same link in the new position
		setSectionIndex(newSectionIndex);
		setLinkIndex(sectionLinks(newSectionIndex)->size()-1);
		return true;
	}
	return false;
}

void Menu::linkLeft() {
	if (iLink%gmenu2x->linkColumns == 0)
		setLinkIndex( sectionLinks()->size()>iLink+gmenu2x->linkColumns-1 ? iLink+gmenu2x->linkColumns-1 : sectionLinks()->size()-1 );
	else
		setLinkIndex(iLink-1);
}

void Menu::linkRight() {
	if (iLink%gmenu2x->linkColumns == (gmenu2x->linkColumns-1) || iLink == (int)sectionLinks()->size()-1)
		setLinkIndex(iLink-iLink%gmenu2x->linkColumns);
	else
		setLinkIndex(iLink+1);
}

void Menu::linkUp() {
	int l = iLink-gmenu2x->linkColumns;
	if (l<0) {
		uint rows = (uint)ceil(sectionLinks()->size()/(double)gmenu2x->linkColumns);
		l = (rows*gmenu2x->linkColumns)+l;
		if (l >= (int)sectionLinks()->size())
			l -= gmenu2x->linkColumns;
	}
	setLinkIndex(l);
}

void Menu::linkDown() {
	uint l = iLink+gmenu2x->linkColumns;
	if (l >= sectionLinks()->size()) {
		uint rows = (uint)ceil(sectionLinks()->size()/(double)gmenu2x->linkColumns);
		uint curCol = (uint)ceil((iLink+1)/(double)gmenu2x->linkColumns);
		if (rows > curCol)
			l = sectionLinks()->size()-1;
		else
			l %= gmenu2x->linkColumns;
	}
	setLinkIndex(l);
}

int Menu::selLinkIndex() {
	return iLink;
}

Link *Menu::selLink() {
	if (sectionLinks()->size()==0) return NULL;
	return sectionLinks()->at(iLink);
}

LinkApp *Menu::selLinkApp() {
	return dynamic_cast<LinkApp*>(selLink());
}

void Menu::setLinkIndex(int i) {
	if (i<0)
		i=sectionLinks()->size()-1;
	else if (i>=(int)sectionLinks()->size())
		i=0;

	if (i>=(int)(iFirstDispRow*gmenu2x->linkColumns+gmenu2x->linkColumns*gmenu2x->linkRows))
		iFirstDispRow = i/gmenu2x->linkColumns-gmenu2x->linkRows+1;
	else if (i<(int)(iFirstDispRow*gmenu2x->linkColumns))
		iFirstDispRow = i/gmenu2x->linkColumns;

	iLink = i;
}

void Menu::readLinks() {
	vector<string> linkfiles;

	iLink = 0;
	iFirstDispRow = 0;

	DIR *dirp;
	struct stat st;
	struct dirent *dptr;
	string filepath;

	for (uint i=0; i<links.size(); i++) {
		links[i].clear();
		linkfiles.clear();

		if ((dirp = opendir(sectionPath(i).c_str())) == NULL) continue;

		while ((dptr = readdir(dirp))) {
			if (dptr->d_name[0]=='.') continue;

			filepath = sectionPath(i)+dptr->d_name;
			
			//senquack - SD stuff
			// Do not process the link on Open2X firmware if it is the LCD settings app in the
			// settings section and we are not an F200
			if (gmenu2x->fwType == "open2x" && !gmenu2x->f200 &&
					filepath.find("settings/tscalibrate") != std::string::npos) continue;

			int statRet = stat(filepath.c_str(), &st);
			if (S_ISDIR(st.st_mode)) continue;
			if (statRet != -1) {
				linkfiles.push_back(filepath);
			}
		}

		sort(linkfiles.begin(), linkfiles.end(),case_less());
		for (uint x=0; x<linkfiles.size(); x++) {
			LinkApp *link = new LinkApp(gmenu2x, linkfiles[x].c_str());
			link->setSize(gmenu2x->skinConfInt["linkWidth"],gmenu2x->skinConfInt["linkHeight"]);
			if (link->targetExists())
				links[i].push_back( link );
			else
				free(link);
		}

		closedir(dirp);
	}
}
