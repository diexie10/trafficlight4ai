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

linuxdeploy="${tool_dir}/linuxdeploy-x86_64.AppImage"
qt_plugin="${tool_dir}/linuxdeploy-plugin-qt-x86_64.AppImage"
curl -L -o "${linuxdeploy}" \
    https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
curl -L -o "${qt_plugin}" \
    https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x "${linuxdeploy}" "${qt_plugin}"

if command -v qmake6 >/dev/null 2>&1; then
    export QMAKE="$(command -v qmake6)"
elif [ -x /usr/lib/qt6/bin/qmake ]; then
    export QMAKE=/usr/lib/qt6/bin/qmake
fi

export APPIMAGE_EXTRACT_AND_RUN=1
export OUTPUT="${output_dir}/${package}-${version}-linux-amd64.AppImage"
"${linuxdeploy}" \
    --appdir "${app_dir}" \
    --desktop-file "${app_dir}/usr/share/applications/${package}.desktop" \
    --icon-file "${app_dir}/usr/share/pixmaps/${package}.png" \
    --plugin qt \
    --output appimage
