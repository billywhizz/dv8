Building From Source
https://v8.dev/docs/build#installing-build-dependencies

Checking Out the Source
https://v8.dev/docs/source-code

Chrome Releases
https://omahaproxy.appspot.com/

Release Process
https://v8.dev/docs/release-process

Build

docker run -it --rm -v $(pwd)/build:/build --workdir /build debian:stretch-slim /bin/bash

mkdir /build
cd /build
apt update
apt upgrade
apt install -y git curl python lsb-release sudo
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH=$PATH:/build/depot_tools
gclient
fetch v8
cd v8
git checkout branch-heads/7.5
gclient sync
./build/install-build-deps.sh --no-arm --no-nacl --no-backwards-compatible
./tools/dev/v8gen.py \
	x64.release \
	-- \
	binutils_path=\"/usr/bin\" \
	target_os=\"linux\" \
	target_cpu=\"x64\" \
	v8_target_cpu=\"x64\" \
	v8_use_external_startup_data=false \
	v8_enable_future=true \
	is_official_build=true \
	is_component_build=false \
	is_cfi=false \
	is_asan=false \
	is_clang=false \
	use_custom_libcxx=false \
	use_sysroot=false \
	use_gold=false \
	use_allocator_shim=false \
	treat_warnings_as_errors=false \
	strip_debug_info=true \
	is_desktop_linux=false \
	v8_enable_i18n_support=false \
	symbol_level=0 \
	v8_use_snapshot=true \
	v8_static_library=true \
	v8_monolithic=true \
	v8_experimental_extra_library_files=[] \
	v8_extra_library_files=[] \
	proprietary_codecs=false \
	safe_browsing_mode=0 \
	toolkit_views=false \
	use_aura=false \
	use_dbus=false \
	use_gio=false \
	use_glib=false \
	use_ozone=false \
	use_udev=false \
	clang_use_chrome_plugins=false \
	v8_deprecation_warnings=false \
	v8_enable_gdbjit=false \
	v8_imminent_deprecation_warnings=false \
	v8_untrusted_code_mitigations=false

ninja v8_monolith -C out.gn/x64.release/ -j 2
#./tools/dev/gm.py x64.release
gn args --list