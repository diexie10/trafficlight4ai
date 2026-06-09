#!/usr/bin/env bash
set -euo pipefail

version="${1:?version required}"
arch="${2:-$(uname -m)}"
package="trafficlight4ai"
build_dir="${BUILD_DIR:-build-macos-package}"
dist_dir="${DIST_DIR:-dist/release}"
stage_dir="${STAGE_DIR:-dist/macos/${package}-${version}-macos-${arch}}"
archive="${dist_dir}/${package}-${version}-macos-${arch}.zip"

if [[ -z "${CMAKE_PREFIX_PATH:-}" ]] \
    && command -v brew >/dev/null 2>&1 \
    && qt_prefix="$(brew --prefix qt 2>/dev/null)"; then
    CMAKE_PREFIX_PATH="${qt_prefix}"
    export CMAKE_PREFIX_PATH
fi

cmake -S . -B "${build_dir}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}" \
    -DCMAKE_OSX_ARCHITECTURES="${CMAKE_OSX_ARCHITECTURES:-${arch}}" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="${CMAKE_OSX_DEPLOYMENT_TARGET:-12.0}"
cmake --build "${build_dir}" --config Release --parallel "$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)"

rm -rf "${stage_dir}" "${archive}"
mkdir -p "${stage_dir}/bin" "${stage_dir}/docs" "${dist_dir}"
archive_abs="$(cd "$(dirname "${archive}")" && pwd)/$(basename "${archive}")"

app_path="${stage_dir}/${package}.app"
cp -R "${build_dir}/src/${package}.app" "${app_path}"
cp "${build_dir}/tools/tl4ai-ctl" "${app_path}/Contents/MacOS/tl4ai-ctl"
cp README.md README_zh.md LICENSE "${stage_dir}/docs/"

cat > "${stage_dir}/bin/tl4ai-ctl" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
script_dir="$(cd "$(dirname "$0")" && pwd)"
exec "${script_dir}/../trafficlight4ai.app/Contents/MacOS/tl4ai-ctl" "$@"
EOF
chmod +x "${stage_dir}/bin/tl4ai-ctl"

macdeployqt "${app_path}" \
    -executable="${app_path}/Contents/MacOS/tl4ai-ctl" \
    -verbose=1

(cd "$(dirname "${stage_dir}")" && ditto -c -k --sequesterRsrc --keepParent "$(basename "${stage_dir}")" "${archive_abs}")
