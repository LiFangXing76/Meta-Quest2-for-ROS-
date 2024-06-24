# Copyright (c) 2017-2024, The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Hint the path to the OpenCVConfig.cmake file
if(ANDROID)
    set(OpenCV_DIR "D:/opencv/opencv/build/install/OpenCV-android-sdk/sdk/native/jni")
    set(GSTREAMER_ROOT_ANDROID "D:/Downloads/gstreamer-1.0-android-universal-1.24.3.tar/gstreamer-1.0-android-universal-1.24.3")
else () # case of Windows
    set(OpenCV_DIR "D:/opencv/opencv/build/install/OpenCV-android-sdk/sdk/native/jni")
    set(GSTREAMER_ROOT_ANDROID "D:/Downloads/gstreamer-1.0-android-universal-1.24.3.tar/gstreamer-1.0-android-universal-1.24.3")
endif ()

find_package(OpenCV REQUIRED java)

set(LOCAL_HEADERS
    check.h
    common.h
    geometry.h
    graphicsapi.h
    graphicsplugin.h
    logger.h
    openxr_program.h
    options.h
    pch.h
    platformdata.h
    platformplugin.h 
    pipeline.h
)
set(LOCAL_SOURCE
    graphicsplugin_factory.cpp
    graphicsplugin_vulkan.cpp
    logger.cpp
    main.cpp
    openxr_program.cpp
    pch.cpp
    platformplugin_android.cpp
    platformplugin_factory.cpp
    platformplugin_posix.cpp
    platformplugin_win32.cpp 
    pipeline.cpp
)
set(VULKAN_SHADERS vulkan_shaders/frag.glsl vulkan_shaders/vert.glsl)
#set(VULKAN_SHADERS vulkan_shaders/shader.frag vulkan_shaders/shader.vert)

if(ANDROID)
    if (${ANDROID_ABI} STREQUAL "armeabi")
        set(GSTREAMER_ROOT "${GSTREAMER_ROOT_ANDROID}/arm")
    elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
        set(GSTREAMER_ROOT "${GSTREAMER_ROOT_ANDROID}/armv7")
    elseif (${ANDROID_ABI} STREQUAL "arm64-v8a")
        set(GSTREAMER_ROOT "${GSTREAMER_ROOT_ANDROID}/arm64")
    elseif (${ANDROID_ABI} STREQUAL "x86")
        set(GSTREAMER_ROOT "${GSTREAMER_ROOT_ANDROID}/x86")
    elseif (${ANDROID_ABI} STREQUAL "x86_64")
        set(GSTREAMER_ROOT "${GSTREAMER_ROOT_ANDROID}/x86_64")
    else ()
        message(STATUS "error Target arch ABI not supported: ${ANDROID_ABI}")
    endif()

    link_directories(
            ${GSTREAMER_ROOT}/lib
            ${GSTREAMER_ROOT}/lib/gstreamer-1.0
            ${GSTREAMER_ROOT}/lib/gio/modules
    )

    SET(GST_LIBS gstreamer-1.0 gioopenssl gmodule-2.0 gobject-2.0 glib-2.0 iconv intl ffi pcre2-8 z gio-2.0 gstnet-1.0 gsttag-1.0 gstinsertbin-1.0)
    SET(GST_PLUGINS_CORE_LIBS gstvideosignal gstwebrtcdsp gstladspa gstrsaudiofx gstrsvideofx gstvideoframe_audiolevel gstclosedcaption gstdebugutilsbad gstfieldanalysis gstfreeverb gstfrei0r gstgaudieffects gstgeometrictransform gstinter gstinterlace gstisac gstivtc gstlegacyrawparse gstproxy gstremovesilence gstsegmentclip gstsmooth gstspeed gstsoundtouch gsttimecode gstvideofiltersbad gstvideobox gstvideocrop gstvideomixer gstaccurip gstaes gstaiff gstaudiobuffersplit gstaudiofxbad gstaudiolatency gstaudiomixmatrix gstautoconvert gstbayer gstcoloreffects gstdebug gstdeinterlace gstdtmf gsteffectv gstequalizer gstgdkpixbuf gstimagefreeze gstinterleave gstlevel gstmultifile gstreplaygain gstshapewipe gstsmpte gstspectrum gstalpha gstalphacolor gstaudiofx gstcairo gstcutter gstopengl gstipcpipeline gstrsrtp gstrsrtsp gstwebrtchttp gstrswebrtc gstudp gstdtls gstnetsim gstrist gstrtmp2 gstrtpmanagerbad gstrtponvif gstsctp gstsdpelem gstsrtp gstsrt gstwebrtc gstnice gstrtspclientsink gstaws gsthlssink3 gstndi gstrsonvif gstraptorq gstreqwest gstclaxon gstdav1d gstrsclosedcaption gstffv1 gstfmp4 gstmp4 gstgif gsthsv gstlewton gstrav1e gstjson gstrspng gstregex gsttextwrap gsttextahead gsttcp gstrtsp gstrtp gstrtpmanager gstsoup gstmxf gstopenh264 gstopusparse gstpcapparse gstpnm gstrfbsrc gstsiren gstsmoothstreaming gstsubenc gstvideoparsersbad gsty4mdec gstjpegformat gstgdp gstrsvg gstopenjpeg gstspandsp gstsbc gstzbar gstandroidmedia gstcdg gstauparse gstwavenc gstwavpack gstwavparse gstxingmux gsty4menc gstadpcmdec gstadpcmenc gstassrender gstbz2 gstcodecalpha gstcodectimestamper gstdash gstdvbsubenc gstdvbsuboverlay gstdvdspu gsthls gstid3tag gstivfparse gstmidi gstaudioparsers gstavi gstdv gstflac gstflv gstflxdec gsticydemux gstid3demux gstisomp4 gstjpeg gstlame gstmatroska gstmpg123 gstmulaw gstmultipart gstpng gstspeex gsttaglib gstvpx gsturiplaylistbin gstsubparse gstogg gsttheora gstvorbis gstopus gstadaptivedemux2 gstalaw gstamrnb gstamrwbdec gstapetag gstplayback gstcoreelements gstcoretracers gstadder gstapp gstaudioconvert gstaudiomixer gstaudiorate gstaudioresample gstaudiotestsrc gstcompositor gstgio gstoverlaycomposition gstpango gstrawparse gsttypefindfunctions gstvideoconvertscale gstvideorate gstvideotestsrc gstvolume gstautodetect gstvideofilter gstinsertbin gstswitchbin gstfallbackswitch gstlivesync gstrsinter gstrstracers gstthreadshare gsttogglerecord)
    SET(GST_PLUGINS_BASE gstvideotestsrc gstautodetect gstallocators-1.0 orc-test-0.4 gstapp-1.0 gstaudio-1.0 gstfft-1.0 gstpbutils-1.0 gstriff-1.0 gstrtp-1.0 gstrtsp-1.0 gstsdp-1.0 gsttag-1.0 gstvideo-1.0 gstgl-1.0 orc-0.4 gstopensles OpenSLES gstaudio-1.0 gstbase-1.0 png16 freetype pangoft2-1.0 cairo bz2 pixman-1 fontconfig expat harfbuzz fribidi pangocairo-1.0 pango-1.0 opencore-amrwb opencore-amrnb gstpbutils-1.0 gstnet-1.0 psl xml2 crypto soup-2.4 sqlite3 ssl gstpbutils-1.0 opus vorbisenc vorbis ogg theoradec theoraenc vpx tag speex mpg123 mp3lame jpeg FLAC gstcodecparsers-1.0 dv gsturidownloader-1.0 gstisoff-1.0 gstadaptivedemux-1.0 srt ass nice gstwebrtc-1.0 gstsdp-1.0 gstsctp-1.0 srtp2 wavpack gstphotography-1.0 gstallocators-1.0 gstgl-1.0 zbar sbc spandsp gstcodecparsers-1.0 openjp2 gstwebrtc-1.0 gstsctp-1.0 gstsdp-1.0 gstwebrtcnice-1.0 gstrtsp-1.0 gstrtspserver-1.0 cairo rsvg-2 gdk_pixbuf-2.0 ffi tiff openh264 dav1d gdk_pixbuf-2.0 pangocairo-1.0 pangoft2-1.0 pango-1.0 croco-0.6 gstcontroller-1.0 graphene-1.0 gstgl-1.0 EGL GLESv2 m dl cairo cairo-gobject SoundTouch ltc webrtc-audio-coding-1 webrtc-audio-processing-1 gstbadaudio-1.0 android)

    add_library(static_gstreamer SHARED nativelib/dummy.cpp nativelib/static_gstreamer.c)
    target_include_directories(
            static_gstreamer PRIVATE
            "${GSTREAMER_ROOT}/include/gstreamer-1.0"
            "${GSTREAMER_ROOT}/include/glib-2.0"
            "${GSTREAMER_ROOT}/lib/glib-2.0/include")
    target_link_libraries(
            static_gstreamer PUBLIC ${GST_LIBS} ${GST_PLUGINS_CORE_LIBS} ${GST_PLUGINS_BASE} iconv log android
    )
    target_link_libraries( static_gstreamer PUBLIC
            ${GLIB_LIBRARIES}
            ${GST_LIBRARIES}
            ${log-lib}
            orc-0.4
            -liconv
    )
    add_library(
        hello_xr MODULE
        ${LOCAL_SOURCE}
        ${LOCAL_HEADERS}
        ${VULKAN_SHADERS}
        $<TARGET_OBJECTS:android_native_app_glue>
    )

    target_include_directories(
            hello_xr PRIVATE
            "${GSTREAMER_ROOT}/include/gstreamer-1.0"
            "${GSTREAMER_ROOT}/include/glib-2.0"
            "${GSTREAMER_ROOT}/lib/glib-2.0/include"
            )
    
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    target_link_libraries(
        hello_xr PRIVATE ${ANDROID_LIBRARY} ${ANDROID_LOG_LIBRARY} static_gstreamer          
    )

    # Only for Android because it lacks a command line
    set(HELLOXR_DEFAULT_GRAPHICS_PLUGIN "Vulkan")

    # Now handle the value, normally provided by Gradle
    message(STATUS "hello_xr will default to Vulkan graphics plugin")
    target_compile_definitions(
        hello_xr PRIVATE DEFAULT_GRAPHICS_PLUGIN_VULKAN
    )

else()
    add_executable(hello_xr ${LOCAL_SOURCE} ${LOCAL_HEADERS} ${VULKAN_SHADERS})
endif()

set_target_properties(hello_xr PROPERTIES FOLDER ${SAMPLES_FOLDER})
source_group("Headers" FILES ${LOCAL_HEADERS})
source_group("Shaders" FILES ${VULKAN_SHADERS})

target_link_libraries(hello_xr PRIVATE OpenXR::openxr_loader ${OpenCV_LIBS})

compile_glsl(run_hello_xr_glsl_compiles ${VULKAN_SHADERS})

add_dependencies(hello_xr run_hello_xr_glsl_compiles)

target_include_directories(
    hello_xr
    PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/src/common"
        "${PROJECT_SOURCE_DIR}/src/tests/hello_xr/vulkan_shaders"        
        # for helper headers
        "${PROJECT_SOURCE_DIR}/external/include"
        #json
        "${PROJECT_SOURCE_DIR}/src/tests/hello_xr/json/include"
        # For including compiled shaders
        "${CMAKE_CURRENT_BINARY_DIR}"
		# OpenCV
		${OpenCV_INCLUDE_DIRS}
)


if(GLSLANG_VALIDATOR AND NOT GLSLC_COMMAND)
    target_compile_definitions(hello_xr PRIVATE USE_GLSLANGVALIDATOR)
endif()

if(XR_USE_GRAPHICS_API_VULKAN)
    target_include_directories(hello_xr PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(hello_xr PRIVATE ${Vulkan_LIBRARY})
endif()

if(TARGET openxr-gfxwrapper)
    target_link_libraries(hello_xr PRIVATE openxr-gfxwrapper)
endif()
if(WIN32)
    target_link_libraries(hello_xr PRIVATE ole32)
    if(MSVC)
        target_compile_definitions(hello_xr PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_options(hello_xr PRIVATE /Zc:wchar_t /Zc:forScope /W4)
        if(NOT
           CMAKE_CXX_COMPILER_ID
           STREQUAL
           "Clang"
        )
            # If actually msvc and not clang-cl
            target_compile_options(openxr_c_compile_test PRIVATE /WX)
        endif()

        # Right now can't build d3d features on MinGW because of directxcolors, directxmath, etc.
        target_link_libraries(
            hello_xr
            PRIVATE
                d3d11
                d3d12
                d3dcompiler
                dxgi
        )
    endif()
endif()

if(NOT ANDROID)
    install(
        TARGETS hello_xr RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                 COMPONENT hello_xr
    )
    if(NOT WIN32)
        install(
            FILES hello_xr.1
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/
            COMPONENT ManPages
        )
    endif()
endif()


