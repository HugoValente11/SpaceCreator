#BINARY_ARTIFACTS_BRANCH = master
#PROJECT_USER_FILE_EXTENSION = .user

set(IDE_VERSION ${QTC_VERSION_STR})               # The IDE version.
set(IDE_VERSION_COMPAT ${QTC_VERSION_STR})        # The IDE Compatibility version.
set(IDE_VERSION_DISPLAY ${QTC_VERSION_STR})       # The IDE display version.
set(IDE_COPYRIGHT_YEAR "2018")          # The IDE copyright year.

set(IDE_SETTINGSVARIANT "QtProject")                  # The IDE settings variation.
set(IDE_COPY_SETTINGSVARIANT "Nokia")                 # The IDE settings to initially import.
set(IDE_DISPLAY_NAME "Qt Creator")                    # The IDE display name.
set(IDE_ID "qtcreator")                               # The IDE id (no spaces, lowercase!)
set(IDE_CASED_ID "QtCreator")                         # The cased IDE id (no spaces!)
set(IDE_BUNDLE_IDENTIFIER "org.qt-project.${IDE_ID}") # The macOS application bundle identifier.
