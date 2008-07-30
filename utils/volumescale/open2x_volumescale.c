#include <stdlib.h>

#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>
#include <fcntl.h>

const int max_scale_factor = 200;

void gp2x_set_volume(int newvol)
{
   if ((newvol >= 0) && (newvol <= 100)) {
      unsigned long soundDev = open("/dev/mixer", O_RDWR);
      if (soundDev)
       {
           int vol = ((newvol << 8) | newvol);
           ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
           close(soundDev);
       }
    }
}
 
//DKS - new, called at startup so we can
//    set it back to what it was when we exit.
// Returns 0-100, current mixer volume, -1 on error.
int gp2x_get_volume(void)
{
   int vol = -1;
    unsigned long soundDev = open("/dev/mixer", O_RDONLY);
    if (soundDev)
    {
        ioctl(soundDev, SOUND_MIXER_READ_PCM, &vol);
        close(soundDev);
        if (vol != -1) {
            //just return one channel , not both channels, they're hopefully the same anyways
          return (vol & 0xFF);
        }
    }
   return vol;
}

int gp2x_get_scalefactor(void)
{
   int currentscalefactor = -1;
    unsigned long soundDev = open("/dev/mixer", O_RDONLY);
    if (soundDev)
    {
        ioctl(soundDev, SOUND_MIXER_PRIVATE1, &currentscalefactor);
        close(soundDev);
    }
	return currentscalefactor;
}

void gp2x_set_scalefactor(int newfactor)
{
   if ((newfactor >= 0) && (newfactor <= max_scale_factor)) {
      unsigned long soundDev = open("/dev/mixer", O_WRONLY);
      if (soundDev)
       {
           ioctl(soundDev, SOUND_MIXER_PRIVATE2, &newfactor);
           close(soundDev);
       }
    }
}

// Entry point
int main(int argc, char *argv[])
{

	if (argc > 1) {
		// We have been passed command-line parameters
		switch (argv[1][1]) {
			case 'h':
				// display help:
				printf("GP2X master volume scaling/muting utility written by Senor Quack v1.0\n");
				printf("GPL license, free for copying and use under GPL terms.\n\n");
				printf("OPTIONS:\n");
				printf("-h\t\t\t:\tDisplay this help and version info\n\n");
				printf("-sSCALEFACTOR\t\t:\tSet the overriding scale factor to a number between 0 and 200.\n");
				printf("\t\t\t\t(Returns immediately, return status is 0 on success.)\n");
				printf("\tSCALEFACTOR is stored in the kernel audio driver and all values\n");
				printf("\twritten to /dev/mixer involving volume changes get scaled to this\n");
				printf("\tpercentage.  Therefore a value of 100 would cause no overall changes.\n");
				printf("\tA value of 50 would cause all values sent to be half their original value.\n");
				printf("\tA value of 0 will mute the entire system until the scale factor is set\n");
				printf("\tdifferently.\n\n");
				printf("\tValues > %d will be clipped to %d.\n", max_scale_factor, max_scale_factor);
				printf("\tValues < 0 will be clipped to 0.\n\n");
				printf("* The current mixer value is read and then set to a new scaled value if\n");
				printf("\tthe -s option was used to set a new scaling factor, so changes take\n");
				printf("\tplace immediately.\n");
				printf("* Program will return the resulting scale factor as its return value, \n");
				printf("\tor, if no arguments were passed, the current factor, or -1 on error..\n");
				break;
			case 's':
				// caller wants to set a new volume scale factor
				if (argv[1][2] == '\0') {
					//caller didn't supply required argument to -s
					printf("Error:  -s requires a parameter, see help by passing -h\n");
					return -1;
				} else {
					int newscalefactor = atoi(&argv[1][2]);
					if (newscalefactor < 0) {
						//caller supplied scale factor that is out of range
						newscalefactor = 0;
					} else if (newscalefactor > max_scale_factor) {
						//caller supplied scale factor that is out of range
						newscalefactor = max_scale_factor;
					} 
					
					// set scaling factor, then read and write back current mixer value to
					// make changes effective immediately
					gp2x_set_scalefactor(newscalefactor);

					int current_vol;
				  	current_vol	= gp2x_get_volume();	
					if ((current_vol >= 0) && (current_vol <= 100)) {
						gp2x_set_volume(current_vol);
					}
					
					return gp2x_get_scalefactor();

				}
				break;
			default:
				// invalid parameter
				printf("Error: invalid parameter passed, see help by passing -h\n");
				return -1;
		} // switch
	} else {
		// we were not passed any parameters, just return the current scalefactor
		return gp2x_get_scalefactor();
	}

	// if we get here, something odd happened, return an error
	return -1;


}

