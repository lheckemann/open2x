Movie Player for GP2X
(from mplayer on gp2x)

* Change Logs
- fourth
fixed frame time rules, more smoothly playing in some files.
subtitle class parser bug in style tag.
if not exists lang value in mp2x.ini, get charset value in GP2X system setting.

- third
add zoom function for wide resolution movie. push Select button to change zoom size.
add tv-out support, but don't work zoom function.

- second
extracted subtitle setting to mp2x.ini file.

- first
added liba52-cvs(fixed point mode), libfaad2 library.
changed internal font to unicode font.
fixed some bugs.

* License
This program is under the GPL, see http://www.fsf.org/ for more info.

* Compile Issue
If you want to compile source, need GP2X official SDK and cygwin environment.
or edit source/config.mak for your environment.

* Subtitle Setting(in mp2x.ini)
- lang : encoding charset(reference by http://www.gnu.org/software/libiconv/)
- left, right, top, bottom : margin of subtitle
- base : base position of top align
- font_red, font_green, font_blue : font color
- outline_red, outline_green, outline_blue : outline color
