#!/usr/bin/env bash
set -euo pipefail

version="${1:?usage: build-deb.sh <version> [output-dir]}"
output_dir="${2:-dist/release}"
package="trafficlight4ai"
arch="amd64"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="${repo_root}/build/package-deb"
root_dir="${repo_root}/build/package-deb-root"
work_dir="${repo_root}/build/package-deb-work"

rm -rf "${build_dir}" "${root_dir}" "${work_dir}"
mkdir -p "${output_dir}" "${work_dir}"

cmake -S "${repo_root}" -B "${build_dir}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "${build_dir}" -j"$(nproc)"
DESTDIR="${root_dir}" cmake --install "${build_dir}" --strip

mkdir -p "${root_dir}/DEBIAN" "${work_dir}/debian"
cat > "${work_dir}/debian/control" <<EOF
Source: ${package}
Section: utils
Priority: optional
Maintainer: Michael Leo <noreply@example.com>
Standards-Version: 4.7.0
Package: ${package}
Architecture: ${arch}
Description: Visual traffic light status indicator for AI coding tools
 trafficlight4ai shows AI coding assistant state through a floating desktop
 traffic light, tray icon, and local CLI integration.
EOF

pushd "${work_dir}" >/dev/null
deps="$(dpkg-shlibdeps -O "${root_dir}/usr/bin/trafficlight4ai" "${root_dir}/usr/bin/tl4ai-ctl" \
    | sed -n 's/^shlibs:Depends=//p')"
popd >/dev/null

installed_size="$(du -sk "${root_dir}" | awk '{print $1}')"
cat > "${root_dir}/DEBIAN/control" <<EOF
Package: ${package}
Version: ${version}
Section: utils
Priority: optional
Architecture: ${arch}
Maintainer: Michael Leo <noreply@example.com>
Installed-Size: ${installed_size}
Depends: ${deps}
Homepage: https://github.com/yhz61010/trafficlight4ai
Description: Visual traffic light status indicator for AI coding tools
 trafficlight4ai shows AI coding assistant state through a floating desktop
 traffic light, tray icon, and local CLI integration.
EOF

dpkg-deb --build --root-owner-group "${root_dir}" \
    "${output_dir}/${package}-${version}-linux-${arch}.deb"
