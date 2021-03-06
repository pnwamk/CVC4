# Find JUnit
# JUnit_FOUND - system has JUnit lib
# JUnit_JAR - JUnit jar file
# JUnit_JAR_DEPS - JUnit jar dependencies

find_package(Java REQUIRED)
include(UseJava)
find_package(Hamcrest REQUIRED)

find_jar(JUnit_JAR NAMES junit junit4)

if(JUnit_JAR)
  set(JUnit_JAR_DEPS ${Hamcrest_JAR})
  # Determine version of JUnit
  execute_process(
    COMMAND ${Java_JAVA_EXECUTABLE} -cp ${JUnit_JAR} junit.runner.Version
    OUTPUT_VARIABLE JUnit_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JUnit
  REQUIRED_VARS JUnit_JAR JUnit_JAR_DEPS
  VERSION_VAR JUnit_VERSION)

mark_as_advanced(JUnit_JAR JUnit_JAR_DEPS)
