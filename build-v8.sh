## generate
./tools/dev/v8gen.py x64.release \
    -- binutils_path=\"/usr/bin\" \
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
    v8_monolithic=true
## compile
ninja v8_base -C out.gn/x64.release/ -j 2
ninja v8_libbase -C out.gn/x64.release/ -j 2
ninja v8_libplatform -C out.gn/x64.release/ -j 2
ninja v8_snapshot -C out.gn/x64.release/ -j 2
ninja v8_libsampler -C out.gn/x64.release/ -j 2
#ninja v8_init -C out.gn/x64.release/ -j 2
#ninja v8_initializers -C out.gn/x64.release/ -j 2