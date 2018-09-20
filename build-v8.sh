## generate
./tools/dev/v8gen.py x64.release -- binutils_path="\"/usr/bin\"" target_os="\"linux\"" target_cpu="\"x64\"" v8_target_cpu="\"x64\"" v8_monolithic=true v8_use_external_startup_data=false v8_enable_future=true is_official_build=true is_component_build=false is_cfi=false is_clang=false strip_debug_info=true is_desktop_linux=false use_aura=false use_dbus=false use_gio=false use_glib=false use_icf=false use_udev=false use_custom_libcxx=false use_sysroot=false use_gold=false use_allocator_shim=false treat_warnings_as_errors=false symbol_level=0 v8_static_library=true v8_enable_i18n_support=false v8_experimental_extra_library_files="[]" v8_extra_library_files="[]" v8_use_snapshot=false
## compile
ninja v8_base -C out.gn/x64.release/ -j 2
ninja v8_libbase -C out.gn/x64.release/ -j 2
ninja v8_libplatform -C out.gn/x64.release/ -j 2
ninja v8_snapshot -C out.gn/x64.release/ -j 2
ninja v8_libsampler -C out.gn/x64.release/ -j 2
## link
export V8_LIBS=$(pwd)/out.gn/x64.release/obj
export OUTDIR=$(pwd)/dv8-build
#libv8_libplatform
rm -f $OUTDIR/libv8_libplatform.a && ar crsT $OUTDIR/libv8_libplatform.a $V8_LIBS/v8_libplatform/*.o
#libv8_libbase
rm -f $OUTDIR/libv8_libbase.a && ar crsT $OUTDIR/libv8_libbase.a $V8_LIBS/v8_libbase/*.o
#libv8_libsampler
rm -f $OUTDIR/libv8_libsampler.a && ar crsT $OUTDIR/libv8_libsampler.a $V8_LIBS/v8_libsampler/*.o
#libv8_snapshot
#rm -f $OUTDIR/libv8_snapshot.a && ar crsT $OUTDIR/libv8_snapshot.a $V8_LIBS/v8_snapshot/*.o
#libv8_base
rm -f $OUTDIR/libv8_base.a && ar crsT $OUTDIR/libv8_base.a $V8_LIBS/v8_base/*.o