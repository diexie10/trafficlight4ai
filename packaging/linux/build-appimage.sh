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
QT_PLUGIN_SHA256="15106be885c1c48a021198e7e1e9a48ce9d02a86dd0a1848f00bdbf3c1c92724"
RUNTIME_VER="20251108"
RUNTIME_SHA256="2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d"

# Download a tool from pinned release, falling back to continuous on failure.
download_tool() {
    local output="$1" pinned_url="$2" continuous_url="$3" expected_sha="$4"

    if curl --fail --location --retry 3 --retry-all-errors --connect-timeout 20 -o "${output}" "${pinned_url}"; then
        if echo "${expected_sha}  ${output}" | sha256sum -c --strict 2>/dev/null; then
            echo "Downloaded pinned release: ${pinned_url##*/}"
            return 0
        fi
        echo "WARNING: SHA256 mismatch for pinned release, falling back to continuous"
    else
        echo "WARNING: Pinned release download failed, falling back to continuous"
    fi

    curl --fail --location --retry 3 --retry-all-errors --connect-timeout 20 -o "${output}" "${continuous_url}"
    echo "Downloaded continuous release: ${continuous_url##*/}"
}

linuxdeploy="${tool_dir}/linuxdeploy-x86_64.AppImage"
qt_plugin="${tool_dir}/linuxdeploy-plugin-qt-x86_64.AppImage"

download_tool "${linuxdeploy}" \
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/${LINUXDEPLOY_VER}/linuxdeploy-x86_64.AppImage" \
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" \
    "${LINUXDEPLOY_SHA256}"

download_tool "${qt_plugin}" \
    "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/${QT_PLUGIN_VER}/linuxdeploy-plugin-qt-x86_64.AppImage" \
    "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage" \
    "${QT_PLUGIN_SHA256}"

# Pre-download AppImage runtime so appimagetool doesn't fetch it at build time.
runtime="${tool_dir}/runtime-x86_64"
download_tool "${runtime}" \
    "https://github.com/AppImage/type2-runtime/releases/download/${RUNTIME_VER}/runtime-x86_64" \
    "https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-x86_64" \
    "${RUNTIME_SHA256}"

chmod +x "${linuxdeploy}" "${qt_plugin}"
ln -sf "${qt_plugin}" "${tool_dir}/linuxdeploy-plugin-qt"
export PATH="${tool_dir}:${PATH}"
export LDAI_RUNTIME_FILE="${runtime}"

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
