/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. Jan, 2005
 */


#ifndef _SUBDISP_H_
#define _SUBDISP_H_

void framebuffer_on(int fbnum);
void framebuffer_off(int fbnum);
int switch_fb0_to_fb1(void);
int switch_fb1_to_fb0(void);
int sub_disp(void);

#endif


