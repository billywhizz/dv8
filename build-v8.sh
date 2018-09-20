## generate
./tools/dev/v8gen.py x64.release -- binutils_path="\"/usr/bin\"" target_os="\"linux\"" target_cpu="\"x64\"" v8_target_cpu="\"x64\"" v8_monolithic=true v8_use_external_startup_data=false v8_enable_future=true is_official_build=true is_component_build=false is_cfi=false is_clang=false strip_debug_info=true is_desktop_linux=false use_aura=false use_dbus=false use_gio=false use_glib=false use_icf=false use_udev=false thin_lto_enable_optimizations=false use_custom_libcxx=false use_sysroot=false use_gold=false use_allocator_shim=false treat_warnings_as_errors=false symbol_level=0
## compile
ninja v8_base -C out.gn/x64.release/ -j 2
ninja v8_libbase -C out.gn/x64.release/ -j 2
ninja v8_libplatform -C out.gn/x64.release/ -j 2
#ninja v8_snapshot -C out.gn/x64.release/ -j 2
ninja v8_libsampler -C out.gn/x64.release/ -j 2
## link
