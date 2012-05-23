EXTERNAL_TOOLCHAIN ??= "/usr/local/${TARGET_ARCH}"

TOOLCHAIN_PATH_ADD = "${EXTERNAL_TOOLCHAIN}/bin:"
PATH =. "${TOOLCHAIN_PATH_ADD}"

TOOLCHAIN_OPTIONS = " --sysroot=${STAGING_DIR_HOST}"

# Point to the appropriate multilib sysroot from the external toolchain, whose
# files will be extracted into the OE sysroot
def exttc_run(d, cmd):
    try:
        return bb.process.run(cmd, shell=True, env={'PATH': d.getVar('PATH', True)})[0].rstrip()
    except (OSError, bb.process.CmdError):
        return ''

EXTERNAL_TOOLCHAIN_SYSROOT_CMD = "${TARGET_PREFIX}gcc ${TARGET_CC_ARCH} -print-sysroot"
EXTERNAL_TOOLCHAIN_SYSROOT ??= "${@exttc_run(d, EXTERNAL_TOOLCHAIN_SYSROOT_CMD)}"

# Ensure we use our external toolchain recipe
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}binutils = "${TCRECIPE}"
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}gcc-initial = "${TCRECIPE}"
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}gcc-intermediate = "${TCRECIPE}"
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}gcc = "${TCRECIPE}"
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}g++ = "${TCRECIPE}"
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}compilerlibs = "${TCRECIPE}"
PREFERRED_PROVIDER_libgcc = "${TCRECIPE}"

PREFERRED_PROVIDER_linux-libc-headers = "${TCRECIPE}"
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}libc-for-gcc = "${TCRECIPE}"
PREFERRED_PROVIDER_virtual/libc = "${TCRECIPE}"
PREFERRED_PROVIDER_virtual/libintl = "${TCRECIPE}"
PREFERRED_PROVIDER_virtual/libiconv = "${TCRECIPE}"

# Verify that the exists and is hopefully sane
python toolchain_check_sanity () {
    if not isinstance(e, bb.event.ConfigParsed):
        return

    d = e.data
    l = d.createCopy()
    l.finalize()

    def error(msg):
        bb.warn(l.expand('Invalid external toolchain at ${EXTERNAL_TOOLCHAIN}: ' + msg))

    if not os.path.exists(l.expand('${EXTERNAL_TOOLCHAIN}/bin/${TARGET_PREFIX}gcc')):
        error('no ${TARGET_PREFIX}gcc found')
        return

    sysroot = l.getVar('EXTERNAL_TOOLCHAIN_SYSROOT', True)
    if not sysroot:
        error('unable to determine sysroot (-print-sysroot)')
        return

    if not os.path.exists(sysroot):
        error('sysroot "${EXTERNAL_TOOLCHAIN_SYSROOT}" does not exist')
        return
}
addhandler toolchain_check_sanity

# Extract version information from the toolchain for use in the recipe
require conf/distro/include/toolchain-versions.inc
