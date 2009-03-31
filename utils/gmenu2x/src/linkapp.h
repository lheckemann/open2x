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
#ifndef LINKAPP_H
#define LINKAPP_H

#include <string>
#include <iostream>
//senquack - needed for new gpio remapping options:
#include <iomanip>

#include "link.h"

//senquack
#include "open2x.h"

using std::string;

class GMenu2X;

/**
Parses links files.

	@author Massimiliano Torromeo <massimiliano.torromeo@gmail.com>
*/
class LinkApp : public Link {
private:
	string sclock, svolume;
	int iclock, ivolume;
	//G
	string sgamma;
	//G
	int igamma;
	string exec, params, workdir, manual, selectordir, selectorfilter, selectorscreens;
	bool selectorbrowser, useRamTimings;

	//senquack - new open2x support for /dev/GPIO button remapping:
	int o2x_gpio_mapping[19];
	bool o2x_gpio_remapping;

	// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
	// 	no longer necessary:
	int o2x_upper_memory_cached;

	//senquack - new Open2X support for TV tweaking daemon:
	bool o2x_tv_daemon_enabled;
	bool o2x_tv_daemon_tweak_yuv;
	bool o2x_tv_daemon_scaling_tweak;
	int o2x_tv_daemon_xoffset;
	int o2x_tv_daemon_yoffset;
	int o2x_tv_daemon_xscale;
	int o2x_tv_daemon_yscale;
	int o2x_tv_daemon_vxscale;
	int o2x_tv_daemon_vyscale;
	int o2x_tv_daemon_first_delay;
	int o2x_tv_daemon_delay;
	bool o2x_tv_daemon_tweak_only_once;
	//	Deprecated:
//	bool o2x_tv_daemon_force_720_pitch;
//	bool o2x_tv_daemon_stubborn_fix;
	bool o2x_tv_daemon_always_started;
	bool o2x_tv_daemon_enable_tv_mode;

	// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
	// 	a USB gamepad.  It should be configurable because it will greatly interfere
	// 	with apps like Picodrive that already know how to use USB joysticks.
	bool o2x_link_uses_joy2xd;	
	bool o2x_joy2xd_hides_js0;

	void drawRun();

	string aliasfile;

public:
	LinkApp(GMenu2X *gmenu2x, const char* linkfile);
	virtual string searchIcon();

	string getExec();
	void setExec(string exec);
	string getParams();
	void setParams(string params);
	string getWorkdir();
	void setWorkdir(string workdir);
	string getManual();
	void setManual(string manual);
	string getSelectorDir();
	void setSelectorDir(string selectordir);
	bool getSelectorBrowser();
	void setSelectorBrowser(bool value);
	bool getUseRamTimings();
	void setUseRamTimings(bool value);
	string getSelectorScreens();
	void setSelectorScreens(string selectorscreens);
	string getSelectorFilter();
	void setSelectorFilter(string selectorfilter);
	string getAliasFile();
	void setAliasFile(string aliasfile);
	
	//senquack - new open2x gpio remapping support:
	int getGpioMapping(int button);
	void setGpioMapping(int button, int mapped_to);
	bool getGpioRemappingStatus(void);
	void setGpioRemappingStatus(bool newstatus);

	// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
	// 	no longer necessary:
	void setUpperMemoryCachingStatus(int upper_memory_cached);
	bool getUpperMemoryCachingStatus(void);

	//senquack - new Open2X support for TV tweaking daemon:
	bool 	getTVDaemonStatus(void);
	void 	setTVDaemonStatus(bool daemon_enabled);
	bool 	getTVDaemonTweakYuv(void);
	void 	setTVDaemonTweakYuv(bool tweak_yuv);
	bool 	getTVDaemonScalingTweak(void);
	void 	setTVDaemonScalingTweak(bool scaling_tweak);
	int 	getTVDaemonXOffset(void);
	void 	setTVDaemonXOffset(int xoffset);
	int 	getTVDaemonYOffset(void);
	void 	setTVDaemonYOffset(int yoffset);
	int 	getTVDaemonXScale(void);
	void 	setTVDaemonXScale(int xscale);
	int 	getTVDaemonYScale(void);
	void 	setTVDaemonYScale(int yscale);
	int 	getTVDaemonVXScale(void);
	void 	setTVDaemonVXScale(int vxscale);
	int 	getTVDaemonVYScale(void);
	void 	setTVDaemonVYScale(int vyscale);
	int 	getTVDaemonFirstDelay(void);
	void 	setTVDaemonFirstDelay(int delay);
	int 	getTVDaemonDelay(void);
	void 	setTVDaemonDelay(int delay);
	bool 	getTVDaemonTweakOnlyOnce(void);
	void 	setTVDaemonTweakOnlyOnce(bool tweak_only_once);
	//	Deprecated:
//	bool 	getTVDaemonForce720Pitch(void);
//	void 	setTVDaemonForce720Pitch(bool force_720_pitch);
//	bool 	getTVDaemonStubbornFix(void);
//	void 	setTVDaemonStubbornFix(bool stubborn_fix);
	bool 	getTVDaemonAlwaysStarted(void);
	void 	setTVDaemonAlwaysStarted(bool always_started);
	bool 	getTVDaemonEnableTVMode(void);
	void 	setTVDaemonEnableTVMode(bool enable_tv_mode);

	// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
	// 	a USB gamepad.  It should be configurable because it will greatly interfere
	// 	with apps like Picodrive that already know how to use USB joysticks.
	void setJoy2xdStatus(bool enabled);
	bool getJoy2xdStatus(void);
	void setJoy2xdHidesJs0(bool hides);
	bool getJoy2xdHidesJs0(void);


	string file;

	int clock();
	string clockStr(int maxClock);
	void setClock(int mhz);

	int volume();
	string volumeStr();
	void setVolume(int vol);

//G
	int gamma();
	string gammaStr();
	void setGamma(int gamma);
// /G

	bool wrapper;
	bool dontleave;

	bool save();
	void run();
	void showManual();
	void selector(int startSelection=0, string selectorDir="");
	void launch(string selectedFile="", string selectedDir="");
	bool targetExists();
};

#endif
