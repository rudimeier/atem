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
MAKE="make -j2"
DUMP_CONFIG_LOG="short"

function xconfigure
{
	local ret
	./configure "$@"
	ret=$?
	if [ "$DUMP_CONFIG_LOG" = "short" ]; then
		grep -B1 -A10000 "^## Output variables" config.log | grep -v "_FALSE="
	elif [ "$DUMP_CONFIG_LOG" = "full" ]; then
		cat config.log
	fi
	return $ret
}

function script_generic
{
	xconfigure ${CONFOPTS} || return

	if test "${DISTCHECK}" = "yes"; then
		$MAKE distcheck || return
	else
		$MAKE || return
		$MAKE check || { cat test/test-suite.log; return 1; }
	fi
}

function script_mingw
{
	local md5a md5b

	unset CC; unset CXX;

	# Create a fake executable to use wine in the test suite.
	echo -e '#!/bin/bash\nwine $0.exe "$@"' > src/atem;
	md5a=($(md5sum src/atem))
	chmod ugo+x src/atem;

	xconfigure ${CONFOPTS} || return
	$MAKE || return
	$MAKE check || { cat test/test-suite.log; return 1; }

	# Is this still our fake executable?
	md5b=($(md5sum src/atem))
	if [ "$md5a" != "$md5b" ]; then
		echo "error: probably a non-windows compiler was used!"
	fi
}

function install_deps_linux
{
	# the default packages are installed via apt addon in .travis.yml

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
	local ret
	set -o xtrace

	autoreconf -vfi
	ret=$?

	set +o xtrace
	return $ret
}

function travis_script
{
	local ret
	set -o xtrace

	if test "${BUILD_MINGW}" = "yes"; then
		script_mingw
	else
		script_generic
	fi

	# We exit here with if-else return value!
	ret=$?
	set +o xtrace
	return $ret
}
