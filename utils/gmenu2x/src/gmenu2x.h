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

#ifndef GMENU2X_H
#define GMENU2X_H

#include <string>
#include <iostream>
//senquack - including this locally since Open2X doesn't have google's hash map in its toolchain
#ifdef OPEN2X
#include "google/dense_hash_map"
#else
#include <google/dense_hash_map>
#endif

#include "surfacecollection.h"
#include "iconbutton.h"
#include "translator.h"
#include "FastDelegate.h"
#include "utilities.h"
#include "touchscreen.h"
#include "inputmanager.h"

//senquack - for new Open2X stick-click emulation
#define OPEN2X_STICK_CLICK_DISABLED		0	// stick-click emulation disabled (default)
#define OPEN2X_STICK_CLICK_DPAD			1	// stick-click emulated by pressing UP+DOWN+LEFT+RIGHT
#define OPEN2X_STICK_CLICK_VOLUPDOWN	2	// stick click emulated by pressing VOLUP+VOLDOWN	
// /dev/GPIO open2x-specific ioctl commands:
#define GP2X_SET_STICK_CLICK_EMULATION_MODE		40 
// senquack - new /dev/GPIO button remapping controllable by these commands:
#define GP2X_REMAP_BUTTON_00  50
#define GP2X_REMAP_BUTTON_01  51
#define GP2X_REMAP_BUTTON_02  52
#define GP2X_REMAP_BUTTON_03  53
#define GP2X_REMAP_BUTTON_04  54
#define GP2X_REMAP_BUTTON_05  55
#define GP2X_REMAP_BUTTON_06  56
#define GP2X_REMAP_BUTTON_07  57
#define GP2X_REMAP_BUTTON_08  58
#define GP2X_REMAP_BUTTON_09  59
#define GP2X_REMAP_BUTTON_10  60
#define GP2X_REMAP_BUTTON_11  61
#define GP2X_REMAP_BUTTON_12  62
#define GP2X_REMAP_BUTTON_13  63
#define GP2X_REMAP_BUTTON_14  64
#define GP2X_REMAP_BUTTON_15  65
#define GP2X_REMAP_BUTTON_16  66
#define GP2X_REMAP_BUTTON_17  67
#define GP2X_REMAP_BUTTON_18  68
#define GP2X_DISABLE_REMAPPING   69
#define GP2X_SET_UPPER_MEMORY_CACHING	80

const int MAX_VOLUME_SCALE_FACTOR = 200;
const int VOLUME_SCALER_MUTE = 0;
const int VOLUME_SCALER_PHONES = 65;
const int VOLUME_SCALER_NORMAL = 100;
const int VOLUME_MODE_MUTE = 0;
const int VOLUME_MODE_PHONES = 1;
const int VOLUME_MODE_NORMAL = 2;
const int BATTERY_READS = 10;

const int LOOP_DELAY=30000;

//senquack
const string OPEN2X_VERSION_FILENAME = "/etc/open2x";

using std::string;
using fastdelegate::FastDelegate0;
using google::dense_hash_map;

typedef FastDelegate0<> MenuAction;
typedef dense_hash_map<string, string, hash<string> > ConfStrHash;
typedef dense_hash_map<string, int, hash<string> > ConfIntHash;
typedef dense_hash_map<string, RGBAColor, hash<string> > ConfRGBAHash;

typedef struct {
	unsigned short batt;
	unsigned short remocon;
} MMSP2ADC;

struct MenuOption {
	string text;
	MenuAction action;
};

class Menu;

class GMenu2X {
private:
	string path; //!< Contains the working directory of GMenu2X
	/*!
	Retrieves the free disk space on the sd
	@return String containing a human readable representation of the free disk space
	*/
	//senquack - moved to utilities.cpp
//	string getDiskFree();
	unsigned short cpuX; //!< Offset for displaying cpu clock information
	unsigned short volumeX; //!< Offset for displaying volume level
	unsigned short manualX; //!< Offset for displaying the manual indicator in the taskbar
	//senquack - added optional display of uptime to help gauge remaining battery life
	unsigned short uptimeX; //!< Offset for displaying uptime
	//senquack - added for display of current volume scaling mode in Open2X:
	unsigned short scalerX;	//!< Offset for displaying current Open2X volume scaling mode

	/*!
	Reads the current battery state and returns a number representing it's level of charge
	@return A number representing battery charge. 0 means fully discharged. 5 means fully charged. 6 represents a gp2x using AC power.
	*/
	unsigned short getBatteryLevel();
	int batteryHandle;
	void browsePath(string path, vector<string>* directories, vector<string>* files);
	/*!
	Starts the scanning of the nand and sd filesystems, searching for gpe and gpu files and creating the links in 2 dedicated sections.
	*/
	void scanner();
	/*!
	Performs the actual scan in the given path and populates the files vector with the results. The creation of the links is not performed here.
	@see scanner
	*/
	void scanPath(string path, vector<string> *files);

	/*!
	Displays a selector and launches the specified executable file
	*/
	void explorer();

	bool inet, //!< Represents the configuration of the basic network services. @see readCommonIni @see usbnet @see samba @see web
		usbnet,
		samba,
		web;
	string ip, defaultgw, lastSelectorDir;
	int lastSelectorElement;
	void readConfig();
	void readConfigOpen2x();
	void readTmp();
	void readCommonIni();
	void writeCommonIni();

	void initServices();
	void initFont();
	void initMenu();

#ifdef TARGET_GP2X
	unsigned long gp2x_mem;
	unsigned short *gp2x_memregs;
	volatile unsigned short *MEM_REG;
	int cx25874; //tv-out
#endif
	void gp2x_tvout_on(bool pal);
	void gp2x_tvout_off();
	void gp2x_init();
	void gp2x_deinit();
	void toggleTvOut();

public:
	GMenu2X(int argc, char *argv[]);
	~GMenu2X();
	void quit();

	/*
	 * Variables needed for elements disposition
	 */
	uint resX, resY, halfX, halfY;
	uint bottomBarIconY, bottomBarTextY, linkColumns, linkRows;

	/*!
	Retrieves the parent directory of GMenu2X.
	This functions is used to initialize the "path" variable.
	@see path
	@return String containing the parent directory
	*/
	string getExePath();

	InputManager input;
	Touchscreen ts;

	//Configuration hashes
	ConfStrHash confStr, skinConfStr;
	ConfIntHash confInt, skinConfInt;
	ConfRGBAHash skinConfColors;

	//Configuration settings
	bool useSelectionPng;
	void setSkin(string skin, bool setWallpaper = true);
	
	//senquack - SD stuff
	string fwType, fwVersion;
	//gp2x type
	bool f200;

	// Open2x settings ---------------------------------------------------------
	bool o2x_usb_net_on_boot, o2x_ftp_on_boot, o2x_telnet_on_boot, 
		  o2x_gp2xjoy_on_boot, o2x_usb_host_on_boot, o2x_usb_hid_on_boot, 
		  o2x_usb_storage_on_boot; 

	//senquack - added some settings: 
	//				1.) o2x_auto_import_links: whether to check
	//						inserted SDs for existence of open2x links folder 
	//						OPEN2X_LINKS_FOLDER.  If it is not found but there is
	//						a folder /mnt/sd/gmenu2x, the links will be copied a new
	//						folder: /mnt/sd/.open2x_gmenu2x, and made compatible with 
	//						open2x's version of gmenu2x (if that step is even necessary)
	//				2.) o2x_use_autorun_on_SD: if autorun.gpu is on SD, run it instead of gmenu2x
	//				3.) o2x_SD_read_only: is one of the following true of the currently
	//						inserted SD?
	//						* SDs using copy protection that	will break if we write 
	//							to it (i.e., Payback) are detected 
	//						* Write-protected SD is inserted 
	//						* SD that has very little space free is inserted
	//				4.) o2x_store_links_on_SD: store most gmenu2x links in special folders on SDs
	//						instead of on NAND all together.
	//senquack SD stuff
	bool o2x_auto_import_links, o2x_use_autorun_on_SD, o2x_SD_read_only, o2x_store_links_on_SD; 
	
	//senquack stick-click emulation stiff
	int o2x_stick_click_mode;	// Can be one of three things:
//OPEN2X_STICK_CLICK_DISABLED		0	// stick-click emulation disabled 
//												//		(default for F100 is DISABLED)
//OPEN2X_STICK_CLICK_DPAD			1	// stick-click emulated by pressing UP+DOWN+LEFT+RIGHT 
//												//		(default for F200 is DPAD)
//OPEN2X_STICK_CLICK_VOLUPDOWN	2	// stick click emulated by pressing VOLUP+VOLDOWN	


	string o2x_links_folder;

	string o2x_usb_net_ip;

	int o2x_volumeMode, o2x_savedVolumeMode;		//	just use the const int scale values at top of source

	//  Volume scaling values to store from config files
	int o2x_volumeScalerPhones;
	int o2x_volumeScalerNormal;
	//--------------------------------------------------------------------------

	SurfaceCollection sc;
	Translator tr;
	Surface *s, *bg;
	ASFont *font;

	//Status functions
	int main();
	void options();
	void settingsOpen2x();
	void skinMenu();
	void activateSdUsb();
	void activateNandUsb();
	void activateRootUsb();
	
	//senquack - added new open2x functions:
	void unmountSD();
	void restoreO2XAppSection();
	void aboutOpen2X();

	void about();
	void viewLog();
	void contextMenu();
	void changeWallpaper();
	void saveScreenshot();

	void applyRamTimings();
	void applyDefaultTimings();

	void setClock(unsigned mhz);
	void setGamma(int gamma);

	void setVolume(int vol);
	int getVolume();
	void setVolumeScaler(int scaler);
	int getVolumeScaler();

	//senquack - new optional emulation of stick-click for DPAD-modded F100s and F200s
	void setStickClickEmulation(int mode);

	//senquack - new Open2X support for button remapping:
	void remapGpioButton(int button, int remapped_to);
	void disableGpioRemapping(void);

	// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
	// 	no longer necessary:
	void setUpperMemoryCaching(int caching_enabled);


	void setInputSpeed();

	void writeConfig();
	void writeConfigOpen2x();
	void writeSkinConfig();
	void writeTmp(int selelem=-1, string selectordir="");

	void ledOn();
	void ledOff();

	void addLink();
	void editLink();
	void deleteLink();
	void addSection();
	void renameSection();
	void deleteSection();

	void initBG();
	int drawButton(IconButton *btn, int x=5, int y=-10);
	int drawButton(Surface *s, string btn, string text, int x=5, int y=-10);
	int drawButtonRight(Surface *s, string btn, string text, int x=5, int y=-10);
	void drawScrollBar(uint pagesize, uint totalsize, uint pagepos, uint top, uint height);

	void drawTitleIcon(string icon, bool skinRes=true, Surface *s=NULL);
	void writeTitle(string title, Surface *s=NULL);
	void writeSubTitle(string subtitle, Surface *s=NULL);
	void drawTopBar(Surface *s=NULL);
	void drawBottomBar(Surface *s=NULL);

	Menu* menu;
};

#endif
