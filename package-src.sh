#!/bin/bash
# Package source distribution for RM1x Controller
set -euo pipefail

VERSION="0.2.1"
ROOT="$(cd "$(dirname "$0")" && pwd)"
OUTDIR="${1:-$ROOT}"
TARBALL="rm1x-controller-${VERSION}-src.tar.gz"
ZIPFILE="rm1x-controller-${VERSION}-src.zip"

echo "Packaging RM1x Controller v${VERSION} source distribution..."

TMPDIR=$(mktemp -d)
SRCDIR="${TMPDIR}/rm1x-controller-${VERSION}"
mkdir -p "${SRCDIR}"

cp -r "$ROOT/CMakeLists.txt" "${SRCDIR}/"
cp -r "$ROOT/Source" "${SRCDIR}/"
cp -r "$ROOT/Resources" "${SRCDIR}/"
cp -r "$ROOT/README.md" "${SRCDIR}/"
cp -r "$ROOT/BUILD.md" "${SRCDIR}/"
cp -r "$ROOT/TESTING.md" "${SRCDIR}/"
cp -r "$ROOT/ARCHITECTURE.md" "${SRCDIR}/"
cp -r "$ROOT/CHANGELOG.md" "${SRCDIR}/"
cp -r "$ROOT/LICENSE" "${SRCDIR}/"
cp -r "$ROOT/rebuild.bat" "${SRCDIR}/"
cp -r "$ROOT/install-vst3.ps1" "${SRCDIR}/"
cp -r "$ROOT/.gitignore" "${SRCDIR}/"
cp -r "$ROOT/docs" "${SRCDIR}/"
[ -f "$ROOT/MIDI-INPUT-NOTES.md" ] && cp -r "$ROOT/MIDI-INPUT-NOTES.md" "${SRCDIR}/"

cd "${TMPDIR}"
tar czf "${OUTDIR}/${TARBALL}" "rm1x-controller-${VERSION}/"
(command -v zip >/dev/null && zip -r -q "${OUTDIR}/${ZIPFILE}" "rm1x-controller-${VERSION}/") || true
rm -rf "${TMPDIR}"

echo ""
echo "Created source distributions:"
echo "  ${OUTDIR}/${TARBALL}"
[ -f "${OUTDIR}/${ZIPFILE}" ] && echo "  ${OUTDIR}/${ZIPFILE}"
ls -lh "${OUTDIR}/${TARBALL}" ${OUTDIR}/${ZIPFILE} 2>/dev/null || true
