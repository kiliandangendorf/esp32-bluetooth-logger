Import("env")

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}
print(defines)
version_s:str=defines.get('FW_VERSION')
version_s=version_s.replace(".", "-")
version_s=version_s.replace("\"", "")
print("Retrieved version from build_flag 'FW_VERSION': '{}' sanitized to '{}'".format(defines.get('FW_VERSION'),version_s))
filename:str="firmware-{}".format(version_s)
print("- filename: '{}'".format(filename))

env.Replace(PROGNAME=filename)