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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include "linkapp.h"
#include "menu.h"
#include "selector.h"
#include "textmanualdialog.h"

using namespace std;

LinkApp::LinkApp(GMenu2X *gmenu2x, const char* linkfile)
	: Link(gmenu2x) {
	this->gmenu2x = gmenu2x;
	manual = "";
	file = linkfile;
	wrapper = false;
	dontleave = false;
	setClock(200);
	//	senquack - default volume is now 67 (that has always been the GP2X's default)  and the
	//		volume always gets set before launching a link now (so that one program cannot affect
	//		another's volume)
//	setVolume(-1);
	setVolume(67);
	//G
	setGamma(0);
	selectordir = "";
	selectorfilter = "";
	icon = iconPath = "";
	selectorbrowser = false;
	useRamTimings = false;

	//senquack - new open2x support for /dev/GPIO button remapping:
	o2x_gpio_remapping = false;
	int i;
	for (i = 18; i >= 0; i--)
	{
		o2x_gpio_mapping[i] = i;
	}

	// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
	// 	no longer necessary:
	o2x_upper_memory_cached = false;

	//senquack - new Open2X support for TV tweaking daemon:
	o2x_tv_daemon_enabled = true;
	o2x_tv_daemon_tweak_yuv = false;
	o2x_tv_daemon_scaling_tweak = true;
	// copy in offsets and scale percents from gmenu2x's current tvout settings:
	o2x_tv_daemon_xoffset = (gmenu2x->confStr["tvoutEncoding"] == "PAL") ? 
		gmenu2x->confInt["tvoutXOffsetPAL"] 										:
		gmenu2x->confInt["tvoutXOffsetNTSC"];
	o2x_tv_daemon_yoffset = (gmenu2x->confStr["tvoutEncoding"] == "PAL") ?
		gmenu2x->confInt["tvoutYOffsetPAL"]											:
		gmenu2x->confInt["tvoutYOffsetNTSC"];
	o2x_tv_daemon_xscale = gmenu2x->confInt["tvoutXScale"];
	o2x_tv_daemon_yscale = gmenu2x->confInt["tvoutYScale"];
	o2x_tv_daemon_vxscale = 100;
	o2x_tv_daemon_vyscale = 100;
	// Delay before first tweak:
	o2x_tv_daemon_first_delay = TV_DAEMON_DEFAULT_FIRST_DELAY;	
	// Delay inbetween subsequent tweaks:
	o2x_tv_daemon_delay = TV_DAEMON_DEFAULT_DELAY;	
	// Tweak just once after pausing for the delay above and terminate?
	o2x_tv_daemon_tweak_only_once = false;		
	//	Deprecated:
//	// Needed for some things like gngeo:
//	o2x_tv_daemon_force_720_pitch = false;	
	// Special fix for some games that used a buggy SDL, like Tilematch and some
	// of Ruckage's earlier games:
//	o2x_tv_daemon_stubborn_fix = false;
	// Rage2X segfaults when started in TVout mode, but will allow you to 
	// turn TV mode on from inside it.  This option lets you launch the tv tweaker
	// even when TV mode isn't enabled (it actually works)
	o2x_tv_daemon_always_started = false;
	// Not normally needed, since tv mode should already be enabled:
	o2x_tv_daemon_enable_tv_mode = false;

	// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
	// 	a USB gamepad.  It should be configurable because it will greatly interfere
	// 	with apps like Picodrive that already know how to use USB joysticks.
	o2x_link_uses_joy2xd = true;

	// When this is true, a hack is implemented that presents a dummy js0 
	// joystick to all applications, and the first USB joystick is thus
	// not seen at all by any apps.  This is very useful for some apps like
	// PocketSNES that don't have very flexible button remapping and won't
	// let player 2 use the second joystick otherwise.
	o2x_joy2xd_hides_js0 = true;


	string line;
	ifstream infile (linkfile, ios_base::in);
	while (getline(infile, line, '\n')) {
		line = trim(line);
		if (line=="") continue;
		if (line[0]=='#') continue;

		string::size_type position = line.find("=");
		string name = trim(line.substr(0,position));
		string value = trim(line.substr(position+1));
		if (name == "title") {
			title = value;
		} else if (name == "description") {
			description = value;
		} else if (name == "icon") {
			setIcon(value);
		} else if (name == "exec") {
			exec = value;
		} else if (name == "params") {
			params = value;
		} else if (name == "workdir") {
			workdir = value;
		} else if (name == "manual") {
			manual = value;
		} else if (name == "wrapper") {
			if (value=="true") wrapper = true;
		} else if (name == "dontleave") {
			if (value=="true") dontleave = true;
		} else if (name == "clock") {
			setClock( atoi(value.c_str()) );
		//G
		} else if (name == "gamma") {
			setGamma( atoi(value.c_str()) );
		} else if (name == "volume") {
			setVolume( atoi(value.c_str()) );
		} else if (name == "selectordir") {
			setSelectorDir( value );
		} else if (name == "selectorbrowser") {
			if (value=="true") selectorbrowser = true;
		} else if (name == "useramtimings") {
			if (value=="true") useRamTimings = true;
		} else if (name == "selectorfilter") {
			setSelectorFilter( value );
		} else if (name == "selectorscreens") {
			setSelectorScreens( value );
		} else if (name == "selectoraliases") {
			setAliasFile( value );
		//senquack - new open2x support for /dev/GPIO button remapping:
		} else if (name == "gpio_button_00") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[0] = i;
		} else if (name == "gpio_button_01") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[1] = i;
		} else if (name == "gpio_button_02") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[2] = i;
		} else if (name == "gpio_button_03") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[3] = i;
		} else if (name == "gpio_button_04") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[4] = i;
		} else if (name == "gpio_button_05") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[5] = i;
		} else if (name == "gpio_button_06") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[6] = i;
		} else if (name == "gpio_button_07") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[7] = i;
		} else if (name == "gpio_button_08") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[8] = i;
		} else if (name == "gpio_button_09") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[9] = i;
		} else if (name == "gpio_button_10") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[10] = i;
		} else if (name == "gpio_button_11") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[11] = i;
		} else if (name == "gpio_button_12") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[12] = i;
		} else if (name == "gpio_button_13") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[13] = i;
		} else if (name == "gpio_button_14") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[14] = i;
		} else if (name == "gpio_button_15") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[15] = i;
		} else if (name == "gpio_button_16") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[16] = i;
		} else if (name == "gpio_button_17") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[17] = i;
		} else if (name == "gpio_button_18") {
			i = atoi(value.c_str());
			if (i >= -1 && i <= 18)
				o2x_gpio_mapping[18] = i;
		} else if (name == "gpio_remapping") {
			if (value=="true") o2x_gpio_remapping = true;
		// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
		// 	no longer necessary:
		} else if (name == "upper_memory_cached") {
			if (value=="true") o2x_upper_memory_cached = true;
		//senquack - new Open2X support for TV tweaking daemon:
		} else if (name == "tv_daemon_enabled") {
			if (value=="false") o2x_tv_daemon_enabled = false;
		} else if (name == "tv_daemon_tweak_yuv") {
			if (value=="true") o2x_tv_daemon_tweak_yuv = true;
		} else if (name == "tv_daemon_scaling_tweak") {
			if (value=="false") o2x_tv_daemon_scaling_tweak = false;
		} else if (name == "tv_daemon_xoffset") {
			i = atoi(value.c_str());
			if (i >= TV_MIN_XOFFSET && i <= TV_MAX_XOFFSET)
				o2x_tv_daemon_xoffset = i;
		} else if (name == "tv_daemon_yoffset") {
			i = atoi(value.c_str());
			if (i >= TV_MIN_YOFFSET && i <= TV_MAX_YOFFSET)
				o2x_tv_daemon_yoffset = i;
		} else if (name == "tv_daemon_xscale") {
			i = atoi(value.c_str());
			if (i >= TV_MIN_XSCALE && i <= TV_MAX_XSCALE)
				o2x_tv_daemon_xscale = i;
		} else if (name == "tv_daemon_yscale") {
			i = atoi(value.c_str());
			if (i >= TV_MIN_YSCALE && i <= TV_MAX_YSCALE)
				o2x_tv_daemon_yscale = i;
		} else if (name == "tv_daemon_vxscale") {
			i = atoi(value.c_str());
			if (i >= TV_MIN_VXSCALE && i <= TV_MAX_VXSCALE)
				o2x_tv_daemon_vxscale = i;
		} else if (name == "tv_daemon_vyscale") {
			i = atoi(value.c_str());
			if (i >= TV_MIN_VYSCALE && i <= TV_MAX_VYSCALE)
				o2x_tv_daemon_vyscale = i;
		} else if (name == "tv_daemon_first_delay") {
			i = atoi(value.c_str());
			if (i >= TV_DAEMON_MIN_FIRST_DELAY && i <= TV_DAEMON_MAX_FIRST_DELAY)
				o2x_tv_daemon_first_delay = i;
		} else if (name == "tv_daemon_delay") {
			i = atoi(value.c_str());
			if (i >= TV_DAEMON_MIN_DELAY && i <= TV_DAEMON_MAX_DELAY)
				o2x_tv_daemon_delay = i;
		} else if (name == "tv_daemon_tweak_only_once") {
			if (value=="true") o2x_tv_daemon_tweak_only_once = true;
			//		Deprecated:
//		} else if (name == "tv_daemon_force_720_pitch") {
//			if (value=="true") o2x_tv_daemon_force_720_pitch = true;
//		} else if (name == "tv_daemon_stubborn_fix") {
//			if (value=="true") o2x_tv_daemon_stubborn_fix = true;
		} else if (name == "tv_daemon_always_started") {
			if (value=="true") o2x_tv_daemon_always_started = true;
		} else if (name == "tv_daemon_enable_tv_mode") {
			if (value=="true") o2x_tv_daemon_enable_tv_mode = true;
		} else if (name == "link_uses_joy2xd") {
			if (value=="false") o2x_link_uses_joy2xd = false;
		} else if (name == "joy2xd_hides_js0") {
			if (value=="false") o2x_joy2xd_hides_js0 = false;
		} else {
			cout << "Unrecognized option: " << name << endl;
			break;
		}
	}
	infile.close();

	if (iconPath.empty()) searchIcon();

	edited = false;
}

string LinkApp::searchIcon() {
	string execicon = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) execicon = exec.substr(0,pos);
	execicon += ".png";
	string exectitle = execicon;
	pos = execicon.rfind("/");
	if (pos != string::npos)
		string exectitle = execicon.substr(pos+1,execicon.length());

	if (!gmenu2x->sc.getSkinFilePath("icons/"+exectitle).empty())
		iconPath = gmenu2x->sc.getSkinFilePath("icons/"+exectitle);
	else if (fileExists(execicon))
		iconPath = execicon;
	else
		iconPath = gmenu2x->sc.getSkinFilePath("icons/generic.png");

	return iconPath;
}

int LinkApp::clock() {
	return iclock;
}

string LinkApp::clockStr(int maxClock) {
	if (iclock>maxClock) setClock(maxClock);
	return sclock;
}

void LinkApp::setClock(int mhz) {
	iclock = constrain(mhz,50,325);
	stringstream ss;
	sclock = "";
	ss << iclock << "Mhz";
	ss >> sclock;

	edited = true;
}

int LinkApp::volume() {
	return ivolume;
}

string LinkApp::volumeStr() {
	return svolume;
}

void LinkApp::setVolume(int vol) {
	ivolume = constrain(vol,-1,100);
	stringstream ss;
	svolume = "";
	if (ivolume<0)
		ss << gmenu2x->confInt["globalVolume"];
	else
		ss << ivolume;
	ss >> svolume;

	edited = true;
}

//G
int LinkApp::gamma() {
	return igamma;
}

string LinkApp::gammaStr() {
	return sgamma;
}

void LinkApp::setGamma(int gamma) {
	igamma = constrain(gamma,0,100);
	stringstream ss;
	sgamma = "";
	ss << igamma;
	ss >> sgamma;

	edited = true;
}
// /G

bool LinkApp::targetExists() {
#ifndef TARGET_GP2X
	return true; //For displaying elements during testing on pc
#endif

	string target = exec;
	if (!exec.empty() && exec[0]!='/' && !workdir.empty())
		target = workdir + "/" + exec;

	return fileExists(target);
}

bool LinkApp::save() {
	if (!edited) return false;

	ofstream f(file.c_str());
	if (f.is_open()) {
		if (title!=""          ) f << "title="           << title           << endl;
		if (description!=""    ) f << "description="     << description     << endl;
		if (icon!=""           ) f << "icon="            << icon            << endl;
		if (exec!=""           ) f << "exec="            << exec            << endl;
		if (params!=""         ) f << "params="          << params          << endl;
		if (workdir!=""        ) f << "workdir="         << workdir         << endl;
		if (manual!=""         ) f << "manual="          << manual          << endl;
		if (iclock!=0          ) f << "clock="           << iclock          << endl;
		if (useRamTimings      ) f << "useramtimings=true"                  << endl;
		if (ivolume>0          ) f << "volume="          << ivolume         << endl;
		//G
		if (igamma!=0          ) f << "gamma="           << igamma          << endl;
		if (selectordir!=""    ) f << "selectordir="     << selectordir     << endl;
		if (selectorbrowser    ) f << "selectorbrowser=true"                << endl;
		if (selectorfilter!="" ) f << "selectorfilter="  << selectorfilter  << endl;
		if (selectorscreens!="") f << "selectorscreens=" << selectorscreens << endl;
		if (aliasfile!=""      ) f << "selectoraliases=" << aliasfile       << endl;
		if (wrapper            ) f << "wrapper=true"                        << endl;
		if (dontleave          ) f << "dontleave=true"                      << endl;
		if (gmenu2x->fwType == "open2x")
		{
			//senquack - new open2x support for /dev/GPIO button remapping:
			if (o2x_gpio_remapping ) f << "gpio_remapping=true" 						<< endl;
			int i;
			for(i=0; i<=18; i++)
			{
				f << "gpio_button_" << setw(2) << setfill('0') << right << i << "=" << 
					o2x_gpio_mapping[i] << endl;
			}

			// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
			// 	no longer necessary:
			if (o2x_upper_memory_cached) f << "upper_memory_cached=true"			<< endl;

			//senquack - new Open2X support for TV tweaking daemon:
			if (!o2x_tv_daemon_enabled) f << "tv_daemon_enabled=false" << endl;
			if (o2x_tv_daemon_tweak_yuv) f << "tv_daemon_tweak_yuv=true" << endl;
			if (!o2x_tv_daemon_scaling_tweak) f << "tv_daemon_scaling_tweak=false" << endl;
			f << "tv_daemon_xoffset=" << o2x_tv_daemon_xoffset << endl;
			f << "tv_daemon_yoffset=" << o2x_tv_daemon_yoffset << endl;
			f << "tv_daemon_xscale=" << o2x_tv_daemon_xscale << endl;
			f << "tv_daemon_yscale=" << o2x_tv_daemon_yscale << endl;
			f << "tv_daemon_vxscale=" << o2x_tv_daemon_vxscale << endl;
			f << "tv_daemon_vyscale=" << o2x_tv_daemon_vyscale << endl;
			f << "tv_daemon_first_delay=" << o2x_tv_daemon_first_delay << endl;
			f << "tv_daemon_delay=" << o2x_tv_daemon_delay << endl;
			if (o2x_tv_daemon_tweak_only_once) f << "tv_daemon_tweak_only_once=true" << endl;
			//			Deprecated:
//			if (o2x_tv_daemon_force_720_pitch) f << "tv_daemon_force_720_pitch=true" << endl;
//			if (o2x_tv_daemon_stubborn_fix) f << "tv_daemon_stubborn_fix=true" << endl;
			if (o2x_tv_daemon_always_started) f << "tv_daemon_always_started=true" << endl;
			if (o2x_tv_daemon_enable_tv_mode) f << "tv_daemon_enable_tv_mode=true" << endl;

			if (!o2x_link_uses_joy2xd) f << "link_uses_joy2xd=false" << endl;
			if (!o2x_joy2xd_hides_js0) f << "joy2xd_hides_js0=false" << endl;
		}
		
		f.close();
		return true;
	} else
		cout << "\033[0;34mGMENU2X:\033[0;31m Error while opening the file '" << file << "' for write\033[0m" << endl;
	return false;
}

void LinkApp::drawRun() {
	//Darkened background
	gmenu2x->s->box(0, 0, gmenu2x->resX, gmenu2x->resY, 0,0,0,150);

	string text = gmenu2x->tr.translate("Launching $1",getTitle().c_str(),NULL);
	int textW = gmenu2x->font->getTextWidth(text);
	int boxW = 62+textW;
	int halfBoxW = boxW/2;

	//outer box
	gmenu2x->s->box(gmenu2x->halfX-2-halfBoxW, gmenu2x->halfY-23, halfBoxW*2+5, 47, gmenu2x->skinConfColors["messageBoxBg"]);
	//inner rectangle
	gmenu2x->s->rectangle(gmenu2x->halfX-halfBoxW, gmenu2x->halfY-21, boxW, 42, gmenu2x->skinConfColors["messageBoxBorder"]);

	int x = gmenu2x->halfX+10-halfBoxW;
	/*if (getIcon()!="")
		gmenu2x->sc[getIcon()]->blit(gmenu2x->s,x,104);
	else
		gmenu2x->sc["icons/generic.png"]->blit(gmenu2x->s,x,104);*/
	gmenu2x->sc[getIconPath()]->blit(gmenu2x->s,x,gmenu2x->halfY-16);
	gmenu2x->s->write( gmenu2x->font, text, x+42, gmenu2x->halfY+1, SFontHAlignLeft, SFontVAlignMiddle );
	gmenu2x->s->flip();
}

void LinkApp::run() {
	if (selectordir!="")
		selector();
	else
		launch();
}

void LinkApp::showManual() {
	if (manual=="" || !fileExists(manual)) return;

	// Png manuals
	string ext8 = manual.substr(manual.size()-8,8);
	if (ext8==".man.png" || ext8==".man.bmp" || ext8==".man.jpg" || manual.substr(manual.size()-9,9)==".man.jpeg") {
		//Raise the clock to speed-up the loading of the manual
		gmenu2x->setClock(200);

		Surface pngman(manual);
		Surface bg(gmenu2x->confStr["wallpaper"],false);
		stringstream ss;
		string pageStatus;

		bool close = false, repaint = true;
		int page=0, pagecount=pngman.raw->w/320;

		ss << pagecount;
		string spagecount;
		ss >> spagecount;

		//Lower the clock
		gmenu2x->setClock(gmenu2x->confInt["menuClock"]);

		while (!close) {
			if (repaint) {
				bg.blit(gmenu2x->s, 0, 0);
				pngman.blit(gmenu2x->s, -page*320, 0);

				gmenu2x->drawBottomBar();
				gmenu2x->drawButton(gmenu2x->s, "x", gmenu2x->tr["Exit"],
				gmenu2x->drawButton(gmenu2x->s, "right", gmenu2x->tr["Change page"],
				gmenu2x->drawButton(gmenu2x->s, "left", "", 5)-10));

				ss.clear();
				ss << page+1;
				ss >> pageStatus;
				pageStatus = gmenu2x->tr["Page"]+": "+pageStatus+"/"+spagecount;
				gmenu2x->s->write(gmenu2x->font, pageStatus, 310, 230, SFontHAlignRight, SFontVAlignMiddle);

				gmenu2x->s->flip();
				repaint = false;
			}

			gmenu2x->input.update();
			if ( gmenu2x->input[ACTION_Y] || gmenu2x->input[ACTION_X] || gmenu2x->input[ACTION_START] ) close = true;
			if ( gmenu2x->input[ACTION_LEFT] && page>0 ) { page--; repaint=true; }
			if ( gmenu2x->input[ACTION_RIGHT] && page<pagecount-1 ) { page++; repaint=true; }
		}
		return;
	}

	// Txt manuals
	if (manual.substr(manual.size()-8,8)==".man.txt") {
		vector<string> txtman;

		string line;
		ifstream infile(manual.c_str(), ios_base::in);
		if (infile.is_open()) {
			gmenu2x->setClock(200);
			while (getline(infile, line, '\n')) txtman.push_back(line);
			infile.close();

			TextManualDialog tmd(gmenu2x, getTitle(), getIconPath(), &txtman);
			gmenu2x->setClock(gmenu2x->confInt["menuClock"]);
			tmd.exec();
		}

		return;
	}

	//Readmes
	vector<string> readme;

	string line;
	ifstream infile(manual.c_str(), ios_base::in);
	if (infile.is_open()) {
		gmenu2x->setClock(200);
		while (getline(infile, line, '\n')) readme.push_back(line);
		infile.close();

		TextDialog td(gmenu2x, getTitle(), "ReadMe", getIconPath(), &readme);
		gmenu2x->setClock(gmenu2x->confInt["menuClock"]);
		td.exec();
	}
}

void LinkApp::selector(int startSelection, string selectorDir) {
	//Run selector interface
	Selector sel(gmenu2x, this, selectorDir);
	int selection = sel.exec(startSelection);
	if (selection!=-1) {
		gmenu2x->writeTmp(selection,sel.dir);
		launch(sel.file, sel.dir);
	}
}

void LinkApp::launch(string selectedFile, string selectedDir) {
	drawRun();
	save();
#ifndef TARGET_GP2X
	//delay for testing
	SDL_Delay(1000);
#endif

	//Set correct working directory
	string wd = workdir;
	if (wd=="") {
		string::size_type pos = exec.rfind("/");
		if (pos!=string::npos)
			wd = exec.substr(0,pos);
	}
	if (!wd.empty()) {
		if (wd[0]!='/') wd = gmenu2x->getExePath() + wd;
		chdir(wd.c_str());
	}

	//selectedFile
	if (selectedFile!="") {
		string selectedFileExtension;
		string::size_type i = selectedFile.rfind(".");
		if (i != string::npos) {
			selectedFileExtension = selectedFile.substr(i,selectedFile.length());
			selectedFile = selectedFile.substr(0,i);
		}

		if (selectedDir=="")
			selectedDir = getSelectorDir();
		if (params=="") {
			params = cmdclean(selectedDir+selectedFile+selectedFileExtension);
		} else {
			string origParams = params;
			params = strreplace(params,"[selFullPath]",cmdclean(selectedDir+selectedFile+selectedFileExtension));
			params = strreplace(params,"[selPath]",cmdclean(selectedDir));
			params = strreplace(params,"[selFile]",cmdclean(selectedFile));
			params = strreplace(params,"[selExt]",cmdclean(selectedFileExtension));
			if (params == origParams) params += " " + cmdclean(selectedDir+selectedFile+selectedFileExtension);
		}
	}

	//senquack - because we have new option, alwaysUseFastTimings, check for it and never
	//				change the timings if it is set.  (Fast ram timings will be applied on gmenu2x's
	//				startup and never changed)
	if (!gmenu2x->confInt["alwaysUseFastTimings"] && useRamTimings)
		gmenu2x->applyRamTimings();
	if (volume()>=0)
		gmenu2x->setVolume(volume());

#ifdef DEBUG
	cout << "\033[0;34mGMENU2X:\033[0m Executing '" << title << "' (" << exec << " " << params << ")" << endl;
#endif

	//check if we have to quit
	string command = cmdclean(exec);

	// Check to see if permissions are desirable
	struct stat fstat;
	if( stat( command.c_str(), &fstat ) == 0 ) {
		struct stat newstat = fstat;
		if( S_IRUSR != ( fstat.st_mode & S_IRUSR ) )
			newstat.st_mode |= S_IRUSR;
		if( S_IXUSR != ( fstat.st_mode & S_IXUSR ) )
			newstat.st_mode |= S_IXUSR;
		if( fstat.st_mode != newstat.st_mode )
			chmod( command.c_str(), newstat.st_mode );
	} // else, well.. we are no worse off :)

	if (gmenu2x->fwType == "open2x")
	{
		//senquack - new open2x support for /dev/GPIO button remapping:
		if (getGpioRemappingStatus())
		{
			int i;
			for (i = 0; i <= 18; i++)
			{
				gmenu2x->remapGpioButton(i,getGpioMapping(i));
			}
		}

		//senquack - new Open2X support for TV tweaking daemon:
		if ((gmenu2x->gp2x_tv_mode || getTVDaemonAlwaysStarted()) && getTVDaemonStatus())
		{
//			// TV daemon enabled, assemble the parameters 
////			string tv_cmdline = OPEN2X_TV_DAEMON_FULLPATH;
////			if (gmenu2x->confStr["tvoutEncoding"] == "PAL")
////				tv_cmdline += " -p";
////			if (getTVDaemonEnableTVMode())
////				tv_cmdline += " -e";
////			if (getTVDaemonForce720Pitch())
////				tv_cmdline += " -f720";
////			if (getTVDaemonTweakOnlyOnce())
////				tv_cmdline += " -t";
////			tv_cmdline += " -x" + getTVDaemonXOffset();
////			tv_cmdline += " -y" + getTVDaemonYOffset();
////			tv_cmdline += " -X" + getTVDaemonXScale();
////			tv_cmdline += " -Y" + getTVDaemonYScale();
//			tv_cmdline += " -d" + (string)getTVDaemonDelay();
//			tv_cmdline += "&";
//			fflush(NULL);
//			cout << "Starting Open2X TV Daemon with following command line: " << endl << tv_cmdline << endl;
//			cout << endl << endl << endl;
//			
//			fflush(NULL);
//
//			printf("tvdaemondelay(): %d\n", getTVDaemonDelay());
//			fflush(NULL);
//
//			
//			system(tv_cmdline.c_str());

			// TV daemon enabled, assemble the parameters:
			stringstream tv_cmdline_ss;
			string tmpstr = "";
			string tv_cmdline = OPEN2X_TV_DAEMON_FULLPATH;
			if (gmenu2x->confStr["tvoutEncoding"] == "PAL")
				tv_cmdline += " -p";
			if (gmenu2x->confInt["tvoutPalOverscanFix"])
				tv_cmdline += " -P";
			if (getTVDaemonEnableTVMode())
				tv_cmdline += " -e";
			//			Deprecated:
//			if (getTVDaemonForce720Pitch())
//				tv_cmdline += " -f720";
//			if (getTVDaemonStubbornFix())
//				tv_cmdline += " -s";
			if (getTVDaemonTweakOnlyOnce())
				tv_cmdline += " -t";
			if (getTVDaemonTweakYuv())
				tv_cmdline += " -V";
			if (!getTVDaemonScalingTweak())
				tv_cmdline += " -S";
			tv_cmdline += " -x";
			tv_cmdline_ss.clear();
			tv_cmdline_ss << getTVDaemonXOffset();
			tv_cmdline_ss >> tmpstr;
			tv_cmdline += tmpstr;
			tv_cmdline += " -y";
			tv_cmdline_ss.clear();
			tv_cmdline_ss << getTVDaemonYOffset();
			tv_cmdline_ss >> tmpstr;
			tv_cmdline += tmpstr;
			tv_cmdline += " -X";
			tv_cmdline_ss.clear();
		  	tv_cmdline_ss << getTVDaemonXScale();
			tv_cmdline_ss >> tmpstr;
			tv_cmdline += tmpstr;
			tv_cmdline += " -Y";
			tv_cmdline_ss.clear();
			tv_cmdline_ss << getTVDaemonYScale();
			tv_cmdline_ss >> tmpstr;
			tv_cmdline += tmpstr;
			tv_cmdline += " -vx";
			tv_cmdline_ss.clear();
		  	tv_cmdline_ss << getTVDaemonVXScale();
			tv_cmdline_ss >> tmpstr;
			tv_cmdline += tmpstr;
			tv_cmdline += " -vy";
			tv_cmdline_ss.clear();
			tv_cmdline_ss << getTVDaemonVYScale();
			tv_cmdline_ss >> tmpstr;
			tv_cmdline += tmpstr;
			tv_cmdline += " -d";
			tv_cmdline_ss.clear();
			tv_cmdline_ss << getTVDaemonDelay();
			tv_cmdline_ss >> tmpstr;
			tv_cmdline += tmpstr;
			tv_cmdline += " -D";
			tv_cmdline_ss.clear();
			tv_cmdline_ss << getTVDaemonFirstDelay();
			tv_cmdline_ss >> tmpstr;
			tv_cmdline += tmpstr;
			tv_cmdline += "&";

			fflush(NULL);
			cout << "Starting Open2X TV Daemon with following command line: " << endl << tv_cmdline << endl;
			cout << endl << endl << endl;
			
			fflush(NULL);

			system(tv_cmdline.c_str());

		}

		// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
		// 	no longer necessary:
		if (getUpperMemoryCachingStatus())
			gmenu2x->setUpperMemoryCaching(1);

		// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
		// 	a USB gamepad.  It should be configurable because it will greatly interfere
		// 	with apps like Picodrive that already know how to use USB joysticks.
		if (!getJoy2xdStatus() && gmenu2x->o2x_gmenu2x_starts_joy2xd)
		{
			gmenu2x->deactivateJoy2xd();
		}

		if (getJoy2xdStatus() && gmenu2x->o2x_gmenu2x_starts_joy2xd && getJoy2xdHidesJs0())
		{
			string js_cmdline;
			js_cmdline = "/usr/sbin/joy2xd_make_dummy_js0";
			system(js_cmdline.c_str());
		}
	}
			
	if (params!="") command += " " + params;
	if (gmenu2x->confInt["outputLogs"]) command += " &> " + cmdclean(gmenu2x->getExePath()) + "/log.txt";
	if (wrapper) command += "; sync & cd "+cmdclean(gmenu2x->getExePath())+"; exec ./gmenu2x";
	if (dontleave) {
		system(command.c_str());
		//senquack - new open2x support for /dev/GPIO button remapping:
		gmenu2x->disableGpioRemapping();
	} else {
		if (gmenu2x->confInt["saveSelection"] && (gmenu2x->confInt["section"]!=gmenu2x->menu->selSectionIndex() || gmenu2x->confInt["link"]!=gmenu2x->menu->selLinkIndex()))
			gmenu2x->writeConfig();
		if (gmenu2x->fwType == "open2x" && gmenu2x->o2x_savedVolumeMode != gmenu2x->o2x_volumeMode)
			gmenu2x->writeConfigOpen2x();
		if (selectedFile=="")
			gmenu2x->writeTmp();
		gmenu2x->quit();
		if (clock()!=gmenu2x->confInt["menuClock"])
			gmenu2x->setClock(clock());
		if (gamma()!=0 && gamma()!=gmenu2x->confInt["gamma"])
			gmenu2x->setGamma(gamma());
		execlp("/bin/sh","/bin/sh","-c",command.c_str(),NULL);
		//if execution continues then something went wrong and as we already called SDL_Quit we cannot continue
		//try relaunching gmenu2x:
		chdir(gmenu2x->getExePath().c_str());
		execlp("./gmenu2x", "./gmenu2x", NULL);
	}


	chdir(gmenu2x->getExePath().c_str());
}

string LinkApp::getExec() {
	return exec;
}

void LinkApp::setExec(string exec) {
	this->exec = exec;
	edited = true;
}

string LinkApp::getParams() {
	return params;
}

void LinkApp::setParams(string params) {
	this->params = params;
	edited = true;
}

string LinkApp::getWorkdir() {
	return workdir;
}

void LinkApp::setWorkdir(string workdir) {
	this->workdir = workdir;
	edited = true;
}

string LinkApp::getManual() {
	return manual;
}

void LinkApp::setManual(string manual) {
	this->manual = manual;
	edited = true;
}

string LinkApp::getSelectorDir() {
	return selectordir;
}

void LinkApp::setSelectorDir(string selectordir) {
	if (selectordir!="" && selectordir[selectordir.length()-1]!='/') selectordir += "/";
	this->selectordir = selectordir;
	edited = true;
}

bool LinkApp::getSelectorBrowser() {
	return selectorbrowser;
}

void LinkApp::setSelectorBrowser(bool value) {
	selectorbrowser = value;
	edited = true;
}

bool LinkApp::getUseRamTimings() {
	return useRamTimings;
}

void LinkApp::setUseRamTimings(bool value) {
	useRamTimings = value;
	edited = true;
}

string LinkApp::getSelectorFilter() {
	return selectorfilter;
}

void LinkApp::setSelectorFilter(string selectorfilter) {
	this->selectorfilter = selectorfilter;
	edited = true;
}

string LinkApp::getSelectorScreens() {
	return selectorscreens;
}

void LinkApp::setSelectorScreens(string selectorscreens) {
	this->selectorscreens = selectorscreens;
	edited = true;
}

string LinkApp::getAliasFile() {
	return aliasfile;
}

void LinkApp::setAliasFile(string aliasfile) {
	if (fileExists(aliasfile)) {
		this->aliasfile = aliasfile;
		edited = true;
	}
}

//senquack - new open2x gpio remapping support:
int LinkApp::getGpioMapping(int button) {
	return o2x_gpio_mapping[button];
}

void LinkApp::setGpioMapping(int button, int mapped_to) {
	o2x_gpio_mapping[button] = mapped_to;
}

bool LinkApp::getGpioRemappingStatus(void) {
	return o2x_gpio_remapping;
}

void LinkApp::setGpioRemappingStatus(bool newstatus) {
	o2x_gpio_remapping = newstatus;
}

// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
// 	no longer necessary:
void LinkApp::setUpperMemoryCachingStatus(int upper_memory_cached) {
	o2x_upper_memory_cached = upper_memory_cached;
}

//senquack - new Open2X support for TV tweaking daemon:
bool LinkApp::getUpperMemoryCachingStatus(void) {
	return o2x_upper_memory_cached;
}

bool LinkApp::getTVDaemonStatus(void) {
	return o2x_tv_daemon_enabled;
}

void LinkApp::setTVDaemonStatus(bool daemon_enabled) {
	o2x_tv_daemon_enabled = daemon_enabled;
}

bool LinkApp::getTVDaemonTweakYuv(void) {
	return o2x_tv_daemon_tweak_yuv;
}

void LinkApp::setTVDaemonTweakYuv(bool tweak_yuv) {
	o2x_tv_daemon_tweak_yuv = tweak_yuv;
}

bool LinkApp::getTVDaemonScalingTweak(void) {
	return o2x_tv_daemon_scaling_tweak;
}

void LinkApp::setTVDaemonScalingTweak(bool scaling_tweak) {
	o2x_tv_daemon_scaling_tweak = scaling_tweak;
}
int LinkApp::getTVDaemonXOffset(void) {
	return o2x_tv_daemon_xoffset;
}

void LinkApp::setTVDaemonXOffset(int xoffset) {
	o2x_tv_daemon_xoffset = xoffset;
}

int LinkApp::getTVDaemonYOffset(void) {
	return o2x_tv_daemon_yoffset;
}

void LinkApp::setTVDaemonYOffset(int yoffset) {
	o2x_tv_daemon_yoffset = yoffset;
}

int LinkApp::getTVDaemonXScale(void) {
	return o2x_tv_daemon_xscale;
}

void LinkApp::setTVDaemonXScale(int xscale) {
	o2x_tv_daemon_xscale = xscale;
}

int LinkApp::getTVDaemonYScale(void) {
	return o2x_tv_daemon_yscale;
}

void LinkApp::setTVDaemonYScale(int yscale) {
	o2x_tv_daemon_yscale = yscale;
}

int LinkApp::getTVDaemonVXScale(void) {
	return o2x_tv_daemon_vxscale;
}

void LinkApp::setTVDaemonVXScale(int vxscale) {
	o2x_tv_daemon_vxscale = vxscale;
}

int LinkApp::getTVDaemonVYScale(void) {
	return o2x_tv_daemon_vyscale;
}

void LinkApp::setTVDaemonVYScale(int vyscale) {
	o2x_tv_daemon_vyscale = vyscale;
}

int LinkApp::getTVDaemonFirstDelay(void) {
	return o2x_tv_daemon_first_delay;
}

void LinkApp::setTVDaemonFirstDelay(int delay) {
	o2x_tv_daemon_first_delay = delay;
}

int LinkApp::getTVDaemonDelay(void) {
	return o2x_tv_daemon_delay;
}

void LinkApp::setTVDaemonDelay(int delay) {
	o2x_tv_daemon_delay = delay;
}

bool LinkApp::getTVDaemonTweakOnlyOnce(void) {
	return o2x_tv_daemon_tweak_only_once;
}

void LinkApp::setTVDaemonTweakOnlyOnce(bool tweak_only_once) {
	o2x_tv_daemon_tweak_only_once = tweak_only_once;
}

//Deprecated:
//bool LinkApp::getTVDaemonForce720Pitch(void) {
//	return o2x_tv_daemon_force_720_pitch;
//}
//
//void LinkApp::setTVDaemonForce720Pitch(bool force_720_pitch) {
//	o2x_tv_daemon_force_720_pitch = force_720_pitch;
//}
//
//bool LinkApp::getTVDaemonStubbornFix(void) {
//	return o2x_tv_daemon_stubborn_fix;
//}
//
//void LinkApp::setTVDaemonStubbornFix(bool stubborn_fix) {
//	o2x_tv_daemon_stubborn_fix = stubborn_fix;
//}

bool LinkApp::getTVDaemonAlwaysStarted(void) {
	return o2x_tv_daemon_always_started;
}

void LinkApp::setTVDaemonAlwaysStarted(bool always_started) {
	o2x_tv_daemon_always_started = always_started;
}

bool LinkApp::getTVDaemonEnableTVMode(void) {
	return o2x_tv_daemon_enable_tv_mode;
}

void LinkApp::setTVDaemonEnableTVMode(bool enable_tv_mode) {
	o2x_tv_daemon_enable_tv_mode = enable_tv_mode;
}

// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
// 	a USB gamepad.  It should be configurable because it will greatly interfere
// 	with apps like Picodrive that already know how to use USB joysticks.
void LinkApp::setJoy2xdStatus(bool enabled) {
	o2x_link_uses_joy2xd = enabled;
}

//senquack - new Open2X support for TV tweaking daemon:
bool LinkApp::getJoy2xdStatus(void) {
	return o2x_link_uses_joy2xd;
}

void LinkApp::setJoy2xdHidesJs0(bool hides) {
	o2x_joy2xd_hides_js0 = hides;
}

bool LinkApp::getJoy2xdHidesJs0(void) {
	return o2x_joy2xd_hides_js0;
}
