# 
# Copyright (c) 2015-2016 CNRS
# Copyright (c) 2015 Wandercraft, 86 rue de Paris 91400 Orsay, France.
# 
# This file is part of Pinocchio
# Pinocchio is free software: you can redistribute it
# and/or modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either version
# 3 of the License, or (at your option) any later version.
# Pinocchio is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Lesser Public License for more details. You should have
# received a copy of the GNU Lesser General Public License along with
# Pinocchio If not, see
# <http://www.gnu.org/licenses/>.

MACRO(ADD_GROUP GROUP_NAME FILENAMES)
  FOREACH(filename ${${FILENAMES}})
    GET_FILENAME_COMPONENT(filenamePath ${filename} PATH)
    IF(NOT (filenamePath STREQUAL ""))
      STRING(REGEX REPLACE "/" "\\\\" filenamePath ${filenamePath}) 
      SOURCE_GROUP("${GROUP_NAME}\\${filenamePath}" FILES ${filename})
    ELSE()
      SOURCE_GROUP("${GROUP_NAME}" FILES ${filename})
    ENDIF()
  ENDFOREACH()
ENDMACRO(ADD_GROUP) 

MACRO(ADD_HEADER_GROUP FILENAMES)
  ADD_GROUP("Header Files" ${FILENAMES})
ENDMACRO(ADD_HEADER_GROUP FILENAMES)

MACRO(ADD_SOURCE_GROUP FILENAMES)
  ADD_GROUP("Source Files" ${FILENAMES})
ENDMACRO(ADD_SOURCE_GROUP FILENAMES)

IF(BUILD_PYTHON_INTERFACE)
  # --- Collect header files
  SET(${PROJECT_NAME}_PYTHON_HEADERS
    eigen_container.hpp
    handler.hpp
    python.hpp
    se3.hpp
    force.hpp
    motion.hpp
    inertia.hpp
    joints-models.hpp
    joints-variant.hpp
    joint.hpp
    joint-derived.hpp
    frame.hpp
    model.hpp
    data.hpp
    algorithms.hpp
    parsers.hpp
    explog.hpp
    geometry-object.hpp
    geometry-model.hpp
    geometry-data.hpp
    )

  LIST(APPEND HEADERS ${${PROJECT_NAME}_PYTHON_HEADERS})
  LIST(REMOVE_DUPLICATES HEADERS)

  MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/bindings/python")
  FOREACH(header ${${PROJECT_NAME}_PYTHON_HEADERS})
    GET_FILENAME_COMPONENT(headerName ${header} NAME)
    GET_FILENAME_COMPONENT(headerPath ${header} PATH)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E ${LINK}
      ${${PROJECT_NAME}_SOURCE_DIR}/bindings/python/${header}
      ${${PROJECT_NAME}_BINARY_DIR}/include/${PROJECT_NAME}/bindings/python/${header})
    INSTALL(FILES ${${PROJECT_NAME}_SOURCE_DIR}/bindings/python/${header}
      DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}/bindings/python
            PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)
  ENDFOREACH(header)

  MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/lib/python/${PROJECT_NAME}")
  
  # --- COMPILE WRAPPER
  SET(PYWRAP ${PROJECT_NAME}_pywrap)
  ADD_LIBRARY(${PYWRAP} SHARED module.cpp python.cpp)
  PKG_CONFIG_USE_DEPENDENCY(${PYWRAP} eigenpy)

  IF(URDFDOM_FOUND)
    PKG_CONFIG_USE_DEPENDENCY(${PYWRAP} urdfdom)
  ENDIF(URDFDOM_FOUND)
  IF(HPP_FCL_FOUND)
    PKG_CONFIG_USE_DEPENDENCY(${PYWRAP} hpp-fcl)
  ENDIF(HPP_FCL_FOUND)
  IF(LUA5_1_FOUND)
    PKG_CONFIG_USE_DEPENDENCY(${PYWRAP} lua5.1)
  ENDIF(LUA5_1_FOUND)

  TARGET_LINK_LIBRARIES(${PYWRAP} ${PROJECT_NAME} ${Boost_LIBRARIES})
  TARGET_LINK_BOOST_PYTHON(${PYWRAP})

  IF(APPLE)
    # We need to change the extension for python bindings
    SET_TARGET_PROPERTIES(${PYWRAP} PROPERTIES SUFFIX ".so")
  ENDIF(APPLE)

  SET_TARGET_PROPERTIES(${PYWRAP} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/python/${PROJECT_NAME}")

  INSTALL(FILES
    "${CMAKE_BINARY_DIR}/lib/python/${PROJECT_NAME}/lib${PYWRAP}.so"
    DESTINATION ${PYTHON_SITELIB}/${PROJECT_NAME})
  
  # --- INSTALL SCRIPTS 
  SET(PYTHON_FILES
    __init__.py
    utils.py
    robot_wrapper.py
    romeo_wrapper.py
    rpy.py
    explog.py
    )
  
  FOREACH(python ${PYTHON_FILES})
    GET_FILENAME_COMPONENT(pythonFile ${python} NAME)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E ${LINK}
      ${${PROJECT_NAME}_SOURCE_DIR}/bindings/python/${python}
      ${${PROJECT_NAME}_BINARY_DIR}/lib/python/${PROJECT_NAME}/${pythonFile})
      
    # Tag pyc file as generated.
    SET_SOURCE_FILES_PROPERTIES(
      "${${PROJECT_NAME}_BINARY_DIR}/lib/python/${PROJECT_NAME}/${pythonFile}c"
      PROPERTIES GENERATED TRUE)
      
    EXECUTE_PROCESS(COMMAND
      ${PYTHON_EXECUTABLE} -m py_compile
      ${${PROJECT_NAME}_BINARY_DIR}/lib/python/${PROJECT_NAME}/${pythonFile})
      
    # Clean generated files.
    SET_PROPERTY(
      DIRECTORY APPEND PROPERTY
      ADDITIONAL_MAKE_CLEAN_FILES
      "${${PROJECT_NAME}_BINARY_DIR}/lib/python/${PROJECT_NAME}/${pythonFile}c")
      
    INSTALL(FILES
      "${${PROJECT_NAME}_SOURCE_DIR}/bindings/python/${python}"
      "${${PROJECT_NAME}_BINARY_DIR}/lib/python/${PROJECT_NAME}/${pythonFile}c"
      DESTINATION ${PYTHON_SITELIB}/${PROJECT_NAME})
  ENDFOREACH(python)

ENDIF(BUILD_PYTHON_INTERFACE)
