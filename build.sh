#!/bin/bash

PACKAGE=cainteoir-engine
DPUT_PPA=cainteoir-ppa
ANDROID_VERSION=8
ANDROID_COMPILER=4.6
ANDROID_ARCHS="armeabi armeabi-v7a x86 mips"

doclean() {
	rm -vf ../{libcainteoir{0,-dev},cainteoir-data,cainteoir,metadata}_*.deb
	rm -vf ../${PACKAGE}_*.{tar.gz,dsc,build,changes}
	git clean -fxd
}

dodist() {
	( ./autogen.sh && ./configure --prefix=/usr && make dist ) || exit 1
	tar -xf ${PACKAGE}-*.tar.gz || exit 1
	pushd ${PACKAGE}-* || exit 1
	( ./configure --prefix=/usr && make && make check && popd ) || \
		( popd && exit 1 )
}

builddeb() {
	DIST=$1
	shift
	doclean
	cp debian/changelog{,.downstream}
	sed -i -e "s/~unstable\([0-9]*\)) unstable;/~${DIST}\1) ${DIST};/" debian/changelog
	sed -i -e "s/(\([0-9\.\-]*\)) unstable;/(\1~${DIST}1) ${DIST};/" debian/changelog
	if [[ -e debian/$DIST.patch ]] ; then
		patch -f -p1 -i debian/$DIST.patch || touch builddeb.failed
	fi
	if [[ ! -e builddeb.failed ]] ; then
		echo "... building debian packages ($@) ..."
		debuild $@ || touch builddeb.failed
	fi
	if [[ ! -e builddeb.failed ]] ; then
		echo "... validating debian packages ..."
		lintian -Ivi ../${PACKAGE}_*.dsc || touch builddeb.failed
	fi
	if [[ -e debian/$DIST.patch ]] ; then
		patch -Rf -p1 -i debian/$DIST.patch || touch builddeb.failed
	fi
	mv debian/changelog{.downstream,}
	if [[ -e builddeb.failed ]] ; then
		rm builddeb.failed
		exit 1
	fi
}

dorelease() {
	builddeb $1 -S -sa || exit 1
	(sudo pbuilder build ../${PACKAGE}_*.dsc 2>&1 || exit 1) | tee ../${PACKAGE}_build.log
	lintian -Ivi ../${PACKAGE}_*.dsc
}

doinstall() {
	sudo dpkg --install ../{libcainteoir{0,-dev},cainteoir-data,cainteoir,metadata}_*.deb
}

douninstall() {
	yes | sudo apt-get remove libcainteoir{0,-dev} cainteoir-data cainteoir metadata
}

doppa() {
	# Ubuntu supports a single upload building on multiple distros, but only if the
	# package is in bazaar and hosted on Launchpad. The only other way to specify
	# the target version of Ubuntu to upload to is to specify the distro name in the
	# changelog.
	#
	# Ubuntu actually ignores the distro name when building the package, but the
	# |dput| command does not.
	#
	# In addition to this, it is advised that a version identifier is used for ppa
	# files, so a "~<distro-name>N" is appended.
	DIST=$1
	VER=`cat debian/changelog | grep ") unstable" | head -n 1 | sed -e "s/.*(//" -e "s/~unstable\([0-9]*\)) unstable;.*/~${DIST}\1/" -e "s/) unstable;.*/~${DIST}1/"`
	builddeb $DIST -S -sa || exit 1
	dput ${DPUT_PPA} ../${PACKAGE}_${VER}_source.changes
}

doallppa() {
	for DISTRO in oneiric precise quantal raring ; do
		doppa ${DISTRO}
	done
}

doandroid() {
	./autogen.sh || exit 1
	for arch in ${ANDROID_ARCHS} ; do
		echo "===== android ${arch} ====="
		android/ndk-configure android-${ANDROID_VERSION} ${arch} ${ANDROID_COMPILER} || exit 1
		make || exit 1
		sudo make install || exit 1
	done
}

case "$1" in
	allppa)    doallppa ;;
	android)   doandroid ;;
	clean)     doclean ;;
	deb)       builddeb $2 -us -uc ;;
	debsrc)    builddeb $2 -S -sa ;;
	dist)      dodist ;;
	ppa)       doppa $2 ;;
	release)   dorelease $2 ;;
	install)   doinstall ;;
	uninstall) douninstall ;;
	help|*)
		echo "usage: `basename $0` <command>

where <command> is one of:

    allppa         Publish to the Cainteoir Text-to-Speech Ubuntu PPA for all supported distributions.
    android        Build the library for android.
    clean          Clean the build tree and generated files.
    deb <dist>     Create a (development build) debian binary package.
    debsrc <dist>  Create a debian source package.
    dist           Create (and test) a distribution source tarball.
    help           Show this help screen.
    install        Installs the built debian packages.
    ppa <dist>     Publish to the Cainteoir Text-to-Speech Ubuntu PPA for <dist>.
    release        Create a (release build) debian binary package.
    uninstall      Uninstalls the debian packages installed by 'install'.

To publish in source form (for GNU/Linux distributions), run:
    `basename $0` dist

To build and install locally on a Debian system, run:
    `basename $0` deb <dist-name>
    `basename $0` install

To publish to Debian, run:
    `basename $0` release

To publish to the Ubuntu PPA for a specific distribution, run:
    `basename $0` release
    `basename $0` ppa <dist-name>
"
		;;
esac
