#!/bin/bash
# Package source distribution for RM1x Controller

VERSION="0.1.0"
TARBALL="rm1x-controller-${VERSION}-src.tar.gz"
ZIPFILE="rm1x-controller-${VERSION}-src.zip"

echo "Packaging RM1x Controller v${VERSION} source distribution..."

# Create temporary directory
TMPDIR=$(mktemp -d)
SRCDIR="${TMPDIR}/rm1x-controller-${VERSION}"
mkdir -p "${SRCDIR}"

# List of files/directories to include
cp -r CMakeLists.txt "${SRCDIR}/"
cp -r Source "${SRCDIR}/"
cp -r README.md "${SRCDIR}/"
cp -r BUILD.md "${SRCDIR}/"
cp -r TESTING.md "${SRCDIR}/"
cp -r ARCHITECTURE.md "${SRCDIR}/"
cp -r CHANGELOG.md "${SRCDIR}/"
cp -r rebuild.bat "${SRCDIR}/"
cp -r install-vst3.ps1 "${SRCDIR}/"
cp -r .gitignore "${SRCDIR}/"

# Create tarball
cd "${TMPDIR}"
tar czf "/workspace/${TARBALL}" "rm1x-controller-${VERSION}/"

# Create zip (Windows-friendly)
zip -r -q "/workspace/${ZIPFILE}" "rm1x-controller-${VERSION}/"

# Cleanup
rm -rf "${TMPDIR}"

echo ""
echo "Created source distributions:"
echo "  ${TARBALL}"
echo "  ${ZIPFILE}"
echo ""
ls -lh "/workspace/${TARBALL}" "/workspace/${ZIPFILE}"
