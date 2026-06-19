cmake_minimum_required(VERSION 3.16)
project(qvplay VERSION 0.95 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(QVPLAY_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(QVPLAY_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

if(WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  set(QVPLAY_TTY_SOURCE win32/tty_w32.c)
  set(QVPLAY_PLATFORM_SOURCES win32/getopt.c)
  set(QVPLAY_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/win32")
else()
  include(CheckIncludeFile)
  include(CheckFunctionExists)
  check_include_file(fcntl.h HAVE_FCNTL_H)
  check_include_file(sgtty.h HAVE_SGTTY_H)
  check_include_file(sys/ioctl.h HAVE_SYS_IOCTL_H)
  check_include_file(sys/param.h HAVE_SYS_PARAM_H)
  check_include_file(sys/time.h HAVE_SYS_TIME_H)
  check_include_file(termio.h HAVE_TERMIO_H)
  check_include_file(termios.h HAVE_TERMIOS_H)
  check_include_file(unistd.h HAVE_UNISTD_H)
  check_include_file(ttold.h HAVE_TTOLD_H)
  check_include_file(ioctl-types.h HAVE_IOCTL_TYPES_H)
  check_function_exists(select HAVE_SELECT)
  set(STDC_HEADERS 1)
  set(TIME_WITH_SYS_TIME 1)
  configure_file(cmake/config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h")
  set(QVPLAY_TTY_SOURCE tty.c)
  set(QVPLAY_PLATFORM_SOURCES)
  set(QVPLAY_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()

set(QVPLAY_COMMON_SOURCES command.c ${QVPLAY_TTY_SOURCE})

add_executable(qvplay
  qvplay.c command1.c getuint.c jpeg.c ppm.c bmp.c
  ${QVPLAY_COMMON_SOURCES} ${QVPLAY_PLATFORM_SOURCES})
target_include_directories(qvplay PRIVATE
  "${QVPLAY_CONFIG_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(qvalldel
  qvalldel.c command3.c ${QVPLAY_COMMON_SOURCES} ${QVPLAY_PLATFORM_SOURCES})
target_include_directories(qvalldel PRIVATE
  "${QVPLAY_CONFIG_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

include(CTest)
if(BUILD_TESTING)
  add_executable(protocol_test command.c tests/protocol_test.c)
  target_compile_definitions(protocol_test PRIVATE QVPLAY_TEST_TRANSPORT=1)
  target_include_directories(protocol_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME protocol_receive COMMAND protocol_test)
endif()

install(TARGETS qvplay qvalldel RUNTIME DESTINATION bin)
install(FILES qvplay.1 qvalldel.1 DESTINATION share/man/man1)
