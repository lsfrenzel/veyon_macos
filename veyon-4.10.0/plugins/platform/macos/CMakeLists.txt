include(BuildVeyonPlugin)

build_veyon_plugin(macos-platform
	NAME MacOSPlatform
	SOURCES
	MacOSPlatformPlugin.cpp
	MacOSCoreFunctions.cpp
	MacOSPlatformConfigurationPage.h
	MacOSPlatformConfigurationPage.cpp
	MacOSPlatformConfigurationPage.ui
	MacOSFilesystemFunctions.cpp
	MacOSInputDeviceFunctions.cpp
	MacOSNetworkFunctions.cpp
	MacOSServerProcess.cpp
	MacOSServiceCore.cpp
	MacOSServiceFunctions.cpp
	MacOSSessionFunctions.cpp
	MacOSUserFunctions.cpp
	MacOSPlatformPlugin.h
	MacOSPlatformConfiguration.h
	MacOSCoreFunctions.h
	MacOSFilesystemFunctions.h
	MacOSInputDeviceFunctions.h
	MacOSKeyboardShortcutTrapper.h
	MacOSNetworkFunctions.h
	MacOSServerProcess.h
	MacOSServiceCore.h
	MacOSServiceFunctions.h
	MacOSSessionFunctions.h
	MacOSUserFunctions.h
	macos.qrc
	../common/LogonHelper.h
	../common/LogonHelper.cpp
	../common/PersistentLogonCredentials.h
	../common/PersistentLogonCredentials.cpp
	../common/PlatformSessionManager.h
	../common/PlatformSessionManager.cpp
	../common/ServiceDataManager.h
	../common/ServiceDataManager.cpp
)

set_source_files_properties(MacOSCoreFunctions.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)

target_include_directories(macos-platform PRIVATE
	../common
)

target_link_libraries(macos-platform PRIVATE
	"-framework CoreFoundation"
	"-framework IOKit"
	"-framework ApplicationServices"
	"-framework Security"
	"-framework SystemConfiguration"
)
