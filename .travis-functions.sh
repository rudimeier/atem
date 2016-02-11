#!/bin/bash

#
# .travis-functions.sh:
#   - helper functions to be sourced from .travis.yml
#   - designed to respect travis' environment but testing locally is possible
#

if [ ! -f "configure.ac" ]; then
	echo ".travis-functions.sh must be sourced from source dir" >&2
	return 1 || exit 1
fi

# some config settings
MAKE="make -j1"
DUMP_CONFIG_LOG="short"

# We could test (exotic) out-of-tree build dirs using relative or abs paths.
# After sourcing this script we are living in build dir. Tasks for source dir
# have to use $SOURCE_DIR.
SOURCE_DIR="."
BUILD_DIR="."
CONFIGURE="$SOURCE_DIR/configure"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || return 1 || exit 1

function configure_travis
{
	"$CONFIGURE" "$@"
	err=$?
	if [ "$DUMP_CONFIG_LOG" = "short" ]; then
		grep -B1 -A10000 "^## Output variables" config.log | grep -v "_FALSE="
	elif [ "$DUMP_CONFIG_LOG" = "full" ]; then
		cat config.log
	fi
	return $err
}

function script_linux
{
	if test "${BUILD_MINGW}" = "yes"; then
		unset CC; unset CXX;
		echo -e '#!/bin/bash\nwine $0.exe "$@"' > src/atem;
		chmod ugo+x src/atem;
	fi

	configure_travis ${CONFOPTS} || return
	$MAKE || return
	$MAKE check || (cat test/test-suite.log && false) || return

	if test "${BUILD_MINGW}" != "yes"; then
		$MAKE distcheck || return false
	fi
}

function script_osx
{
	configure_travis ${CONFOPTS} \
		|| return
	$MAKE distcheck || return
}

function install_deps_linux
{
	# install some packages from Ubuntu's default sources
	sudo apt-get -qq update || return
	sudo apt-get install -qq >/dev/null \
		gengetopt \
		help2man \
		|| return
	if test "${BUILD_MINGW}" = "yes"; then
		sudo apt-get -qq install wine || return
	fi
}

function install_deps_osx
{
	brew update >/dev/null
	brew install \
		gengetopt \
		help2man \
		xz \
		|| return
}

function travis_have_sudo
{
	HAVE_SUDO="no"
	if test "$(sudo id -ru)" = "0"; then
		HAVE_SUDO="yes"
	fi
	echo "HAVE_SUDO=$HAVE_SUDO"
}

function travis_install_script
{
	if [ "$TRAVIS_OS_NAME" = "osx" ]; then
		install_deps_osx || return
	else
		install_deps_linux || return
	fi
}

function travis_before_script
{
	pushd "$SOURCE_DIR" || return
	set -o xtrace

	autoreconf -vfi
	ret=$?

	set +o xtrace
	popd
	return $ret
}

function travis_script
{
	local ret
	set -o xtrace

	if [ "$TRAVIS_OS_NAME" = "osx" ]; then
		script_osx
	else
		script_linux
	fi

	# We exit here with if-else return value!
	ret=$?
	set +o xtrace
	return $ret
}
