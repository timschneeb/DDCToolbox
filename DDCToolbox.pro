TEMPLATE = subdirs
TARGET = DDCToolbox

SUBDIRS = src

!no_tests {
    SUBDIRS += tests
}
