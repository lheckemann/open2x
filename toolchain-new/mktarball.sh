# mktarball.sh 0.42.2 for crosstool 0.42

# Quick hack to make some kind of package out of crosstool output
#
# Usage: mktarball.sh <dir> <file>
# where
# <dir> is the top directory of the crosstool output
# e.g. /opt/crosstool/gcc-4.1.0-glibc-2.4/arm-unknown-linux-gnueabi
# <file> is the name of the compressed tar file you want written:
# e.g. arm-unknown-linux-gnueabi-gcc-4.1.0-glibc-2.4-crosstool-0.42_i686.tgz
# (valid extensions are .tar, .tgz, .tar.gz, .tbz, .tar.bz2)
#
# If you want to make the closest approximation to a Debian package,
# set prefix=usr below, package to something.tgz, then use something like
#	fakeroot alien --to-deb "$targetfile"`
# to get something.deb, though when you install it, this will overwrite the file
# /usr/include/c++/4.1.0/algorithm if you already have g++-4.1.0 installed.
#
# Note: This script was written for runs of crosstool that did NOT have the
# USE_SYSROOT variable set. It could be modified to work also with that.
#
# Changes from 0.42.1:
# - fix permissions on dirs, .la, *.so *.so.* files to 644
# - strip executables (saves 22MB!)
#
#	Martin Guy <martinwguy@yahoo.it>, 19 May 2006

# Default for where to make the tarball install into: usr or usr/local are
# favourites. Note: NO leading slash here
prefix=opt/open2x/gcc-4.1.1-glibc-2.3.6

#
# Process command line args and perform sanity checks
#

progname=`basename "$0"`

usage () {
	echo "Usage: $progname [--prefix=/usr/local] crosstool-output-dir-root output-tarball-filename" 1>&2
	echo "e.g.: $progname /opt/crosstool/gcc-4.1.0-glibc-2.4/arm-unknown-linux-gnu" 1>&2
	exit 1
}

sourcedir=
targetfile=

for a
do
	case "$a" in
	--prefix=/*)
		prefix=`echo "$a" | sed 's:--prefix=/::'`
		;;
	--prefix=*)
		echo "$progname: --prefix must be an absolute path like /usr/local" 1>&2
		exit 1
		;;
	-*)	# There are no other option flags
		usage
		;;
	*)	if [ -z "$sourcedir" ]; then
			# First arg is source dir
			sourcedir="$a"
		elif [ -z "$targetfile" ]; then
			# Second arg is target tarball
			targetfile="$a"
		else
			usage
		fi
		;;
	esac
done

# Check they gave the required args
[ -z "$sourcedir" -o -z "$targetfile" ] && usage

# and that the source dir exists
[ ! -d $sourcedir ] && {
	echo "$0: \"$sourcedir\" does not exist." 1>&2
	exit 1
}

# Check compression method for output file.
case "$targetfile" in
*.tar.gz|*.tgz) compress_tar_flag=z ;;
*.tar.bz2|*.tbz) compress_tar_flag=j ;;
*.tar) compress_tar_flag= ;;
*)
	echo "$progname: Unknown output file format. Use .tar.gz, .tgz, .tar.bz2, .tbz or .tar" 1>&2
	exit 1
	;;
esac

# Make sure they have given us a crosstool-output dir to package...
if [ ! -f "$sourcedir"/*crosstoolconfig.txt ]; then
	echo "$progname: \"$sourcedir\" does not look like a crosstool output dir to me." 1>&2
	echo "$progname: (it should contain */*.crosstoolconfig.txt)" 1>&2
	exit 1
fi

# ...that was created without USE_SYSROOT
[ -d "$sourcedir"/*/sys-root ] && {
	echo "$progname: \"$sourcedir\" looks like the output from crosstool with USE_SYSROOT" 1>&2
	echo "$progname: This script doesn't support that (yet)." 1>&2
	exit 1
}

#
# Do the work
#

# Dirty hack: instead of copying everything into "./usr/" or "./usr/local/",
# removing the unnecessary cruft and bundling what's left, we simply make a
# symbolic link called "usr" or "usr/local", point that at the output dir
# and then tell tar to bundle up that/. excluding the things we don't want.
# That probably means that the tarfile will contain paths of the form
# prefix/./bin... which will probably blow some package managers apart.
# Hey ho.

# If link exists from a previous run, remove it.
# The preliminary "-h" is necessary 'cos a test on a link
# tests the object that it points to.
if [ -h "$prefix" ]; then	# symlink
	rm "$prefix" || exit 1
elif [ -d "$prefix" ]; then	# dir
	rmdir "$prefix" || exit 1
elif [ -e "$prefix" ]; then	# file or other
	rm "$prefix" || exit 1
fi

# Sourcedir must be absolute for symlink to work
case "$sourcedir" in
/*)	abssourcedir="$sourcedir"
	;;
*)	abssourcedir="`pwd`/$sourcedir"
	;;
esac

# If our fake prefix already exists, remove it.
# If we can't remove it (including cos it's a dir)
# then we don't know what to do, so fail.
if [ -e "$prefix" ]; then
	rm "$prefix" || exit 1
fi

# Make prefix stub. If we create "opt", remember to delete it afterwards.
we_created_usr=false
case "$prefix" in
opt)	# No extra dir creation is necessary
	;;
opt/open2x)
	if [ ! -d opt ]; then
		mkdir opt || exit 1
		we_created_usr=true
	fi
	;;
opt/open2x/gcc-4.1.1-glibc-2.3.6)
	if [ ! -d opt/open2x ]; then
		mkdir opt/open2x || exit 1
		we_created_usr=true
	fi
	;;
*)
	echo "$progname: Prefixes other than \"opt\" and \"opt/open2x\" are not supported" 1>&2
	exit 1
	;;
esac
ln -s "$abssourcedir" "$prefix" || exit 1

# Fix file permissions
find "$sourcedir" -type d \! -perm 755 -exec chmod 755 {} \;
find "$sourcedir" -type f \
	\( -name '*.la' -o -name '*.so' -o -name '*.so.*' \) \
	! -perm 644 -exec chmod 644 {} \;

# Strip binaries for big filesize saving
# (except gdbserver which is a target executable and
# *-gccbug which is a shell script)
for a in "$sourcedir"/bin/*
do
	case $a in
	*/gdbserver) ;;
	*-gccbug) ;;
	*) strip $a ;;
	esac
done

# Exclude all crosstool-specific files, garbage,
# documentation (which is the same as the standard non-crosstool stuff),
# lib/libiberty.a and lib/libarm*.a, which contain x86 objects,
# the directory .../lib/libgcc_s.so.1.dir containing install scripts,
# "share" which only contains locale/... files that would overwrite the
# host compiler's files,
# and the bizarre empty "usr/lib"
tar cf$compress_tar_flag "$targetfile" \
	--exclude="$prefix/./*.crosstoolconfig.txt" \
	--exclude="$prefix/./bin/fix-embedded-paths" \
	--exclude="$prefix/./bin/gdbserver" \
	--exclude="$prefix/./libexec/install-shared-lib.sh" \
	--exclude="*.dir" \
	--exclude="$prefix/./distributed" \
	--exclude="$prefix/./tmp" \
	--exclude="$prefix/./info" \
	--exclude="$prefix/./man" \
	--exclude="$prefix/./*/info" \
	--exclude="$prefix/./share" \
	--exclude="$prefix/./usr" \
	"$prefix"/.
status=$?

# Remove the temp stuff we created
rm "$prefix"
$we_created_usr && rmdir opt

# If "tar" failed, we fail too.
if [ $status != 0 ]; then
	echo "$progname: Failed to make $targetfile." 1>&2
	exit $status
fi

exit 0
