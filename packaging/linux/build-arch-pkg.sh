#!/usr/bin/env bash
set -euo pipefail

version="${1:?usage: build-arch-pkg.sh <version> [output-dir]}"
output_dir="${2:-dist/release}"
package="trafficlight4ai"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
work_dir="${repo_root}/build/arch-package"

rm -rf "${work_dir}"
mkdir -p "${work_dir}" "${output_dir}"

git -C "${repo_root}" archive --format=tar.gz --prefix="${package}-${version}/" \
    -o "${work_dir}/${package}-${version}.tar.gz" HEAD
source_sha="$(sha256sum "${work_dir}/${package}-${version}.tar.gz" | awk '{print $1}')"

cat > "${work_dir}/PKGBUILD" <<EOF
pkgname=${package}
pkgver=${version}
pkgrel=1
pkgdesc='Visual traffic light status indicator for AI coding tools'
arch=('x86_64')
url='https://github.com/yhz61010/trafficlight4ai'
license=('MIT')
depends=('qt6-base' 'qt6-multimedia')
makedepends=('cmake' 'gcc' 'qt6-tools')
source=("${package}-${version}.tar.gz")
sha256sums=('${source_sha}')

build() {
    cmake -S "\$srcdir/${package}-${version}" -B build \\
        -DCMAKE_BUILD_TYPE=Release \\
        -DBUILD_TESTING=OFF \\
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build -j"\$(nproc)"
}

package() {
    DESTDIR="\$pkgdir" cmake --install build --strip
}
EOF

pushd "${work_dir}" >/dev/null
makepkg --force --noconfirm
popd >/dev/null

cp "${work_dir}/${package}-${version}-1-x86_64.pkg.tar.zst" \
    "${output_dir}/${package}-${version}-arch-amd64.pkg.tar.zst"
