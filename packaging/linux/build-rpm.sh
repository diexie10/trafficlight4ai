#!/usr/bin/env bash
set -euo pipefail

version="${1:?usage: build-rpm.sh <version> <distro> [output-dir] [cc] [cxx]}"
distro="${2:?usage: build-rpm.sh <version> <distro> [output-dir] [cc] [cxx]}"
output_dir="${3:-dist/release}"
cc="${4:-gcc}"
cxx="${5:-g++}"
package="trafficlight4ai"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="${repo_root}/build/package-rpm-${distro}"
root_dir="${repo_root}/build/package-rpm-root-${distro}"
rpm_top="${repo_root}/build/rpmbuild-${distro}"

rm -rf "${build_dir}" "${root_dir}" "${rpm_top}"
mkdir -p "${output_dir}" "${rpm_top}/BUILD" "${rpm_top}/RPMS" "${rpm_top}/SOURCES" \
    "${rpm_top}/SPECS" "${rpm_top}/SRPMS"

CC="${cc}" CXX="${cxx}" cmake -S "${repo_root}" -B "${build_dir}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "${build_dir}" -j"$(nproc)"
DESTDIR="${root_dir}" cmake --install "${build_dir}" --strip

cp -a "${root_dir}" "${rpm_top}/SOURCES/package-root"
cat > "${rpm_top}/SPECS/${package}.spec" <<EOF
Name: ${package}
Version: ${version}
Release: 1%{?dist}
Summary: Visual traffic light status indicator for AI coding tools
License: MIT
URL: https://github.com/yhz61010/trafficlight4ai

%description
trafficlight4ai shows AI coding assistant state through a floating desktop
traffic light, tray icon, and local CLI integration.

%prep

%build

%install
mkdir -p %{buildroot}
cp -a %{_sourcedir}/package-root/. %{buildroot}/

%files
%license /usr/share/doc/${package}/LICENSE
%doc /usr/share/doc/${package}/README.md
%doc /usr/share/doc/${package}/README_zh.md
/usr/bin/trafficlight4ai
/usr/bin/tl4ai-ctl
/usr/share/applications/trafficlight4ai.desktop
/usr/share/pixmaps/trafficlight4ai.png

%changelog
* Mon Jun 08 2026 Michael Leo <noreply@example.com> - ${version}-1
- Release ${version}
EOF

rpmbuild --define "_topdir ${rpm_top}" -bb "${rpm_top}/SPECS/${package}.spec"
rpm_file="$(find "${rpm_top}/RPMS" -type f -name '*.rpm' | head -n 1)"
cp "${rpm_file}" "${output_dir}/${package}-${version}-${distro}-amd64.rpm"
