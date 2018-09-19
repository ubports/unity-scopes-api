add_executable(
  QActionMetadata_test
  QActionMetadata_test.cpp)

target_link_libraries(
    QActionMetadata_test
    ${LIBGTEST}
    ${TESTLIBS_QT})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QActionMetadata_test Qt5::Core)

add_test(QActionMetadata QActionMetadata_test)

###################################################

add_executable(
  QCategorisedResult_test
  QCategorisedResult_test.cpp)

target_link_libraries(
    QCategorisedResult_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QCategorisedResult_test Qt5::Core)

add_test(QCategorisedResult QCategorisedResult_test)

###################################################

add_executable(
  QSearchMetadata_test
  QSearchMetadata_test.cpp)

target_link_libraries(
    QSearchMetadata_test
    ${LIBGTEST}
    ${TESTLIBS_QT})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QSearchMetadata_test Qt5::Core)

add_test(QSearchMetadata QSearchMetadata_test)

###################################################

add_executable(
  QCannedQuery_test
  QCannedQuery_test.cpp)

target_link_libraries(
    QCannedQuery_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QCannedQuery_test Qt5::Core)

add_test(QCannedQuery QCannedQuery_test)

###################################################

add_executable(
  QColumnLayout_test
  QColumnLayout_test.cpp)

target_link_libraries(
    QColumnLayout_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QColumnLayout_test Qt5::Core)

add_test(QColumnLayout QColumnLayout_test)

###################################################

add_executable(
  QDepartment_test
  QDepartment_test.cpp)

target_link_libraries(
    QDepartment_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QDepartment_test Qt5::Core)

add_test(QDepartment QDepartment_test)

###################################################

add_executable(
  QSearchQueryBaseAPI_test
  QSearchQueryBaseAPI_test.cpp)

target_link_libraries(
    QSearchQueryBaseAPI_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QSearchQueryBaseAPI_test Qt5::Core)

add_test(QSearchQueryBaseAPI QSearchQueryBaseAPI_test)

###################################################

add_executable(
  QPreviewQueryBaseAPI_test
  QPreviewQueryBaseAPI_test.cpp)

target_link_libraries(
    QPreviewQueryBaseAPI_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QPreviewQueryBaseAPI_test Qt5::Core)

add_test(QPreviewQueryBaseAPI QPreviewQueryBaseAPI_test)

###################################################

add_executable(
  QUtils_test
  QUtils_test.cpp)

target_link_libraries(
    QUtils_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QUtils_test Qt5::Core)

add_test(QUtils QUtils_test)

###################################################

add_executable(
  QVariantBuilder_test
  QVariantBuilder_test.cpp)

target_link_libraries(
    QVariantBuilder_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QVariantBuilder_test Qt5::Core)

add_test(QVariantBuilder QVariantBuilder_test)

###################################################

add_executable(
  QScopeBaseAPIImpl_test
  QScopeBaseAPIImpl_test.cpp
  QScopeBaseAPIMock.h)

target_link_libraries(
    QScopeBaseAPIImpl_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QScopeBaseAPIImpl_test Qt5::Core)

add_test(QScopeBaseAPIImpl QScopeBaseAPIImpl_test)

###################################################

add_executable(
  QScopeCreation_test
  QScopeCreation_test.cpp)

target_link_libraries(
    QScopeCreation_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QScopeCreation_test Qt5::Core)

add_test(QScopeCreation QScopeCreation_test)

###################################################

add_executable(
  QPreviewWidget_test
  QPreviewWidget_test.cpp)

target_link_libraries(
    QPreviewWidget_test
    ${LIBGTEST}
    ${TESTLIBS_QT}
    ${TESTLIBS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(QPreviewWidget_test Qt5::Core)

add_test(QPreviewWidget QPreviewWidget_test)
