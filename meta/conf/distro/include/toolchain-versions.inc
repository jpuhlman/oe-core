def toolchain_run(d, cmd, *args):
	import bb.process
	import subprocess

	topdir = d.getVar('TOPDIR', True)
	toolchain_path = d.getVar('EXTERNAL_TOOLCHAIN', True)
	if not toolchain_path:
		return 'UNKNOWN', 'UNKNOWN'

	target_prefix = d.getVar('TARGET_PREFIX', True)
	path = os.path.join(toolchain_path, 'bin', target_prefix + cmd)
	args = [path] + list(args)

	return bb.process.run(args, cwd=topdir, stderr=subprocess.PIPE)

def toolchain_version(d):
        try:
		stdout, stderr = toolchain_run(d, 'gcc', '-v')
	except bb.process.CmdError as exc:
		bb.error('Failed to obtain toolchain version: %s' % exc)
		return 'UNKNOWN'
	else:
		last_line = stderr.splitlines()[-1]
		return last_line

def toolchain_main_version(d):
	version = toolchain_version(d)
	if version != 'UNKNOWN':
		return version.split()[-1].rstrip(')')
	else:
		return version

def toolchain_gcc_version(d):
	version = toolchain_version(d)
	if version != 'UNKNOWN':
		return version.split()[2]
	else:
		return version

def toolchain_libc_version(d):
	sysroot = d.getVar('EXTERNAL_TOOLCHAIN_SYSROOT', True)
	if sysroot:
		libpath = os.path.join(sysroot, d.getVar('base_libdir', True))

		if os.path.exists(libpath):
			for file in os.listdir(libpath):
				if file.find('libc-') == 0:
					return file[5:-3]
	return 'UNKNOWN'

def toolchain_kernel_version(d):
	sysroot = d.getVar('EXTERNAL_TOOLCHAIN_SYSROOT', True)
	if not sysroot:
		return 'UNKNOWN'

	vf = os.path.join(sysroot, d.getVar('includedir', True), 'linux', 'version.h')
	try:
		f = open(vf, 'r')
	except (OSError, IOError):
		return 'UNKNOWN'

	l = f.readlines();
	f.close();
	for s in l:
		if s.find('LINUX_VERSION_CODE') > 0:
			ver = int(s.split()[2])
			maj = ver / 65536
			ver = ver % 65536
			min = ver / 256
			ver = ver % 256
			return str(maj)+'.'+str(min)+'.'+str(ver)
	return 'UNKNOWN'

def toolchain_gdb_version(d):
	try:
		stdout, stderr = toolchain_run(d, 'gdb', '-v')
	except bb.process.CmdError:
		return 'UNKNOWN'
	else:
		first_line = stdout.splitlines()[0]
		return first_line.split()[-1]

python toolchain_version_handler () {
    if not isinstance(e, bb.event.ConfigParsed):
        return
    d = e.data
    ld = d.createCopy()
    ld.finalize()

    d.setVar('TOOLCHAIN_VER_MAIN', toolchain_main_version(ld))
    d.setVar('TOOLCHAIN_VER_GCC', toolchain_gcc_version(ld))
    d.setVar('TOOLCHAIN_VER_LIBC', toolchain_libc_version(ld))
    d.setVar('TOOLCHAIN_VER_KERNEL', toolchain_kernel_version(ld))
    d.setVar('TOOLCHAIN_VER_GDB', toolchain_gdb_version(ld))
}
addhandler toolchain_version_handler

# Ensure that any variable which includes the --sysroot (CC, CXX, etc) also
# depends on the toolchain version
TOOLCHAIN_OPTIONS[vardeps] += "TOOLCHAIN_VER_MAIN TOOLCHAIN_VER_GCC"
