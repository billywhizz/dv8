## Build v8 monolithic lib
FROM alpine:3.8 as v8-build

LABEL repository.hub="alexmasterov/alpine-libv8:7.1" \
      repository.url="https://github.com/AlexMasterov/dockerfiles" \
      maintainer="Alex Masterov <alex.masterow@gmail.com>"

ARG V8_VERSION=7.1.163
ARG V8_DIR=/usr/local/v8

ARG BUILD_COMMIT=641370b8b44f9bf4714782331cfba1994ccd41a5
ARG BUILDTOOLS_COMMIT=2dff9c9c74e9d732e6fe57c84ef7fd044cc45d96
ARG ICU_COMMIT=7ca3ffa77d635e44b9735e1b54fb9c4da3b6c821
ARG GTEST_COMMIT=2e68926a9d4929e9289373cd49e40ddcb9a628f7
ARG TRACE_EVENT_COMMIT=211b3ed9d0481b4caddbee1322321b86a483ca1f
ARG CLANG_COMMIT=9ad74fabeb3a6635a7205ccb9abfa1b361fb324e
ARG JINJA2_COMMIT=b41863e42637544c2941b574c7877d3e1f663e25
ARG MARKUPSAFE_COMMIT=8f45f5cfa0009d2a70589bcda0349b8cb2b72783
ARG CATAPULT_COMMIT=6f7c60dde99fb258809c0a2b3253fa1d978fdde5

ARG GN_SOURCE=https://www.dropbox.com/s/3ublwqh4h9dit9t/alpine-gn-80e00be.tar.gz
ARG V8_SOURCE=https://chromium.googlesource.com/v8/v8/+archive/${V8_VERSION}.tar.gz

ENV V8_VERSION=${V8_VERSION} \
    V8_DIR=${V8_DIR}

RUN set -x \
  && apk add --update --virtual .v8-build-dependencies \
    at-spi2-core-dev \
    curl \
    g++ \
    gcc \
    glib-dev \
    icu-dev \
    linux-headers \
    make \
    ninja \
    python \
    tar \
    xz
RUN : "---------- V8 ----------" \
  && mkdir -p /tmp/v8 \
  && curl -fSL --connect-timeout 30 ${V8_SOURCE} | tar xmz -C /tmp/v8 \
  && : "---------- Dependencies ----------" \
  && DEPS=" \
    chromium/buildtools.git@${BUILDTOOLS_COMMIT}:buildtools; \
    chromium/src/build.git@${BUILD_COMMIT}:build; \
    chromium/src/base/trace_event/common.git@${TRACE_EVENT_COMMIT}:base/trace_event/common; \
    chromium/src/tools/clang.git@${CLANG_COMMIT}:tools/clang; \
    chromium/src/third_party/jinja2.git@${JINJA2_COMMIT}:third_party/jinja2; \
    chromium/src/third_party/markupsafe.git@${MARKUPSAFE_COMMIT}:third_party/markupsafe; \
    chromium/deps/icu.git@${ICU_COMMIT}:third_party/icu; \
    external/github.com/google/googletest.git@${GTEST_COMMIT}:third_party/googletest/src; \
    catapult.git@${CATAPULT_COMMIT}:third_party/catapult \
  " \
  && while [ "${DEPS}" ]; do \
    dep="${DEPS%%;*}" \
    link="${dep%%:*}" \
    url="${link%%@*}" url="${url#"${url%%[![:space:]]*}"}" \
    hash="${link#*@}" \
    dir="${dep#*:}"; \
    [ -n "${dep}" ] \
      && dep_url="https://chromium.googlesource.com/${url}/+archive/${hash}.tar.gz" \
      && dep_dir="/tmp/v8/${dir}" \
      && mkdir -p ${dep_dir} \
      && curl -fSL --connect-timeout 30 ${dep_url} | tar xmz -C ${dep_dir} \
      & [ "${DEPS}" = "${dep}" ] && DEPS='' || DEPS="${DEPS#*;}"; \
    done; \
    wait \
  && : "---------- Downloads the current stable Linux sysroot ----------" \
  && /tmp/v8/build/linux/sysroot_scripts/install-sysroot.py --arch=amd64 \
  && : "---------- Proper GN ----------" \
  && apk add --virtual .gn-runtime-dependencies \
    libevent \
    libexecinfo \
    libstdc++ \
  && curl -fSL --connect-timeout 30 ${GN_SOURCE} | tar xmz -C /tmp/v8/buildtools/linux64/
RUN cd /tmp/v8 \
  && ./tools/dev/v8gen.py \
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
      symbol_level=1 \
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
      v8_untrusted_code_mitigations=false \
      disable_glibcxx_debug=false

RUN cd /tmp/v8 \
  && ninja v8_monolith -C out.gn/x64.release/ -j 2

## Build libuv
FROM alpine:3.8 as uv-build
WORKDIR /source
RUN apk update \
    && apk add --no-cache --virtual .build-deps make g++ python gcc git
RUN wget "https://github.com/libuv/libuv/archive/v1.23.0.tar.gz" \
    && tar -zxvf v1.23.0.tar.gz \
    && mv libuv-1.23.0 uv
RUN cd uv \
    && git clone https://chromium.googlesource.com/external/gyp build/gyp
RUN cd uv \
    && ./gyp_uv.py -f make \
    && BUILDTYPE=Release make -C out
CMD ["/bin/sh"]