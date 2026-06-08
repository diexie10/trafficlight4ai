#!/usr/bin/env bash
set -euo pipefail

version="${1:?usage: build-appimage.sh <version> [output-dir]}"
output_dir="${2:-dist/release}"
package="trafficlight4ai"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="${repo_root}/build/package-appimage"
app_dir="${repo_root}/build/AppDir"
tool_dir="${repo_root}/build/appimage-tools"

rm -rf "${build_dir}" "${app_dir}" "${tool_dir}"
mkdir -p "${output_dir}" "${tool_dir}"

cmake -S "${repo_root}" -B "${build_dir}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "${build_dir}" -j"$(nproc)"
DESTDIR="${app_dir}" cmake --install "${build_dir}" --strip

# Pinned versions and SHA256 checksums for reproducible builds.
# Update these when upgrading linuxdeploy.
LINUXDEPLOY_VER="1-alpha-20251107-1"
QT_PLUGIN_VER="1-alpha-20250213-1"
LINUXDEPLOY_SHA256="c20cd71e3a4e3b80c3483cef793cda3f4e990aca14014d23c544ca3ce1270b4d"
QT_PLUGIN_SHA256="bef140a1a96994029153dca8c00b1750b9a5a764fb9db2dc68d7bb40e8a29e8a"

linuxdeploy="${tool_dir}/linuxdeploy-x86_64.AppImage"
qt_plugin="${tool_dir}/linuxdeploy-plugin-qt-x86_64.AppImage"
curl -L -o "${linuxdeploy}" \
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/${LINUXDEPLOY_VER}/linuxdeploy-x86_64.AppImage"
curl -L -o "${qt_plugin}" \
    "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/${QT_PLUGIN_VER}/linuxdeploy-plugin-qt-x86_64.AppImage"

echo "${LINUXDEPLOY_SHA256}  ${linuxdeploy}" | sha256sum -c --strict
echo "${QT_PLUGIN_SHA256}  ${qt_plugin}" | sha256sum -c --strict

chmod +x "${linuxdeploy}" "${qt_plugin}"
ln -sf "${qt_plugin}" "${tool_dir}/linuxdeploy-plugin-qt"
export PATH="${tool_dir}:${PATH}"

if command -v qmake6 >/dev/null 2>&1; then
    export QMAKE="$(command -v qmake6)"
elif [ -x /usr/lib/qt6/bin/qmake ]; then
    export QMAKE=/usr/lib/qt6/bin/qmake
fi

export APPIMAGE_EXTRACT_AND_RUN=1
export OUTPUT="${output_dir}/${package}-${version}-linux-amd64.AppImage"
"${linuxdeploy}" \
    --appdir "${app_dir}" \
    --executable "${app_dir}/usr/bin/${package}" \
    --executable "${app_dir}/usr/bin/tl4ai-ctl" \
    --desktop-file "${app_dir}/usr/share/applications/${package}.desktop" \
    --icon-file "${app_dir}/usr/share/pixmaps/${package}.png" \
    --plugin qt \
    --output appimage
