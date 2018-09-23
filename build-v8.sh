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
    v8_monolithic=true \
    use_glib=false \
    use_udev=false \
    v8_experimental_extra_library_files=[] \
    v8_extra_library_files=[]
## compile
ninja v8_monolith -C out.gn/x64.release/ -j 2