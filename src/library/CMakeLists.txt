set(LIBNAME litesql)


##############################################################################
# litesql library

# This library is either shared or static (default shared) depending on the
# user-defined value. It is statically linked with all other libraries
# (backend-specific, litesql-util, etc).

# This library is the only one installed on the system, and should be linked
# with projects wishing to use litesql.
##############################################################################
set(litesql_SOURCES    database.cpp
            expr.cpp
            selectquery.cpp
            persistent.cpp
            backend.cpp
            updatequery.cpp
            field.cpp
            datasource.cpp
            compatibility.hpp
            compatibility.cpp
            datetime.cpp
            ../../include/litesql/commontypes.h
            ../../include/litesql/database.hpp
            ../../include/litesql/expr.hpp
            ../../include/litesql/selectquery.hpp
            ../../include/litesql/persistent.hpp
            ../../include/litesql/backend.hpp
            ../../include/litesql/updatequery.hpp
            ../../include/litesql/field.hpp
            ../../include/litesql/datasource.hpp
            ../../include/litesql/datetime.hpp
)
add_library(${LIBNAME} ${litesql_SOURCES})
target_link_libraries (litesql litesql-util ${BACKEND_LIBRARIES} ${EXPAT_LIBRARIES} pthread dl)
install(TARGETS ${LIBNAME} EXPORT ${LIBNAME}
            LIBRARY DESTINATION lib
            RUNTIME DESTINATION lib
            ARCHIVE DESTINATION lib
            COMPONENT devel)

##############################################################################
# litesql-util library
##############################################################################
set(litesql_util_SOURCES
            split.cpp
            string.cpp
            ../../include/litesql/split.hpp
            ../../include/litesql/string.hpp
            )
add_library(litesql-util STATIC ${litesql_util_SOURCES})
# This compile flag is required when building a shared library from static
# libraries
set_target_properties(litesql-util PROPERTIES COMPILE_FLAGS -fPIC)
target_link_libraries(litesql-util ${BACKEND_LIBRARIES} ${EXPAT_LIBRARIES})


##############################################################################
# litesql_sqlite library
##############################################################################
if (LITESQL_WITH_SQLITE3)
  set(${LIBNAME}_sqlite_SRC
                sqlite3_backend.cpp
                sqlite3.hpp)

  add_library(${LIBNAME}_sqlite STATIC
    ${${LIBNAME}_sqlite_SRC})
  target_link_libraries(litesql sqlite3 ${EXPAT_LIBRARIES})
  set_target_properties(${LIBNAME}_sqlite PROPERTIES COMPILE_FLAGS -fPIC)
endif (LITESQL_WITH_SQLITE3)


##############################################################################
# litesql_mysql library
##############################################################################
if (LITESQL_WITH_MYSQL)
  add_library(${LIBNAME}_mysql STATIC
              mysql.cpp)
set_target_properties(${LIBNAME}_mysql PROPERTIES COMPILE_FLAGS -fPIC)
endif (LITESQL_WITH_MYSQL)


##############################################################################
# litesql_ocilib library
##############################################################################
if (LITESQL_WITH_OCILIB)
  add_library(${LIBNAME}_ocilib STATIC
              ocilib_backend.cpp
              ocilib_backend.hpp
  )
set_target_properties(${LIBNAME}_ocilib PROPERTIES COMPILE_FLAGS -fPIC)
endif (LITESQL_WITH_OCILIB)


##############################################################################
# litesql_postgresql library
##############################################################################
if (LITESQL_WITH_POSTGRES)
  add_library(${LIBNAME}_postgresql STATIC
              postgresql.cpp
  )
set_target_properties(${LIBNAME}_postgresql PROPERTIES COMPILE_FLAGS -fPIC)
endif (LITESQL_WITH_POSTGRES)

##############################################################################
# litesql_odbc library
##############################################################################
if (LITESQL_WITH_ODBC)
  add_library(${LIBNAME}_odbc STATIC
              odbc_backend.cpp
              odbc_backend.hpp
  )
set_target_properties(${LIBNAME}_odbc PROPERTIES COMPILE_FLAGS -fPIC)
endif (LITESQL_WITH_ODBC)

include_directories(../../src/library ../../include)
