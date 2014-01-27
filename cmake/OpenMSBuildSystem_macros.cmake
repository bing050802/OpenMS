## fills ${varname} with the names of Debug and Release libraries (which usually only differ on MSVC)
## @param varname Name of the variable which will hold the result string (e.g. "optimized myLib.so debug myLibDebug.so")
## @param libnames   List of library names which are searched (release libs)
## @param libnames_d List of library names which are searched (debug libs)
## @param human_libname Name of the library (for display only)
MACRO (OPENMS_CHECKLIB varname libnames libnames_d human_libname)
	# force find_library to run again
	SET(${varname}_OPT "${varname}_OPT-NOTFOUND" CACHE FILEPATH "Cleared." FORCE)
	FIND_LIBRARY(${varname}_OPT NAMES ${libnames} PATHS ${CONTRIB_LIB_DIR} DOC "${human_libname} library dir" NO_DEFAULT_PATH)
	if ("${varname}_OPT" STREQUAL "${varname}_OPT-NOTFOUND")
		MESSAGE(FATAL_ERROR "Unable to find ${human_libname} library! Searched names are: [${libnames}]\nPlease make sure it is part of the contrib (which we assume to be in either of these directories: ${CONTRIB_LIB_DIR}). Set custom contrib paths using the CMAKE_FIND_ROOT_PATH variable in CMake.")
	else()
		MESSAGE(STATUS "Found ${human_libname} library (Release) at: " ${${varname}_OPT})
	endif()
	# force find_library to run again
	SET(${varname}_DBG "${varname}_DBG-NOTFOUND" CACHE FILEPATH "Cleared." FORCE)
	FIND_LIBRARY(${varname}_DBG NAMES ${libnames_d} PATHS ${CONTRIB_LIB_DIR} DOC "${human_libname} (Debug) library dir" NO_DEFAULT_PATH)
	if ("${varname}_DBG" STREQUAL "${varname}_DBG-NOTFOUND")
		MESSAGE(FATAL_ERROR "Unable to find ${human_libname} (Debug) library! Searched names are: [${libnames}]\nPlease make sure it is part of the contrib (which we assume to be in either of these directories: ${CONTRIB_LIB_DIR}). Set custom contrib paths using the CMAKE_FIND_ROOT_PATH variable in CMake.")
	else()
		MESSAGE(STATUS "Found ${human_libname} library (Debug) at: " ${${varname}_DBG})
	endif()
	## combine result and include "optimized" and "debug" keywords which are essential for target_link_libraries()
	set(${varname} optimized ${${varname}_OPT} debug ${${varname}_DBG})
ENDMACRO (OPENMS_CHECKLIB)

## Copy the dll produced by the given target to the test/doc binary path.
## @param targetname The target to modify.
## @note This macro will do nothing with non MSVC generators.
macro(copy_dll_to_extern_bin targetname)
  if(MSVC)
    get_target_property(WIN32_DLLLOCATION ${targetname} LOCATION)
    get_filename_component(WIN32_DLLPATH ${WIN32_DLLLOCATION} PATH)

    ## copy OpenMS.dll to test executables dir "$(TargetFileName)" is a placeholder filled by VS at runtime
    file(TO_NATIVE_PATH "${WIN32_DLLPATH}/$(TargetFileName)" DLL_SOURCE)

    file(TO_NATIVE_PATH "${OPENMS_HOST_BINARY_DIRECTORY}/src/tests/class_tests/bin/$(ConfigurationName)/$(TargetFileName)" DLL_TEST_TARGET)
    file(TO_NATIVE_PATH "${OPENMS_HOST_BINARY_DIRECTORY}/src/tests/class_tests/bin/$(ConfigurationName)" DLL_TEST_TARGET_PATH)

    file(TO_NATIVE_PATH "${OPENMS_HOST_BINARY_DIRECTORY}/doc/doxygen/parameters/$(ConfigurationName)/$(TargetFileName)" DLL_DOC_TARGET)
    file(TO_NATIVE_PATH "${OPENMS_HOST_BINARY_DIRECTORY}/doc/doxygen/parameters/$(ConfigurationName)" DLL_DOC_TARGET_PATH)


    add_custom_command(TARGET ${targetname}
                      POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E make_directory "${DLL_TEST_TARGET_PATH}"
                      COMMAND ${CMAKE_COMMAND} -E copy ${DLL_SOURCE} ${DLL_TEST_TARGET}
                      COMMAND ${CMAKE_COMMAND} -E make_directory "${DLL_DOC_TARGET_PATH}"
                      COMMAND ${CMAKE_COMMAND} -E copy ${DLL_SOURCE} ${DLL_DOC_TARGET})
  endif(MSVC)
endmacro()