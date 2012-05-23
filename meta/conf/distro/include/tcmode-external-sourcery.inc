#
# Configuration to use external Sourcery G++ toolchain
#

TCRECIPE = "external-sourcery-toolchain"

PREFERRED_PROVIDER_gdbserver ??= "${TCRECIPE}"

require conf/distro/include/external-toolchain.inc

# No need to re-compile the locale files
GLIBC_INTERNAL_USE_BINARY_LOCALE = "precompiled"
ENABLE_BINARY_LOCALE_GENERATION = ""

# These bits are here temporarily to sidestep the need to use a separate set
# of tune files to pass the appropriate multilib selection arguments to the
# sourcery toolchain, and are needed to ensure the recipe picks up the files.
TUNE_FEATURES[type] = "list"

CSL_MULTILIB_ARGS[i586] = "-msgxx-glibc"
CSL_MULTILIB_ARGS[i686] = "-msgxx-glibc"
CSL_MULTILIB_ARGS[core2] = "-msgxx-glibc"
CSL_MULTILIB_ARGS[x86] = "-msgxx-glibc"
CSL_MULTILIB_ARGS[x86-64] = "-msgxx-glibc"
CSL_MULTILIB_ARGS[ppce500] = "-te500v1"
CSL_MULTILIB_ARGS[ppce500mc] = "-te500mc"
CSL_MULTILIB_ARGS[ppce500v2] = "-te500v2"
CSL_MULTILIB_ARGS[ppce600] = "-te600"

def csl_multilib_arg(d):
    argument = d.getVarFlag('CSL_MULTILIB_ARGS', d.getVar('DEFAULTTUNE', True) or '')
    if argument:
        return argument
    else:
        return ''

EXTERNAL_TOOLCHAIN_SYSROOT_CMD += "${@csl_multilib_arg(d)}"


CSL_TARGET_SYS_powerpc ?= "powerpc-linux-gnu"
CSL_TARGET_SYS_powerpc64 ?= "powerpc-linux-gnu"
CSL_TARGET_SYS_arm ?= "arm-none-linux-gnueabi"
CSL_TARGET_SYS_mips ?= "mips-linux-gnu"
CSL_TARGET_SYS_mipsel ?= "mips-linux-gnu"
CSL_TARGET_SYS_mips64 ?= "mips-linux-gnu"
CSL_TARGET_SYS_i686 ?= "i686-pc-linux-gnu"
CSL_TARGET_SYS_i586 ?= "i686-pc-linux-gnu"
CSL_TARGET_SYS = "${TARGET_SYS}"

TARGET_PREFIX = "${CSL_TARGET_SYS}-"


# Unfortunately, the CSL ia32 toolchain has non-prefixed binaries in its
# bindir (e.g. gcc, ld). To avoid this messing up our build, we avoid adding
# this bindir to our PATH, and instead add symlinks to the prefixed binaries
# to our staging toolchain bindir.
python toolchain_setup () {
    if not isinstance(e, bb.event.ConfigParsed):
        return

    d = e.data

    l = d.createCopy()
    l.finalize()

    if os.path.exists(bb.data.expand('${EXTERNAL_TOOLCHAIN}/bin/gcc', l)):
        d.setVar('TOOLCHAIN_PATH_ADD', '')
        populate_toolchain_links(d)
}
addhandler toolchain_setup

def populate_toolchain_links(d):
    import errno
    import os
    from glob import glob

    d = d.createCopy()
    d.finalize()

    pattern = d.expand('${EXTERNAL_TOOLCHAIN}/bin/${TARGET_PREFIX}*')
    files = glob(pattern)
    if not files:
        bb.fatal("Unable to populate toolchain binary symlinks in %s" % pattern)

    bindir = d.getVar('STAGING_BINDIR_TOOLCHAIN', True)
    bb.mkdirhier(bindir)
    for f in files:
        base = os.path.basename(f)
        newpath = os.path.join(bindir, base)
        try:
            os.symlink(f, newpath)
        except OSError as exc:
            if exc.errno == errno.EEXIST:
                break
            bb.fatal("Unable to populate toolchain binary symlink for %s: %s" % (newpath, exc))
