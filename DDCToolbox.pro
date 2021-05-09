TEMPLATE = subdirs
TARGET = DDCToolbox

!no_app {
    SUBDIRS += src
}

!no_tests {
    SUBDIRS += tests
}

